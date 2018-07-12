#include <sys/time.h>

#include <iostream>
using namespace std;

#include "ble_receiver.h"
#include "lora.h"

int main(int argc, char* argv[]) {
	struct timeval prev,cur;
	uint8_t buf[HCI_MAX_EVENT_SIZE], send_buf[255];
	uint8_t payload_len;
	int len, status, send_state, count = 0, device = open_hci_dev();
	ble_struct msg;
	int max_count, time_interval;

	max_count = atoi(argv[1]);
	time_interval = atoi(argv[2]);

	init_ble_scanner(device, status);
	init_lora((uint8_t)10, (uint32_t)CH_10_900, (char)'H', (uint8_t)0, (uint8_t)1);

	gettimeofday(&prev, NULL);

	while ( 1 ) {
		len = read(device, buf, sizeof(buf));
		msg = interpret_buf(buf, len);
		if ( check_hyundai_beacon(&msg) ) {
			if ( check_btn_event(&msg) ) {
				payload_len = make_btn_event_pkt(&msg, send_buf);
				send_state = sx1272.sendPacketTimeout((uint8_t)10, send_buf, payload_len);
				cout << "button event generated. send_state = " << send_state << endl;
			} else {
				count++;
				insert_sending_data(&msg);
			}
		}

		gettimeofday(&cur, NULL);
		if ( count == 0 ) {
			if ( check_time(&prev, &cur, 10) ) {
				payload_len = make_heartbeat(send_buf);
				send_state = sx1272.sendPacketTimeout((uint8_t)10, send_buf, payload_len);
				cout << "heartbeat sended. send_state = " << send_state << endl;
			}
		} else {
			if ( check_time(&prev, &cur, time_interval) ) {
				payload_len = make_data_pkt(send_buf);
				send_state = sx1272.sendPacketTimeout((uint8_t)10, send_buf, payload_len);
				cout << count << " scaned ble data send_state = " << send_state << endl;
				count = 0;
			} else if ( count == max_count ) {
				payload_len = make_data_pkt(send_buf);
				send_state = sx1272.sendPacketTimeout((uint8_t)10, send_buf, payload_len);
				cout << "20 scaned ble data sended. send_state = " << send_state << endl;				
				count = 0;
			}
		}
	}

	close_ble_scanner(device, status);

	return 0;
}
