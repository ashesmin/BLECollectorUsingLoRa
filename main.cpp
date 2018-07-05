#include <sys/time.h>

#include <iostream>
using namespace std;

#include "ble_receiver.h"
#include "lora.h"

int main(int argc, char* argv[]) {
	struct timeval start,cur;
	int len, status;
	uint8_t buf[HCI_MAX_EVENT_SIZE];
	uint8_t send_buf[255];
	int device = open_hci_dev();
	int send_state;
	ble_struct msg;

	int total_time = atoi(argv[1]);

	init_ble_scanner(device, status);
	init_lora((uint8_t)1, (uint32_t)CH_10_900, (char)'H', (uint8_t)1, (uint8_t)0);

	int count = 0, total_count = 0;
	gettimeofday(&start, NULL);
	while ( 1 ) {
		gettimeofday(&cur, NULL);
		if ( cur.tv_sec - start.tv_sec > total_time ) {
			break;
		}
		len = read(device, buf, sizeof(buf));
		msg = interpret_buf(buf, len);
		if ( check_hyundai_beacon(&msg) ) {
			count++;
			total_count++;
			insert_csv_data(&msg);
			insert_sending_data(&msg);
			//print_vec();
			if ( count == 30 ) {
				count = 0;
				delete_sending_data();
				send_state = sx1272.sendPacketTimeout((uint8_t)1, send_buf, 255);
				cout << "send_state = " << send_state << endl;
			}
		}
	}
	cout << total_count << endl;

	close_ble_scanner(device, status);
	make_csv_files(start, total_time/2);

	return 0;
}
