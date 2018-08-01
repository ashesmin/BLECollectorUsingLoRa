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
	size = length+2;
}

void ble_to_lora::make_values(uint8_t* reader_mac, timeval time) {
	memcpy(reader_buf + 2, reader_mac, 6);
	reader_buf[8] = seq_num++;

	memcpy(reader_buf + 9, &(time.tv_sec), 4);

	short usec = time.tv_usec / 100;
	memcpy(reader_buf + 13, &usec , 2);

	memcpy(send_buf, reader_buf, size);
}

void ble_to_lora::append_tlv(uint8_t* buf, int length) {
	memcpy(send_buf + 15, buf, length);
	size += length;
}

int ble_to_lora::get_send_buf(uint8_t* buf) {
	memcpy(buf, send_buf, size);
	return size;
}

void ble_to_lora::clean_buf() {
	memset(send_buf, 0, size);
	size = 0;
}

void ble_to_lora::print_reader_tag(uint8_t* buf) {
 	timeval cur;
 	short usec;
	int type_usec;

	if ( buf[0] == R_HEARTBEAT ) {
		printf("READER_HEARTBEAT \t/ ");

	 	memcpy(&cur.tv_sec, buf + 9, 4);
	 	memcpy(&usec, buf + 13, 2);
	 	type_usec = usec * 100;
	 	cur.tv_usec = type_usec;

		printf("MAC_ADDRESS : %02X:%02X:%02X:%02X:%02X:%02X \t/ SEQEUNCE_NUM : %d \t/ TIME : %s\n", buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8], print_time(cur));		
	} else if ( buf[0] == R_DATA_FRAME ) {
		printf("READER_DATA_FRMAE \t/ ");

	 	memcpy(&cur.tv_sec, buf + 9, 4);
	 	memcpy(&usec, buf + 13, 2);
	 	type_usec = usec * 100;
	 	cur.tv_usec = type_usec;

		printf("MAC_ADDRESS : %02X:%02X:%02X:%02X:%02X:%02X \t/ SEQEUNCE_NUM : %d \t/ TIME : %s\n", buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8], print_time(cur));
	} else if ( buf[0] == R_ACK_FRAME ) { 
		printf("READER_ACK_FRMAE \t/ ");
	}
}

void ble_to_lora::interpret(uint8_t* buf) {
	print_reader_tag(buf);

	if ( buf[0] == R_DATA_FRAME) {
		int index = 15;
		int size, count;

		if ( buf[index] == T_GENERAL_DATA) {
			size = buf[++index];
			count = size / 7;
			index++;

			for (int i = 0 ; i < count ; i++ ) {
				printf("\t TAG_ADDRESS : %02X:%02X:%02X \t/ BATT_LV : %03d \t/ RSSI : %d \t/ BUTTON : 0 \t/ T_OFFSET : %03d \t/ D_OFFSET : %d\n",
					buf[index+0], buf[index+1], buf[index+2], buf[index+3], buf[index+4]-256, buf[index+5], buf[index+6]);
				index +=7;
			}
		} else if ( buf[index] == T_SPECIAL_DATA ) {
			size = buf[++index];
			count = size / 8;
			index++;

			for (int i = 0 ; i < count ; i++ ) {
				printf("\t TAG_ADDRESS : %02X:%02X:%02X \t/ BATT_LV : %03d \t/ RSSI : %d \t/ BUTTON : %d \t/ T_OFFSET : %03d \t/ D_OFFSET : %d\n",
					buf[index+0], buf[index+1], buf[index+2], buf[index+3], buf[index+4]-256, buf[index+5], buf[index+6], buf[index+7]);
				index +=8;
			}
		}
	}
}