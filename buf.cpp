#include <sys/time.h>

#include <iostream>
using namespace std;

#include "ble_receiver.h"
#include "lora.h"

int main(int argc, char* argv[]) {
	uint8_t send_buf[255];
	uint8_t mac[6];
	uint8_t flag, length;

	/*
	make_heartbeat(send_buf);

	flag = get_pkt_flag(send_buf);
	length = get_length(send_buf);
	get_reader_mac(send_buf, mac);

	printf("%d \n", flag);
	printf("%d \n", length);
	for ( int i = 0 ; i < 6 ; i++ ) {
		printf("%02X:", mac[i]);
	}
	printf("\n");
	*/

	ble_struct tmp;
	uint8_t src_barray[5];
	ble_struct dst;

	tmp.bdaddr.b[0] = 0xB8;
	tmp.bdaddr.b[1] = 0x27;
	tmp.bdaddr.b[2] = 0xEB;
	tmp.bdaddr.b[3] = 0xFF;
	tmp.bdaddr.b[4] = 0x34;
	tmp.bdaddr.b[5] = 0xB9;

	tmp.batt_lv = 0x64;
	tmp.btn = 0x01;
	tmp.rssi = 0x46;

	convert_struct_to_barray(&tmp, src_barray);

	convert_barray_to_struct(&dst, src_barray);
	
	return 0;
}