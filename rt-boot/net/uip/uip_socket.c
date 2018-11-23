/**
 */

#include <kernel/rtthread.h>

#include <net/uip/uip.h>

struct socket_appcall_info {
	void (*socket_appcall)(void);
	u8_t valid :1;
};

static struct socket_appcall_info udp_appcall_list[UIP_UDP_CONNS];
static struct socket_appcall_info tcp_appcall_list[UIP_CONNS];

rt_int32_t register_udp_appcall(void (*callback)(void)) {
	rt_int32_t i;
	for (i = 0; i < UIP_UDP_CONNS; i++) {
		if (!udp_appcall_list[i].valid) {
			udp_appcall_list[i].valid = 1;
			udp_appcall_list[i].socket_appcall = callback;
			return i;
		}
	}
	return -1;
}

rt_int32_t register_tcp_appcall(void (*callback)(void)) {
	rt_int32_t i;
	for (i = 0; i < UIP_CONNS; i++) {
		if (!tcp_appcall_list[i].valid) {
			tcp_appcall_list[i].valid = 1;
			tcp_appcall_list[i].socket_appcall = callback;
			return i;
		}
	}

	return -1;
}

void udp_appcall(void) {
	rt_int32_t i;
	for (i = 0; i < UIP_UDP_CONNS; i++) {
		if (udp_appcall_list[i].valid
				&& (udp_appcall_list[i].socket_appcall != (void *) 0))
			udp_appcall_list[i].socket_appcall();
	}
}

void tcp_appcall(void) {
	rt_int32_t i;
	for (i = 0; i < UIP_CONNS; i++) {
		if (tcp_appcall_list[i].valid
				&& (tcp_appcall_list[i].socket_appcall != (void *) 0))
			tcp_appcall_list[i].socket_appcall();
	}
}

/*-----------------------------------------------------------------------------------*/

/** @} */
/** @} */
