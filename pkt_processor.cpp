#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <mqueue.h>
#include <curses.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>

#include <vector>
#include <fstream>
using namespace std;

typedef struct{ 
	long msgtype;
	char addr[18];
	uint8_t batt_lv;
	uint8_t btn;
	struct timeval time;
	uint8_t rssi;
}  my_msg;
/* sizeof(my_msgbuf) = 36 */

vector<vector<my_msg>> v;
vector<my_msg>::iterator vi;
vector<vector<my_msg>>::iterator vvi;

int parse_payload(char* addr, uint8_t batt_lv, uint8_t btn, struct timeval time, uint8_t rssi) {
	printf("ADDRESS : %s, RSSI : %d, BATTERY_LV : %03d, BUTTON : %02X\n", addr, rssi-256, batt_lv, btn);

	return 0;
}

void print_vec() {
	for(vvi = v.begin() ; vvi != v.end() ; vvi++ ) {
		vector<my_msg>::iterator last_1_vi = vvi->end()-2;
		vector<my_msg>::iterator last_vi = vvi->end()-1;
		struct timeval end;
		gettimeofday(&end, NULL);
		double diff = last_vi->time.tv_sec + last_vi->time.tv_usec / 1000000.0 - last_1_vi->time.tv_sec - last_1_vi->time.tv_usec / 1000000.0;
		double diff2 = end.tv_sec + end.tv_usec / 1000000.0 - last_vi->time.tv_sec - last_vi->time.tv_usec / 1000000.0;

		if(vvi->size() < 2) {
			diff = 0;
		}
		printf("ADDR: %s,\tCOUNT: %d,\t(LAST) RSSI: %d,\tINTERVAL: %.4f,\tCUR-LAST: %.4f,\tBATT_LV: %03d,\tBUTTON: %02X\n", (vvi->begin())->addr, vvi->size(), last_vi->rssi-256, diff, diff2, last_vi->batt_lv, last_vi->btn);
	}
	printf("TOTAL DEVICES : %d\n\n", v.size());
}

bool exist_addr(char* addr) {
	for(vvi = v.begin() ; vvi != v.end() ; vvi++ ) {
		if(!strcmp((vvi->begin())->addr, addr)) {
			return true;
		}			
	}
	return false;
}

int insert_data(my_msg *val) {
	my_msg tmp_struct;
	memcpy(&tmp_struct, val, 36);

	if(exist_addr(val->addr)) {
		(vvi)->push_back(tmp_struct);

		return 1;
	} else {
		vector<my_msg> tmp_v;

		tmp_v.push_back(tmp_struct);
		v.push_back(tmp_v);

		return 0;
	}
}

int check_hyundai_beacon(my_msg *val) {
	bool chk_addr, chk_batt_lv, chk_btn;

	char sub_addr[9];
	strncpy(sub_addr, val->addr, 9);
	sub_addr[9] = '\0';	

	chk_addr = !strcmp(sub_addr, "50:8D:6F:");

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

void sig_handler(int signo) {
    printf("Data stored test.csv...\n");
 	/*
    ofstream outFile("test.scv");
    for(vvi = v.begin() ; vvi != v.end() ; vvi++) {

    }
    outFile << 
	*/

 	exit(0);
}

int main() {
    signal(SIGINT, sig_handler);

	struct mq_attr attr;
	attr.mq_maxmsg = 1000;
	attr.mq_msgsize = 36;
	my_msg val;

	mqd_t mfd;

	mfd = mq_open("/my_mq", O_RDWR | O_CREAT,  0666, &attr);
    if (mfd == -1) {
        perror("open error");
        exit(0);
    }	

    while(1) {
        if((mq_receive(mfd, (char *)&val, attr.mq_msgsize,NULL)) == -1) {
			exit(-1);
		}
		if(check_hyundai_beacon(&val)) {
			//parse_payload(val.addr, val.batt_lv, val.btn, val.time, val.rssi);
			insert_data(&val);
			print_vec();
		}
    }

    return 0;
}