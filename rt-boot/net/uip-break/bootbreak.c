/**
 */

#include <kernel/rtthread.h>

#include <net/uip/uip.h>

void bootbreak_appcall(void) {
    if (uip_udp_conn->lport == HTONS(0x2333)) {
		if (uip_newdata() && (uip_datalen() == 13)) {
			if(!rt_memcmp(uip_appdata,"RT-BOOT:ABORT",13))
			{
				uip_udp_reply("RT-BOOT:ABORTED",15);
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
	bootbreak_conn = uip_udp_new(HTONS(0x2333));
	if(bootbreak_conn != RT_NULL)
	{
		register_udp_appcall(bootbreak_appcall);
	}
}
/*-----------------------------------------------------------------------------------*/
/** @} */
/** @} */
