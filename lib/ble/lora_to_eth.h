#include <stdlib.h>
#include <unistd.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include "util.h"

class lora_to_eth {
private:
	int sock;
	struct ifreq *ifr; // Interface request

	uint8_t seq_num;
	uint8_t gateway_buf[15];
	uint8_t send_buf[255];
	uint8_t mac_address[6];
	int size;

	timeval prev_heartbeat;

public:
	lora_to_eth();
	~lora_to_eth();

	void write(uint8_t type, uint8_t length);
	void make_values(timeval time);
	void append_tlv(uint8_t* buf, int length);
	void append_mark();

	int get_send_buf(uint8_t* buf);

	bool check_heartbeat(timeval* cur, int interval);

	void clean_buf();

	void interpret(uint8_t *buf);
};