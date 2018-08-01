#include "ble_receiver.h"
#include "ble_to_lora.h"
#include "util.h"

int main(int argc, char* argv[]) {
	ble_receiver ble;
	ble_to_lora btol((uint8_t)10, (uint32_t)CH_10_900, (char)'H', (uint8_t)0, (uint8_t)1);

	int max_count, time_interval, buf_len, send_state;
	uint8_t buf[255], reader_mac[6];

	if ( argc != 3 ) {
		printf("Usage : test.cpp_exe [max_count] [time_interval]\n");
		exit(0);
	}

	max_count = atoi(argv[1]);
	time_interval = atoi(argv[2]);

	get_bluetooth_mac(reader_mac);

	timeval cur, time;

	while ( 1 ) {
		ble.read_tag();
		//ble.print_tag();

		gettimeofday(&cur, NULL);
		if ( ble.check_heartbeat(&cur, 10) ) {
			ble.get_heartbeat_time(&time);

			btol.write(R_HEARTBEAT, 13);
			btol.make_values(reader_mac, time);

			printf("%s \t/ ", print_time(time));
			printf("Send Heartbeat\t / send_state = %d \t / buf_len = %d \n", send_state, 15);
		}
		if ( ble.check_general_interval(&cur, time_interval) ) {
			buf_len = ble.get_general_buf(buf, &time);

			btol.write(R_DATA_FRAME, 13);
			btol.make_values(reader_mac, time);
			btol.append_tlv(buf, buf_len);
			buf_len = btol.get_send_buf(buf);
			send_state = sx1272.sendPacketTimeout((uint8_t)10, buf, buf_len);

			printf("%s \t/ ", print_time(time));
			printf("Send general interval\t / send_state = %d \t / buf_len = %d \n", send_state, buf_len);

			btol.clean_buf();
			ble.clean_g_buf();
		}
		if ( ble.check_special_interval(&cur, time_interval) ) {
			buf_len = ble.get_special_buf(buf, &time);

			btol.write(R_DATA_FRAME, 13);
			btol.make_values(reader_mac, time);
			btol.append_tlv(buf, buf_len);
			buf_len = btol.get_send_buf(buf);
			send_state = sx1272.sendPacketTimeout((uint8_t)10, buf, buf_len);

			printf("%s \t/ ", print_time(time));
			printf("Send special interval\t / send_state = %d \t / buf_len = %d \n", send_state, buf_len);

			btol.clean_buf();
			ble.clean_s_buf();
		}
	}
	return 0;
}