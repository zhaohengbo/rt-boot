#include <kernel/rtthread.h>

#include <net/uip/uip.h>
#include <net/uip/uip_arp.h>

#include <net/uip-httpd/httpd.h>

#include <board/network.h>

ALIGN(RT_ALIGN_SIZE)
static char uip_thread_stack[0x1000];
struct rt_thread uip_thread;

#define BUF ((struct uip_eth_hdr *)&uip_buf[0])

#define UIP_EVENT_SEND_ID 0

#define UIP_EVENT_RECV_ID 0
#define UIP_EVENT_PEDC_ID 1
#define UIP_EVENT_ARP_ID 2

static void uip_block_send(struct rt_event *event)
{
	rt_uint32_t uip_send_event_flag;
	while(board_eth_send(uip_buf,uip_len) == -1)
	{
		rt_event_recv(event, (1 << UIP_EVENT_SEND_ID),RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR,RT_WAITING_FOREVER, &uip_send_event_flag);
	}
}

/* 定时器1超时函数 */
static void periodic_timer_timeout(void* parameter)
{
	struct rt_event *event = parameter;
    rt_event_send(event, (1 << UIP_EVENT_PEDC_ID));
}

/* 定时器2超时函数 */
static void arp_timer_timeout(void* parameter)
{
	struct rt_event *event = parameter;
    rt_event_send(event, (1 << UIP_EVENT_ARP_ID));
}


static void rt_thread_uip_thread_entry(void *parameter)
{
	int i;
    uip_ipaddr_t ipaddr;
    struct uip_eth_addr eaddr;
	rt_uint32_t length = 0;
	rt_uint32_t uip_event_flag;
	
	struct rt_event uip_event;
	struct rt_event uip_send_event;
	
	rt_timer_t periodic_timer;
	rt_timer_t arp_timer;
	
	periodic_timer = rt_timer_create("uip_periodic_timer",
        periodic_timer_timeout,
        (void *)&uip_event,
        RT_TICK_PER_SECOND/2,
        RT_TIMER_FLAG_PERIODIC);
	
	arp_timer = rt_timer_create("uip_arp_timer",
        arp_timer_timeout,
        (void *)&uip_event,
        RT_TICK_PER_SECOND*20,
        RT_TIMER_FLAG_PERIODIC);
	
	rt_event_init(&uip_event, "uip event", RT_IPC_FLAG_FIFO);
	rt_event_init(&uip_send_event, "uip send event", RT_IPC_FLAG_FIFO);
	
	board_eth_recv_register_event(&uip_event,UIP_EVENT_RECV_ID);
	board_eth_send_register_event(&uip_send_event,UIP_EVENT_RECV_ID);

	uip_init();
	
	eaddr.addr[0] = 22;
    eaddr.addr[1] = 22;
    eaddr.addr[2] = 22;
    eaddr.addr[3] = 22;
    eaddr.addr[4] = 22;
    eaddr.addr[5] = 22;

    // set MAC address
    uip_setethaddr(eaddr);
	
    uip_ipaddr(ipaddr, 192,168,1,1);
    uip_sethostaddr(ipaddr);
    uip_ipaddr(ipaddr, 192,168,1,1);
    uip_setdraddr(ipaddr);
    uip_ipaddr(ipaddr, 255,255,255,0);
    uip_setnetmask(ipaddr);
	
	httpd_init();
	
	if (periodic_timer != RT_NULL)
        rt_timer_start(periodic_timer);
	if (arp_timer != RT_NULL)
        rt_timer_start(arp_timer);
	
	while (1) 
	{
		rt_event_recv(&uip_event, (1 << UIP_EVENT_RECV_ID) | (1 << UIP_EVENT_PEDC_ID) | (1 << UIP_EVENT_ARP_ID),
					  RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR,RT_WAITING_FOREVER, &uip_event_flag);
		
		if(uip_event_flag & (1 << UIP_EVENT_RECV_ID))
		{
			while(1)
			{
				board_eth_recv(uip_buf,&length);
				uip_len = length;
				if(uip_len == 0)
					break;
				else
				{
					if (BUF->type == htons(UIP_ETHTYPE_IP)) 
					{
						uip_arp_ipin();
						uip_input();
						/* If the above function invocation resulted in data that
				 		should be sent out on the network, the global variable
				 		uip_len is set to a value > 0. */
						if (uip_len > 0) 
						{
							uip_arp_out();
                        	uip_block_send(&uip_send_event);
						}
					} 
					else if (BUF->type == htons(UIP_ETHTYPE_ARP)) 
					{
						uip_arp_arpin();
						/* If the above function invocation resulted in data that
				 		should be sent out on the network, the global variable
				 		uip_len is set to a value > 0. */
						if (uip_len > 0) 
						{
                        	uip_block_send(&uip_send_event);
						}
					}
				}
			}
		}
		
		if(uip_event_flag & (1 << UIP_EVENT_PEDC_ID))
		{
			for (i = 0; i < UIP_CONNS; i++) 
			{
				uip_periodic(i);
				/* If the above function invocation resulted in data that
				 should be sent out on the network, the global variable
				 uip_len is set to a value > 0. */
				if (uip_len > 0) 
				{
					uip_arp_out();
                    uip_block_send(&uip_send_event);
				}
			}

#if UIP_UDP
			for(i = 0; i < UIP_UDP_CONNS; i++) 
			{
				uip_udp_periodic(i);
				/* If the above function invocation resulted in data that
				 should be sent out on the network, the global variable
				 uip_len is set to a value > 0. */
				if(uip_len > 0) 
				{
					uip_arp_out();
                    uip_block_send(&uip_send_event);
				}
			}
#endif /* UIP_UDP */
		}
		
		/* Call the ARP timer function every 10 seconds. */
		if(uip_event_flag & (1 << UIP_EVENT_ARP_ID))
		{
			uip_arp_timer();
		}
	}
}

void network_init(void) 
{
    rt_thread_init(&uip_thread,
                   "uip_thread",
                   &rt_thread_uip_thread_entry,
                   RT_NULL,
                   &uip_thread_stack[0],
                   sizeof(uip_thread_stack),7,3);
    rt_thread_startup(&uip_thread);
}

#ifdef UIP_LOGGING

void uip_log(char *m)
{
	rt_kprintf("uIP log message: %s\n", m);
}

#endif
