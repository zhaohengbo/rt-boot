/**
 */

#ifndef __UIP_SOCKET_H__
#define __UIP_SOCKET_H__

#include <net/uip/uipopt.h>

void udp_appcall(void);
void tcp_appcall(void);

struct tcp_state {
	u8_t state;
	u16_t count;
};

#ifndef UIP_APPCALL
#define UIP_APPCALL		tcp_appcall
#endif

#ifndef UIP_APPSTATE_SIZE
#define UIP_APPSTATE_SIZE (sizeof(struct tcp_state))
#endif

#endif /* __UIP_H__ */


/** @} */

