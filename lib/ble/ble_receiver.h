#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
using namespace std;

typedef struct{ 
	struct timeval time;
	bdaddr_t bdaddr;
	uint8_t batt_lv;
	uint8_t btn;
	uint8_t rssi;
} ble_struct;
/* sizeof(my_msg) = 20 */

// HCI_MAX_EVENT_SIZE 260
// HCI_EVENT_HDR_SIZE   2

int open_hci_dev();
void init_ble_scanner(int device, int status);
void close_ble_scanner(int device, int status);

ble_struct interpret_buf(uint8_t* buf, int size);

void print_ble_struct(ble_struct msg);

int check_hyundai_beacon(ble_struct *val);

void print_vec();
bool exist_addr(bdaddr_t bdaddr);
int insert_csv_data(ble_struct *val);
int insert_sending_data(ble_struct *val);
void delete_sending_data();
void delete_sending_data();
char* print_time(timeval val);
void make_csv_files(timeval start, int max_index);
