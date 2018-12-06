/**
 */

#include <kernel/rtthread.h>

#include <net/lwip/lwip/sockets.h>
#include <net/lwip/lwip/udp.h>

#include <main/main.h>

static void breakd_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *recv_addr, u16_t port)
{
	struct pbuf *q;
	if (p->len == 13)
	{
		if(!rt_memcmp((char *)p->payload,"RT-BOOT:ABORT",13))
		{
			q = pbuf_alloc(PBUF_TRANSPORT, 15, PBUF_RAM);
			rt_memcpy((char *)q->payload,"RT-BOOT:ABORTED",15);
			if(q != RT_NULL)
			{
				board_break_net_notisfy();
				//rt_kprintf("Break Boot:got user-absort-msg from udp socket\n");
				udp_sendto(pcb, q, recv_addr, port);
				pbuf_free(q);
			}
		}
	}
	pbuf_free(p);
}
/*-----------------------------------------------------------------------------------*/
void break_server(void) {
	struct udp_pcb *pcb;
	pcb = udp_new();
    if (pcb == NULL)
    {
        return;
    }

    ip_set_option(pcb, SOF_BROADCAST);
    udp_bind(pcb, IP_ADDR_ANY, htons(0x2333));
    udp_recv(pcb, breakd_recv, 0);
}
