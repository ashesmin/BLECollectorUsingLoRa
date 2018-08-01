#include "lora_to_eth.h"

lora_to_eth::lora_to_eth() {
	struct ifconf ifc;
	int numif;

	memset(&ifc, 0, sizeof(ifc));
	ifc.ifc_ifcu.ifcu_req = NULL;
	ifc.ifc_len = 0;

	sock = socket( PF_INET, SOCK_DGRAM, 0 );
	ioctl(sock, SIOCGIFCONF, &ifc);

	if ((ifr = (ifreq*)malloc(ifc.ifc_len)) == NULL) {
	} else {
		ifc.ifc_ifcu.ifcu_req = ifr;
		ioctl(sock, SIOCGIFCONF, &ifc);
	}

 	numif = ifc.ifc_len / sizeof(struct ifreq);
 	for ( int i = 0 ; i < numif ; i++ ) {
 		struct ifreq *r = &ifr[i];
 		struct sockaddr_in *sin = (struct sockaddr_in *)&r->ifr_addr;
 		if ( !strcmp(r->ifr_name, "lo") ) {
 			continue;
 		}

 		ioctl(sock, SIOCGIFHWADDR, r);

 		char mac_addr[100];
 		sprintf(mac_addr, "[%s] %02X:%02X:%02X:%02X:%02X:%02X", r->ifr_name,
				(unsigned char)r->ifr_hwaddr.sa_data[0],
				(unsigned char)r->ifr_hwaddr.sa_data[1],
				(unsigned char)r->ifr_hwaddr.sa_data[2],
				(unsigned char)r->ifr_hwaddr.sa_data[3],
				(unsigned char)r->ifr_hwaddr.sa_data[4],
				(unsigned char)r->ifr_hwaddr.sa_data[5]);

 		memcpy(mac_address , r->ifr_hwaddr.sa_data , 6);

 		printf("%s\n\n", mac_addr);
 	}

 	seq_num = 0;
	size = 0;

	gettimeofday(&prev_heartbeat, NULL);
}

lora_to_eth::~lora_to_eth() {
	close(sock);
	free(ifr);
}

void lora_to_eth::write(uint8_t type, uint8_t length) {
	gateway_buf[0] = type;
	gateway_buf[1] = length;
	size = length+2;
}

void lora_to_eth::make_values(timeval time) {
	memcpy(gateway_buf + 2, mac_address, 6);
	memcpy(gateway_buf + 8, &(time.tv_sec), 4);

	short usec = time.tv_usec / 100;
	memcpy(gateway_buf + 12, &usec , 2);

	gateway_buf[14] = seq_num++;

	memcpy(send_buf+1, gateway_buf, size);
}

void lora_to_eth::append_tlv(uint8_t* buf, int length) {
	memcpy(send_buf + 16, buf, length);
	size += length;
}

void lora_to_eth::append_mark() {
	send_buf[0] = 0x7e;
	send_buf[size+1] = 0x7e;
	size += 2;
}

int lora_to_eth::get_send_buf(uint8_t* buf) {
	memcpy(buf, send_buf, size);
	return size;
}

bool lora_to_eth::check_heartbeat(timeval* cur, int interval) {
	if ( (cur->tv_sec - prev_heartbeat.tv_sec) >= interval ) {
		prev_heartbeat.tv_sec = cur->tv_sec;
		prev_heartbeat.tv_usec = cur->tv_usec;

		return true;
	}
	return false;	
}

void lora_to_eth::clean_buf() {
	memset(send_buf, 0, size);
	size = 0;
}

void lora_to_eth::interpret(uint8_t *buf) {

}