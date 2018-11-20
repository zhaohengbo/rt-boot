/**
 */

#include <kernel/rtthread.h>

#include <net/uip/uip.h>

void bootbreak_appcall(void) {
    if (uip_udp_conn->lport == HTONS(0x2333)) {
		if (uip_newdata()) {
			if(!rt_memcmp(uip_appdata,"RT-BOOT:ABORT",13))
			{
				uip_send("RT-BOOT:ABORTED",15);
				rt_kprintf("Break Boot:got user-absort-msg from udp socket\n");
			}
		}
		
		/* Finally, return to uIP. Our outgoing packet will soon be on its
		 way... */
		return;
	}
}

/*-----------------------------------------------------------------------------------*/
void bootbreak_init(void) {
	struct uip_udp_conn *bootbreak_conn;
	uip_ipaddr_t addr;
	uip_ipaddr(addr, 255,255,255,255);
	bootbreak_conn = uip_udp_new(&addr,HTONS(0));
	if(bootbreak_conn != RT_NULL)
	{
		uip_udp_bind(bootbreak_conn, HTONS(0x2333));
		register_udp_appcall(bootbreak_appcall);
	}
}
/*-----------------------------------------------------------------------------------*/
/** @} */
/** @} */
