#include "ble_to_lora.h"

ble_to_lora::ble_to_lora() {
	seq_num = 0;
}

ble_to_lora::ble_to_lora(uint8_t p_mode, uint32_t p_channel, char p_power, uint8_t p_srcAddr, uint8_t p_dstAddr) {
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

	seq_num = 0;
}

void ble_to_lora::write(uint8_t type, uint8_t length) {
	reader_buf[0] = type;
	reader_buf[1] = length;
	size = length;
}

void ble_to_lora::make_values(uint8_t* reader_mac, timeval time) {
	memcpy(reader_buf + 2, reader_mac, 6);
	reader_buf[6] = seq_num++;

	memcpy(reader_buf + 9, &(time.tv_sec), 4);

	short usec = time.tv_usec / 100;
	memcpy(reader_buf + 13, &usec , 2);

	memcpy(send_buf, reader_buf, 15);
}

void ble_to_lora::append_tlv(uint8_t* buf, int length) {
	memcpy(send_buf + 15, buf, length);
	size = 15 + length;
	send_buf[1] = size;
}

int ble_to_lora::get_send_buf(uint8_t* buf) {
	memcpy(buf, send_buf, size);
	return size;
}

void ble_to_lora::clean_buf() {
	memset(send_buf, 0, size);
	size = 0;
}