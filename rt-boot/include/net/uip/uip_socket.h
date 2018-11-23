/**
 */

#ifndef __UIP_SOCKET_H__
#define __UIP_SOCKET_H__

#include <net/uip/uipopt.h>

void udp_appcall(void);
void tcp_appcall(void);

struct tcp_socket_state {
	void * pdata;
};

struct udp_socket_state {
	void * pdata;
};

typedef struct tcp_socket_state uip_tcp_appstate_t;
typedef struct udp_socket_state uip_udp_appstate_t;

#ifndef UIP_APPCALL
#define UIP_APPCALL		tcp_appcall
#endif

#ifndef UIP_UDP_APPCALL
#define UIP_UDP_APPCALL  udp_appcall
#endif

rt_int32_t register_udp_appcall(void (*callback)(void));
rt_int32_t register_tcp_appcall(void (*callback)(void));

#endif /* __UIP_H__ */

/** @} */

