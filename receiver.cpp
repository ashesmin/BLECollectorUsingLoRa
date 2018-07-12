#include <sys/time.h>

#include <iostream>
using namespace std;

#include "ble_receiver.h"
#include "lora.h"

int main(int argc, char* argv[]) {
	uint8_t buf[255];
	int state;

	init_lora((uint8_t)10, (uint32_t)CH_10_900, (char)'H', (uint8_t)1, (uint8_t)0);

	while ( 1 ) {
		state = sx1272.receiveAll();
		if ( state == 0 ) {
			printf("Receive LoRa Packet Success!!\n");
			memcpy(buf, sx1272.packet_received.data, 255);
			print_pkt(buf);
		}
	}

	return 0;
}