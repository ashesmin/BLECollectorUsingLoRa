#include "ble_receiver.h"

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

ble_receiver::ble_receiver() {
	device = hci_open_dev(hci_get_route(NULL));
	if ( device < 0 ) { 
		perror("Failed to open HCI device.");

		exit(0);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// Set BLE scan parameters.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	memset(&scan_params_cp, 0, sizeof(scan_params_cp));
	scan_params_cp.type 			= 0x00; 
	scan_params_cp.interval 		= htobs(0x0BB8);
	scan_params_cp.window 			= htobs(0x0BB8);
	scan_params_cp.own_bdaddr_type 	= 0x00; // Public Device Address (default).
	scan_params_cp.filter 			= 0x00; // Accept all.

	struct hci_request scan_params_rq = ble_hci_request(OCF_LE_SET_SCAN_PARAMETERS, LE_SET_SCAN_PARAMETERS_CP_SIZE, &status, &scan_params_cp);
	if ( hci_send_req(device, &scan_params_rq, 1000) < 0 ) {
		hci_close_dev(device);
		perror("Failed to set scan parameters data.");
		exit(0);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// Set BLE events report mask.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	memset(&event_mask_cp, 0, sizeof(le_set_event_mask_cp));

	for (int i = 0 ; i < 8 ; i++ ) event_mask_cp.mask[i] = 0xFF;

	struct hci_request set_mask_rq = ble_hci_request(OCF_LE_SET_EVENT_MASK, LE_SET_EVENT_MASK_CP_SIZE, &status, &event_mask_cp);
	if ( hci_send_req(device, &set_mask_rq, 1000) < 0 ) {
		hci_close_dev(device);
		perror("Failed to set event mask.");
		exit(0);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// Enable scanning.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	
	memset(&scan_cp, 0, sizeof(scan_cp));
	scan_cp.enable 		= 0x01;	// Enable flag.
	scan_cp.filter_dup 	= 0x00; // Filtering disabled.

	struct hci_request enable_adv_rq = ble_hci_request(OCF_LE_SET_SCAN_ENABLE, LE_SET_SCAN_ENABLE_CP_SIZE, &status, &scan_cp);
	if ( hci_send_req(device, &enable_adv_rq, 1000) < 0 ) {
		hci_close_dev(device);
		perror("Failed to enable scan.");
		exit(0);
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
		exit(0);
	}

	gettimeofday(&prev_heartbeat, NULL);
	gettimeofday(&prev_general_interval, NULL);
	gettimeofday(&prev_special_interval, NULL);

	g_buf_size = 0, s_buf_size = 0;
}

ble_receiver::~ble_receiver() {
	////////////////////////////////////////////////////////////////////////////////////////////////////
	// Disable scanning.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	memset(&scan_cp, 0, sizeof(scan_cp));
	scan_cp.enable = 0x00;	// Disable flag.
	struct hci_request disable_adv_rq = ble_hci_request(OCF_LE_SET_SCAN_ENABLE, LE_SET_SCAN_ENABLE_CP_SIZE, &status, &scan_cp);
	if ( hci_send_req(device, &disable_adv_rq, 1000) < 0 ) {
		hci_close_dev(device);
		perror("Failed to disable scan.");
		exit(0);
	}

	hci_close_dev(device);
}

int check_hyundai_beacon(uint8_t* bdaddr, uint8_t btn) {

	if ( !(bdaddr[5] == 0x50 && bdaddr[4] == 0x8D && bdaddr[3] == 0x6F) ) {
		return 2;
	}

	if ( btn == 0x00) {
		return 0;
	} else if ( btn == 0x01 || btn == 0x02 ) {
		return 1;
	}
}

uint8_t cal_time_diff(timeval prev) {
	timeval cur;

	gettimeofday(&cur, NULL);

	return (cur.tv_sec - prev.tv_sec)*100 + (cur.tv_usec - prev.tv_usec)/10000;
}

bool ble_receiver::exist_addr() {

	for ( vi = v.begin() ; vi != v.end() ; vi++ ) {
		if( vi->t[0] == tag[0] &&
			vi->t[1] == tag[1] &&
			vi->t[2] == tag[2] ) {
			return true;
		}
	}
	return false;
}

void ble_receiver::read_tag() {
	int size = read(device, buf, sizeof(buf));

	struct timeval cur;
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

	if ( size >= HCI_EVENT_HDR_SIZE ) {
		meta_event = (evt_le_meta_event*)(buf+HCI_EVENT_HDR_SIZE+1);
		if ( meta_event->subevent == EVT_LE_ADVERTISING_REPORT ) {
			uint8_t reports_count = meta_event->data[0];
			void * offset = meta_event->data + 1;

			info = (le_advertising_info *)offset;

			//tag_flag = check_hyundai_beacon(info->bdaddr.b, buf[41]);
			tag_flag = 0;
			
			tag[0] = info->bdaddr.b[2];
			tag[1] = info->bdaddr.b[1];
			tag[2] = info->bdaddr.b[0];
			tag[3] = buf[39]; /* batt_lv */
			tag[4] = info->data[info->length]; /* rssi */

			if ( tag_flag == 0 ) {
				tag[5] = cal_time_diff(prev_general_interval); /* t_offset */	
			} else if ( tag_flag == 1 ) {
				tag[5] = cal_time_diff(prev_special_interval); /* t_offset */	
			}
			
			if ( !exist_addr() ) {
				tag[6] = 0;
				T tmp;
				memcpy(tmp.t, tag, 7);
				v.push_back(tmp);
			} else {
				if( tag[5] - vi->t[5] > 200 ) {
					tag[6] = 1;
				} else {
					tag[6] = 0;						
				}
				vi->t[5] = tag[5];
			}

			if ( tag_flag == 1 ) {
				memcpy(tag+6, tag+5, 2);
				tag[5] = buf[41]; /* btn */
			} else if ( tag_flag == 2 ) {
				memset(tag, 0, 8);
			}

			if ( tag_flag == 0 ) {
				if ( g_buf_size == 0 ) {
					g_buf[0] = T_GENERAL_DATA;
				}

				memcpy(g_buf + 2 + g_buf_size, tag, 7);
				g_buf_size += 7;
				g_buf[1] = g_buf_size;

			} else if ( tag_flag == 1 ) {
				if ( s_buf_size == 0 ) {
					s_buf[0] = T_SPECIAL_DATA;
				}

				memcpy(s_buf + 2 + s_buf_size, tag, 8);
				s_buf_size += 8;
				s_buf[1] = s_buf_size;
			}			
		}
	}	
}

void ble_receiver::print_tag() {
	if ( tag_flag == 0 ) {
		printf("ADDRESS : %02X:%02X:%02X , RSSI : %d, BATTERY_LV : %03d, BUTTON : 0, T_OFFSET : %d, D_OFFSET : %d\n",
			tag[0], tag[1], tag[2], tag[4]-256, tag[3], tag[5], tag[6]);
	} else if ( tag_flag == 1 ) {
		printf("ADDRESS : %02X:%02X:%02X , RSSI : %d, BATTERY_LV : %03d, BUTTON : %02X, T_OFFSET : %d, D_OFFSET : %d\n",
			tag[0], tag[1], tag[2], tag[4]-256, tag[3], tag[5], tag[6], tag[7]);
	} else {
		printf("It isn't hyundai tag!\n");
	}
}

int ble_receiver::get_general_buf(uint8_t* buf, timeval* time) {
	time->tv_sec = prev_general_interval.tv_sec;
	time->tv_usec = prev_general_interval.tv_usec;
	g_buf_size +=2;
	memcpy(buf, g_buf, g_buf_size);
	return g_buf_size;
}

int ble_receiver::get_special_buf(uint8_t* buf, timeval* time) {
	time->tv_sec = prev_special_interval.tv_sec;
	time->tv_usec = prev_special_interval.tv_usec;
	s_buf_size +=2;
	memcpy(buf, s_buf, s_buf_size);
	return s_buf_size;
}

void ble_receiver::get_heartbeat_time(timeval* time) {
	time->tv_sec = prev_heartbeat.tv_sec;
	time->tv_usec = prev_heartbeat.tv_usec;
}

bool ble_receiver::check_general_interval(timeval* cur, int interval) {
	if ( (cur->tv_sec - prev_general_interval.tv_sec) >= interval ) {
		prev_general_interval.tv_sec = cur->tv_sec;
		prev_general_interval.tv_usec = cur->tv_usec;

		return true;
	}
	return false;	
}

bool ble_receiver::check_general_count(int count) {
	if ( g_buf_size >= count * 7 ) {
		return true;
	}
	return false;
}

bool ble_receiver::check_special_interval(timeval* cur, int interval) {
	if ( (cur->tv_sec - prev_special_interval.tv_sec) >= interval && s_buf_size > 0 ) {
		prev_special_interval.tv_sec = cur->tv_sec;
		prev_special_interval.tv_usec = cur->tv_usec;

		return true;
	}
	return false;	
}

bool ble_receiver::check_special_count(int count) {
	if ( s_buf_size >= count * 8 ) {
		return true;
	}
	return false;
}

bool ble_receiver::check_heartbeat(timeval* cur, int interval) {
	if ( (cur->tv_sec - prev_heartbeat.tv_sec) >= interval ) {
		prev_heartbeat.tv_sec = cur->tv_sec;
		prev_heartbeat.tv_usec = cur->tv_usec;

		return true;
	}
	return false;	
}

void ble_receiver::clean_g_buf() {
	memset(g_buf, 0, g_buf_size);
	g_buf_size = 0;
}

void ble_receiver::clean_s_buf() {
	memset(s_buf, 0, s_buf_size);
	s_buf_size = 0;
}
