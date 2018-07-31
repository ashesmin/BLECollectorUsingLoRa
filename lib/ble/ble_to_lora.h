#include "arduPiLoRa.h"

#include "util.h"

class ble_to_lora {
private:
	uint8_t dstAddr, seq_num;
	uint8_t reader_buf[15];
	uint8_t send_buf[255];
	int size;

public:
	ble_to_lora();
	ble_to_lora(uint8_t p_mode, uint32_t p_channel, char p_power, uint8_t p_srcAddr, uint8_t p_dstAddr);

	void write(uint8_t type, uint8_t length);
	void make_values(uint8_t* reader_mac, timeval time);
	void append_tlv(uint8_t* buf, int length);
	
	int get_send_buf(uint8_t* buf);

	void clean_buf();
};