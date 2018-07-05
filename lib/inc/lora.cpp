#include "lora.h"

uint8_t dstAddr;

void init_lora(uint8_t p_mode, uint32_t p_channel, char p_power, uint8_t p_srcAddr, uint8_t p_dstAddr)
{
		
		int init_state;

		printf("SX1272 module and Raspberry Pi: send packets without ACK\n");

		init_state = sx1272.ON();
		printf("Setting power ON: state %d\n", init_state);

        init_state |= sx1272.setMode(p_mode);
		printf("Setting Mode: state %d\n", init_state);

		init_state |= sx1272.setHeaderON();
		printf("Setting Header ON: state %d\n", init_state);

		init_state |= sx1272.setChannel(p_channel);
		printf("Setting Channel: state %d\n", init_state);

		init_state |= sx1272.setCRC_ON();
		printf("Setting CRC ON: state %d\n", init_state);

		init_state |= sx1272.setPower(p_power);
		printf("Setting Power: state %d\n", init_state);

		init_state |= sx1272.setNodeAddress(p_srcAddr);
		printf("Setting Node address: state %d\n", init_state);

		dstAddr = p_dstAddr;

		if ( init_state == 0) {
			printf("SX1272 successfully configured\n");
		} else {
			printf("SX1272 initialization failed\n");
		}
}
