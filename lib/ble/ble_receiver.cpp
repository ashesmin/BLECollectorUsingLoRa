#include "ble_receiver.h"

le_set_scan_enable_cp scan_cp;

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

int open_hci_dev() {
	int device = hci_open_dev(hci_get_route(NULL));
	if ( device < 0 ) { 
		perror("Failed to open HCI device.");

		exit(0);
	}
	return device;
}

void init_ble_scanner(int device, int status) {
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
	le_set_event_mask_cp event_mask_cp;
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
}

ble_struct interpret_buf(uint8_t* buf, int size) {
	ble_struct msg;
	struct timeval t;
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

			while ( reports_count-- ) {
				info = (le_advertising_info *)offset;
				gettimeofday(&t, NULL);

				msg.bdaddr = info->bdaddr;
				msg.rssi = info->data[info->length] - 128;
				msg.batt_lv = buf[39];
				msg.btn = buf[41];
				msg.time = t;

				offset = info->data + info->length + 2;
			}
		}
	}

	return msg;
}

void close_ble_scanner(int device, int status) {
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

void print_ble_struct(ble_struct msg) {
	char addr[18];

	ba2str(&(msg.bdaddr), addr);
	printf("ADDRESS : %s , RSSI : %d, BATTERY_LV : %03d, BUTTON : %02X\n", addr, msg.rssi, msg.batt_lv, msg.btn);
}

int check_hyundai_beacon(ble_struct *val) {
	bool chk_addr, chk_batt_lv, chk_btn;

	//printf("%02X %02X %02X\n", val->bdaddr.b[6], val->bdaddr.b[1], val->bdaddr.b[2]);
	if(val->bdaddr.b[5] == 0x50 && val->bdaddr.b[4] == 0x8D && val->bdaddr.b[3] == 0x6F ) {
		chk_addr = true;
	} else {
		chk_addr = false;
	}

	if(0 <= val->batt_lv && val->batt_lv <= 100) { 
		chk_batt_lv = true;
	} else {
		chk_batt_lv = false;
	}

	if(val->btn == 0x00 || val->btn == 0x01 || val->btn == 0x02) {
		chk_btn = true;	
	} else {
		chk_btn = false;
	}

	/*
	printf("%d , %d , %d \n", chk_addr, chk_batt_lv, chk_btn);
	*/

	return chk_addr && chk_batt_lv && chk_btn;
}

vector<vector<ble_struct>> v;
vector<ble_struct>::iterator vi;
vector<vector<ble_struct>>::iterator vvi;

vector<ble_struct> s_v;
vector<ble_struct>::iterator s_vi;

void print_vec() {
	for(vvi = v.begin() ; vvi != v.end() ; vvi++ ) {
		vector<ble_struct>::iterator last_1_vi = vvi->end()-2;
		vector<ble_struct>::iterator last_vi = vvi->end()-1;
		struct timeval end;
		gettimeofday(&end, NULL);
		double diff = last_vi->time.tv_sec + last_vi->time.tv_usec / 1000000.0 - last_1_vi->time.tv_sec - last_1_vi->time.tv_usec / 1000000.0;
		double diff2 = end.tv_sec + end.tv_usec / 1000000.0 - last_vi->time.tv_sec - last_vi->time.tv_usec / 1000000.0;

		if(vvi->size() < 2) {
			diff = 0;
		}

		char addr[18];
		ba2str(&((vvi->begin())->bdaddr), addr);

		printf("ADDR: %s,\tCOUNT: %d,\t(LAST) RSSI: %d,\tINTERVAL: %.4f,\tCUR-LAST: %.4f,\tBATT_LV: %03d,\tBUTTON: %02X\n", addr, vvi->size(), last_vi->rssi-128, diff, diff2, last_vi->batt_lv, last_vi->btn);
	}
	printf("TOTAL DEVICES : %d\n\n", v.size());
}

bool exist_addr(bdaddr_t bdaddr) {
	for(vvi = v.begin() ; vvi != v.end() ; vvi++ ) {
		if(vvi->begin()->bdaddr.b[0] == bdaddr.b[0] &&
			vvi->begin()->bdaddr.b[1] == bdaddr.b[1] &&
			vvi->begin()->bdaddr.b[2] == bdaddr.b[2] &&
			vvi->begin()->bdaddr.b[3] == bdaddr.b[3] &&
			vvi->begin()->bdaddr.b[4] == bdaddr.b[4] &&
			vvi->begin()->bdaddr.b[5] == bdaddr.b[5] ) {
			return true;
		}
	}
	return false;
}

int insert_csv_data(ble_struct *val) {
	ble_struct tmp_struct;
	memcpy(&tmp_struct, val, 20);

	if(exist_addr(val->bdaddr)) {
		(vvi)->push_back(tmp_struct);

		return 1;
	} else {
		vector<ble_struct> tmp_v;

		tmp_v.push_back(tmp_struct);
		v.push_back(tmp_v);

		return 0;
	}
}

int insert_sending_data(ble_struct *val) {
	ble_struct tmp_struct;
	memcpy(&tmp_struct, val, 20);

	s_v.push_back(tmp_struct);

	return 1;
}

void delete_sending_data() { s_v.clear(); }

void cal_diff_time(timeval val) {
	int i = 0;
	for ( s_vi = s_v.begin() ; s_vi != s_v.end() ; s_vi++ ) {
		i++; 
		int mili_sec = (s_vi->time.tv_usec - val.tv_usec)/10000;
		cout << "index : " << i << "\tdiff_sec : " << s_vi->time.tv_sec - val.tv_sec << "\t\tdiff_mili_sec : " << mili_sec << endl;
	}
	cout << endl;
}

char* print_time(timeval val) {
    static char now[30];
    struct tm *today;
     
    today = localtime(&val.tv_sec);
    /*
    sprintf(now, "%04d-%02d-%02d %02d:%02d:%02d.%06d", today->tm_year+1900,
                                                       today->tm_mon+1,
                                                       today->tm_mday,
                                                       today->tm_hour,
                                                       today->tm_min,
                                                       today->tm_sec,
                                                       val.tv_usec);
	*/
    sprintf(now, "%02d:%02d:%02d..%03d", today->tm_hour,
										today->tm_min,
										today->tm_sec,
										val.tv_usec);

    return now;
}

void make_csv_files(timeval start, int max_index) {
	ofstream file;
	struct tm *newtime;

	struct stat st;
	int fi = stat("./log", &st);

	if ( fi == -1 ) {
		if ( errno == 2 ) {
			mkdir("./log", 0755);
		}
	}

	for (vvi = v.begin() ; vvi != v.end() ; vvi++ ) {
		char title[40] = "./log/tag_";
		char addr[18];
		ba2str(&((vvi->begin())->bdaddr), addr);
		strcat(title, addr);
		strcat(title, ".csv");
		file.open(title);

		vi = vvi->begin();
		for (int i = 0; i < max_index ; i++) {
			char btn = vi->btn + '0';
			char batt_lv[12];
			sprintf(batt_lv, "%d", vi->batt_lv);
			int index = (vi->time.tv_sec - start.tv_sec) / 2;

			file << i << ",";
			if(i == index) {
				file << print_time(vi->time) << ",";
				file << addr << ",";
				file << batt_lv << ",";
				file << btn << ",";
				file << (vi->rssi - 128) << endl;
				vi++;				
			} else {
				file << endl;
			}
		}
		file.close();
	}
}

bool check_time(timeval* prev, timeval* cur, int diff) {
	if ( (cur->tv_sec - prev->tv_sec) >= diff ) {
		prev->tv_sec = cur->tv_sec;
		prev->tv_usec = cur->tv_usec;

		return true;
	}
	return false;
}

bool check_btn_event(ble_struct *val) {
	if ( val->btn ) {
		return true;
	}
	return false;
}

void get_bluetooth_mac(uint8_t* mac_address) {
    struct ifreq ifr;
    struct ifconf ifc;
    char buf[1024];
    int success = 0;

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock == -1) { /* handle error*/ };

    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;
    if (ioctl(sock, SIOCGIFCONF, &ifc) == -1) { /* handle error */ }

    struct ifreq* it = ifc.ifc_req;
    const struct ifreq* const end = it + (ifc.ifc_len / sizeof(struct ifreq));

    for (; it != end; ++it) {
        strcpy(ifr.ifr_name, it->ifr_name);
        if (ioctl(sock, SIOCGIFFLAGS, &ifr) == 0) {
            if (! (ifr.ifr_flags & IFF_LOOPBACK)) { // don't count loopback
                if (ioctl(sock, SIOCGIFHWADDR, &ifr) == 0) {
                    success = 1;
                    break;
                }
            }
        }
        else { /* handle error */ }
    }

    if (success) memcpy(mac_address, ifr.ifr_hwaddr.sa_data, 6);
}

