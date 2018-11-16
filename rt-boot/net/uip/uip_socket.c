/**
 */

#include <kernel/rtthread.h>

#include <net/uip/uip.h>

void udp_appcall(void) 
{
	rt_kprintf("receive udp frame");
}

void tcp_appcall(void) 
{
	rt_kprintf("receive tcp frame");
}

/*-----------------------------------------------------------------------------------*/

/** @} */
/** @} */
