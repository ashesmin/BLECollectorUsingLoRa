#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#define T_GENERAL_DATA 		0x20
#define T_SPECIAL_DATA 		0x21

#define R_HEARTBEAT 		0x40
#define R_DATA_FRAME 		0x44
#define R_ACK_FRAME 		0x45

#define G_HEARTBEAT 		0x80
#define G_DATA_FRAME		0x84
#define G_ATHEN_REQUEST  	0x88
#define G_ATHEN_RESPONSE	0x89

char* print_time(timeval val);
void get_bluetooth_mac(uint8_t* mac_address);