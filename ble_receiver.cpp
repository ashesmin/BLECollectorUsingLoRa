//  Copyright (c) 2015 Damian Kołakowski. All rights reserved.
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <curses.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <mqueue.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

struct hci_request ble_hci_request(uint16_t ocf, int clen, void * status, void * cparam) {
	// struct hci_request {
	//		uint16_t ogf;
   	//		uint16_t ocf;
	//		int event;
	//		void *cparam;
   	//		int clen;
   	//		void *rparam;
  	// 		int rlen;
  	// };
	struct hci_request rq;

	memset(&rq, 0, sizeof(rq));

	rq.ogf = OGF_LE_CTL;
	rq.ocf = ocf;
	rq.cparam = cparam;
	rq.clen = clen;
	rq.rparam = status;
	rq.rlen = 1;

	return rq;
}

struct my_msg{ 
	long msgtype;
	char addr[18];
	uint8_t batt_lv;
	uint8_t btn;
	struct timeval time;
	uint8_t rssi;
};

int parse_payload(uint8_t* buf, int size, char* addr, uint8_t rssi) {
	printf("ADDRESS : %s , RSSI : %d, BATTERY_LV : %03d, BUTTON : %02X\n", addr, rssi, buf[39], buf[41]);

	return 0;
}

int main() {
	int ret, status;

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// Get HCI device.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	// int hci_get_route(bdaddr_t *bdaddr)
	// 
	// 		Input : module address
	// 		Output : device_id
	//		Descriptor 
	//		 - return device_id of available modules
	//		 - if you input bdaddr, then it return device_id that has bdaddr
	//		 - if you input NULL, then it return device_id that is the first of the available modules
	//

	// int hci_open_dev(int device_id)
	//
	// 		Input : device_id
	// 		Output : file descriptor
	// 		Descriptor
	// 		 - create socket and return file descriptor of socket
	//	
	const int device = hci_open_dev(hci_get_route(NULL));
	if ( device < 0 ) { 
		perror("Failed to open HCI device.");

		return 0; 
	}
	
	struct mq_attr attr;
	attr.mq_maxmsg = 1000;
	attr.mq_msgsize = 36;
	struct my_msg mybuf;

	mqd_t mfd;

	mfd = mq_open("/my_mq", O_RDWR | O_CREAT,  0666, &attr);
    if (mfd == -1) {
        perror("open error");
        exit(0);
    }	

	struct timeval t;
	
	////////////////////////////////////////////////////////////////////////////////////////////////////
	// Set BLE scan parameters.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// typedef struct {
	// 		uint8_t type;
	// 		uint16_t interval;
	//		uint16_t window;
	//		uint8_t own_bdaddr_type;
	//		uint8_t filter;
	//	} __attribute__ ((packed)) le_set_scan_parameters_cp;
	le_set_scan_parameters_cp scan_params_cp;
	memset(&scan_params_cp, 0, sizeof(scan_params_cp));
	scan_params_cp.type 			= 0x00; 
	//scan_params_cp.interval 		= htobs(0x0010);
	scan_params_cp.interval 		= htobs(0x0012);
	//scan_params_cp.window 			= htobs(0x0010);
	scan_params_cp.window 			= htobs(0x0011);
	scan_params_cp.own_bdaddr_type 	= 0x00; // Public Device Address (default).
	scan_params_cp.filter 			= 0x00; // Accept all.

	struct hci_request scan_params_rq = ble_hci_request(OCF_LE_SET_SCAN_PARAMETERS, LE_SET_SCAN_PARAMETERS_CP_SIZE, &status, &scan_params_cp);
	ret = hci_send_req(device, &scan_params_rq, 1000);
	if( ret < 0 ) {
		hci_close_dev(device);
		perror("Failed to set scan parameters data.");
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// Set BLE events report mask.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	le_set_event_mask_cp event_mask_cp;
	memset(&event_mask_cp, 0, sizeof(le_set_event_mask_cp));

	for (int i = 0 ; i < 8 ; i++ ) event_mask_cp.mask[i] = 0xFF;

	struct hci_request set_mask_rq = ble_hci_request(OCF_LE_SET_EVENT_MASK, LE_SET_EVENT_MASK_CP_SIZE, &status, &event_mask_cp);
	ret = hci_send_req(device, &set_mask_rq, 1000);
	if ( ret < 0 ) {
		hci_close_dev(device);
		perror("Failed to set event mask.");
		return 0;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// Enable scanning.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	le_set_scan_enable_cp scan_cp;
	memset(&scan_cp, 0, sizeof(scan_cp));
	scan_cp.enable 		= 0x01;	// Enable flag.
	scan_cp.filter_dup 	= 0x00; // Filtering disabled.

	struct hci_request enable_adv_rq = ble_hci_request(OCF_LE_SET_SCAN_ENABLE, LE_SET_SCAN_ENABLE_CP_SIZE, &status, &scan_cp);
	ret = hci_send_req(device, &enable_adv_rq, 1000);

	if ( ret < 0 ) {
		hci_close_dev(device);
		perror("Failed to enable scan.");
		return 0;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// Get Results.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	struct hci_filter nf;

	hci_filter_clear(&nf);
	hci_filter_set_ptype(HCI_EVENT_PKT, &nf);
	hci_filter_set_event(EVT_LE_META_EVENT, &nf);

	if ( setsockopt(device, SOL_HCI, HCI_FILTER, &nf, sizeof(nf)) < 0 ) {
		hci_close_dev(device);
		perror("Could not set socket options\n");
		return 0;
	}

	printf("Scanning....\n");
	// HCI_MAX_EVENT_SIZE 260
	// HCI_EVENT_HDR_SIZE   2
	uint8_t buf[HCI_MAX_EVENT_SIZE];
	// typedef struct {
	// 		uint8_t subevent;
	// 		uint8_t data[0];
	// 	} __attribute__ ((packed)) evt_le_meta_event;
	evt_le_meta_event * meta_event;
	// typedef struct {
	// 		uint8_t evt_type;
	// 		uint8_t bdaddr_type;
	// 		bdaddr_t bdaddr;
	// 		uint8_t length;
	// 		uint8_t data[0];
	// 	} __attribute__ ((packed))le_advertising_info;
	le_advertising_info * info;

	int len, count = 0;
	struct timeval start,cur;

	gettimeofday(&start, NULL);
	while ( 1 ) {
		gettimeofday(&cur, NULL);
		count++;
		if( cur.tv_sec - start.tv_sec > 600 ) {
			break;
		}

		len = read(device, buf, sizeof(buf));

		if ( len >= HCI_EVENT_HDR_SIZE ) {
			meta_event = (evt_le_meta_event*)(buf+HCI_EVENT_HDR_SIZE+1);
			if ( meta_event->subevent == EVT_LE_ADVERTISING_REPORT ) {
				uint8_t reports_count = meta_event->data[0];
				void * offset = meta_event->data + 1;

				while ( reports_count-- ) {
					info = (le_advertising_info *)offset;
					char addr[18];

					ba2str(&(info->bdaddr), addr);

					gettimeofday(&t, NULL);

					memcpy(mybuf.addr, addr, 18);
					mybuf.batt_lv = buf[39];
					mybuf.btn = buf[41];
					mybuf.time = t;
					mybuf.rssi = info->data[info->length] - 256;

			        if((mq_send(mfd, (char *)&mybuf, attr.mq_msgsize, 1)) == -1) {
			            perror("send error");
			            exit(-1);
			        }

			        parse_payload(buf, sizeof(buf), addr, info->data[info->length]-256);

					offset = info->data + info->length + 2;
				}
			}
		}
	}
	printf("%d\n", count);
	////////////////////////////////////////////////////////////////////////////////////////////////////
	// Disable scanning.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	memset(&scan_cp, 0, sizeof(scan_cp));
	scan_cp.enable = 0x00;	// Disable flag.
	
	struct hci_request disable_adv_rq = ble_hci_request(OCF_LE_SET_SCAN_ENABLE, LE_SET_SCAN_ENABLE_CP_SIZE, &status, &scan_cp);
	ret = hci_send_req(device, &disable_adv_rq, 1000);
	if ( ret < 0 ) {
		hci_close_dev(device);
		perror("Failed to disable scan.");
		return 0;
	}

	hci_close_dev(device);

	return 0;

}
