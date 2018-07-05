#include <sys/time.h>

#include <iostream>
using namespace std;

#include "lib/ble_receiver.h"

int main(int argc, char* argv[]) {
	struct timeval start,cur;
	int len, status;
	uint8_t buf[HCI_MAX_EVENT_SIZE];
	int device = open_hci_dev();
	ble_struct msg;

	int total_time = atoi(argv[1]);

	init_ble_scanner(device, status);

	int count = 0;
	gettimeofday(&start, NULL);
	while ( 1 ) {
		gettimeofday(&cur, NULL);
		if ( cur.tv_sec - start.tv_sec > total_time ) {
			break;
		}
		count++;
		len = read(device, buf, sizeof(buf));
		msg = interpret_buf(buf, len);
		if ( check_hyundai_beacon(&msg) ) {
			insert_data(&msg);
			print_vec();
		}
	}
	cout << count << endl;

	close_ble_scanner(device, status);
	make_csv_files(start, total_time/2);

	return 0;
}