uint8_t make_heartbeat(uint8_t* buf) {
	uint8_t reader_mac[6];

	memset(buf, 0, 255);
	buf[0] = 0x00;

	get_bluetooth_mac(reader_mac);
	memcpy( &(buf[2]), &reader_mac, sizeof(reader_mac) );

	buf[1] = 8;

	return 8;
}

void convert_struct_to_barray(ble_struct* msg, uint8_t* barray) {
	memcpy( barray, &(msg->bdaddr.b[0]), 3);

	short tmp = 0;

	tmp = msg->batt_lv;
	tmp = tmp << 7;
	tmp |= msg->rssi;
	tmp = tmp << 2;
	tmp |= msg->btn;

	memcpy( barray + 3, &tmp, 2 );
}

void convert_barray_to_struct(ble_struct* msg, uint8_t* barray) {
	msg->bdaddr.b[5] = 0x50;
	msg->bdaddr.b[4] = 0x8D;
	msg->bdaddr.b[3] = 0x6F;
	msg->bdaddr.b[2] = barray[2];
	msg->bdaddr.b[1] = barray[1];
	msg->bdaddr.b[0] = barray[0];

	short tmp;
	memcpy( &tmp, barray + 3 , 2 );

	msg->btn = tmp & 0x03;
	tmp = tmp >> 2;
	msg->rssi = tmp & 0x7F;
	tmp = tmp >> 7;
	msg->batt_lv = tmp & 0x7F;
}

