#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <net/if.h> 
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>

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
void cal_diff_time(timeval val);
char* print_time(timeval val);
void make_csv_files(timeval start, int max_index);

void get_bluetooth_mac(uint8_t* mac_address);

bool check_time(timeval* prev, timeval* cur, int diff);
bool check_btn_event(ble_struct *val);

void convert_struct_to_barray(ble_struct* msg, uint8_t* barray);
void convert_barray_to_struct(ble_struct* msg, uint8_t* barray);
uint8_t make_heartbeat(uint8_t* buf);
uint8_t make_data_pkt(uint8_t* buf);
uint8_t make_btn_event_pkt(ble_struct* msg, uint8_t* buf);

uint8_t get_pkt_flag(uint8_t* buf);
uint8_t get_length(uint8_t* buf);
void get_reader_mac(uint8_t* buf, uint8_t* addr);

void print_pkt(uint8_t* buf);