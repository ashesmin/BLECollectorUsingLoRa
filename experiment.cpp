#include <sys/time.h>

#include <iostream>
using namespace std;

#include "ble_receiver.h"

int main(int argc, char* argv[]) {
	struct timeval cur, start;
	int len, status;
	uint8_t buf[HCI_MAX_EVENT_SIZE];
	int device = open_hci_dev();
	ble_struct msg;
	int count = 0;

	int total_time = atoi(argv[1]);

	init_ble_scanner(device, status);
 
	gettimeofday(&start, NULL);
	while ( 1 ) {
		gettimeofday(&cur, NULL);
		if ( cur.tv_sec - start.tv_sec > total_time ) {
			break;
		}

		len = read(device, buf, sizeof(buf));
		msg = interpret_buf(buf, len);
		if ( check_hyundai_beacon(&msg) ) {
			insert_csv_data(&msg);
			print_vec();
		}
	}

	close_ble_scanner(device, status);
	make_csv_files(start, total_time/2);

	return 0;
}
