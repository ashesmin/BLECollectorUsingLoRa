#include "ble_to_lora.h"
#include "lora_to_eth.h"
#include "util.h"

int main(int argc, char* argv[]) {
	uint8_t buf[255];
	int state, size;
	timeval cur;

	lora_to_eth ltoeth;
	ble_to_lora btol((uint8_t)10, (uint32_t)CH_10_900, (char)'H', (uint8_t)1, (uint8_t)0);

	gettimeofday(&cur,NULL);
	// Athentication
	ltoeth.write(G_ATHEN_REQUEST, 13);
	ltoeth.make_values(cur);
	ltoeth.append_mark();
	// send()
	printf("ATHENTICATION Send()\n");
	ltoeth.clean_buf();

	while ( 1 ) {
		state = sx1272.receiveAll();
		if ( state == 0 ) {
			memcpy(buf, sx1272.packet_received.data, 255);
			size = sx1272.packet_received.length-5;
			printf("Receive LoRa Packet Success!! / Buffer Size : %d\n ", size);

			btol.interpret(buf);
			gettimeofday(&cur, NULL);

			ltoeth.write(G_DATA_FRAME, 13);
			ltoeth.make_values(cur);
			ltoeth.append_tlv(buf, size);
			ltoeth.append_mark();
			// send()
			printf("DATA Send()\n");
			ltoeth.clean_buf();

			if ( ltoeth.check_heartbeat(&cur, 10) ) {
				ltoeth.write(G_HEARTBEAT, 13);
				ltoeth.make_values(cur);
				ltoeth.append_mark();
				// send()
				printf("HEARTBEAT Send()\n");
				ltoeth.clean_buf();
			}
		}
	}
}