uint8_t make_data_pkt(uint8_t* buf) {
	uint8_t reader_mac[6];
	uint8_t length = 8;

	memset(buf, 0, 255);
	buf[0] = 0x01;

	get_bluetooth_mac(reader_mac);
	memcpy( &(buf[2]), &reader_mac, sizeof(reader_mac) );

	for ( s_vi = s_v.begin() ; s_vi != s_v.end() ; s_vi++ ) {
		convert_struct_to_barray( &(*s_vi), buf + length );
		length += 5;
	}
	delete_sending_data();
	buf[1] = length;

	return length;	
}

uint8_t make_btn_event_pkt(ble_struct* msg, uint8_t* buf) {
	uint8_t reader_mac[6];

	memset(buf, 0, 255);
	buf[0] = 0x01;

	get_bluetooth_mac(reader_mac);
	memcpy( &(buf[2]), &reader_mac, sizeof(reader_mac) );

	convert_struct_to_barray( msg, buf+8 );

	buf[1] = 13 /*length*/;	
	return 13;
}

uint8_t get_pkt_flag(uint8_t* buf) {
	return buf[0];
}
uint8_t get_length(uint8_t* buf) {
	return buf[1];
}
void get_reader_mac(uint8_t* buf, uint8_t* addr) {
	memcpy( addr, &(buf[2]), 6 );
}

void print_pkt(uint8_t* buf) {
	ble_struct msg;
	uint8_t flag = get_pkt_flag(buf);
	uint8_t length = get_length(buf);
	uint8_t reader_mac[6];

	get_reader_mac(buf, reader_mac);
	printf("%d, %d, %02X:%02X:%02X:%02X:%02X:%02X \n", flag, length, reader_mac[0], reader_mac[1], reader_mac[2], reader_mac[3], reader_mac[4], reader_mac[5]);
	int max_index = (length-8)/5;
	if ( max_index > 0) {
		char addr[18];
		for ( int i = 0 ; i < max_index ; i++ ) {
			convert_barray_to_struct(&msg, buf+8+i*5);
			ba2str(&(msg.bdaddr), addr);
			printf("\tBeacon %d - addr : %s, batt_lv : %d, rssi : %d, btn : %d\n", i, addr, msg.batt_lv, msg.rssi-128, msg.btn);
		}
	} else {
		printf("\tHeart Beat Received!");
	}		
}