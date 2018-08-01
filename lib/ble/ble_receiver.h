#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#include "util.h"

#include <iostream>
#include <vector>
using namespace std;

struct T {
	uint8_t t[8];
};

class ble_receiver {
private:
	int status;
	int device;
	//
	// typedef struct {
	// 		uint8_t type;
	// 		uint16_t interval;
	//		uint16_t window;
	//		uint8_t own_bdaddr_type;
	//		uint8_t filter;
	//	} __attribute__ ((packed)) le_set_scan_parameters_cp;
	le_set_scan_parameters_cp scan_params_cp;
	le_set_scan_enable_cp scan_cp;
	le_set_event_mask_cp event_mask_cp;

	uint8_t tag[8], buf[HCI_MAX_EVENT_SIZE], g_buf[255], s_buf[255];
	uint8_t g_buf_size, s_buf_size;
	int tag_flag;

	vector<T> v;
	vector<T>::iterator vi;

	timeval prev_heartbeat, prev_general_interval, prev_special_interval;

public:
	ble_receiver();
	~ble_receiver();

	void read_tag();
	void print_tag();

	bool exist_addr();

	int get_general_buf(uint8_t* buf, timeval* time);
	int get_special_buf(uint8_t* buf, timeval* time);
	void get_heartbeat_time(timeval* time);

	bool check_general_interval(timeval* cur, int interval);
	bool check_general_count(int count);
	bool check_special_interval(timeval* cur, int interval);
	bool check_special_count(int count);
	bool check_heartbeat(timeval* cur, int interval);

	void clean_g_buf();
	void clean_s_buf();
};
