#include "util.h"

char* print_time(timeval val) {
    static char now[30];
    struct tm *today;
     
    today = localtime(&val.tv_sec);

    sprintf(now, "%02d:%02d:%02d.%06d", today->tm_hour,
										today->tm_min,
										today->tm_sec,
										val.tv_usec);

    return now;
}

void get_bluetooth_mac(uint8_t* mac_address) {
	struct ifreq ifr;
	struct ifconf ifc;
	char buf[1024];
	int success = 0;

	int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (sock == -1) { /* handle error*/ };

	ifc.ifc_len = sizeof(buf);
	ifc.ifc_buf = buf;
	if (ioctl(sock, SIOCGIFCONF, &ifc) == -1) { /* handle error */ }

	struct ifreq* it = ifc.ifc_req;
	const struct ifreq* const end = it + (ifc.ifc_len / sizeof(struct ifreq));

	for (; it != end; ++it) {
		strcpy(ifr.ifr_name, it->ifr_name);
		if (ioctl(sock, SIOCGIFFLAGS, &ifr) == 0) {
			if (! (ifr.ifr_flags & IFF_LOOPBACK)) { // don't count loopback
				if (ioctl(sock, SIOCGIFHWADDR, &ifr) == 0) {
					success = 1;
					break;
				}
			}
		} else { /* handle error */ }
	}

	if (success) memcpy(mac_address, ifr.ifr_hwaddr.sa_data, 6);
}