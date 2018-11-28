
#include <kernel/rtthread.h>
#include <drivers/rtdevice.h>
#include <net/lwip/netif/ethernetif.h>

#include <board/network.h>

#define ETH_EVENT_RECV_ID 0
#define ETH_EVENT_SEND_ID 0

#define MAX_ADDR_LEN 6
struct rt_boot_eth
{
	/* inherit from ethernet device */
	struct eth_device parent;
	rt_uint8_t dev_addr[MAX_ADDR_LEN];
	
	struct rt_event eth_recv_event;
	struct rt_event eth_send_event;
};

static struct rt_boot_eth rt_boot_eth_device;

static rt_err_t rt_boot_eth_init(rt_device_t dev)
{
	return RT_EOK;
}

static rt_err_t rt_boot_eth_open(rt_device_t dev, rt_uint16_t oflag)
{
	return RT_EOK;
}

static rt_err_t rt_boot_eth_close(rt_device_t dev)
{
	return RT_EOK;
}

static rt_size_t rt_boot_eth_read(rt_device_t dev, rt_off_t pos, void* buffer, rt_size_t size)
{
	rt_set_errno(-RT_ENOSYS);
	return 0;
}

static rt_size_t rt_boot_eth_write (rt_device_t dev, rt_off_t pos, const void* buffer, rt_size_t size)
{
	rt_set_errno(-RT_ENOSYS);
	return 0;
}

static rt_err_t rt_boot_eth_control(rt_device_t dev, int cmd, void *args)
{
    switch (cmd)
    {
    case NIOCTL_GADDR:
        if (args) rt_memcpy(args, rt_boot_eth_device.dev_addr, 6);
        else return -RT_ERROR;
        break;

    default :
        break;
    }
    return RT_EOK;
}

rt_err_t rt_boot_eth_tx( rt_device_t dev, struct pbuf* p)
{
	struct pbuf* q;
	rt_uint32_t eth_send_event_flag;
    /* copy frame from pbufs to driver buffers */
    for(q = p; q != NULL; q = q->next)
    {
		while(board_eth_send(q->payload,q->len) == -1)
		{
			rt_event_recv(&rt_boot_eth_device.eth_send_event, (1 << ETH_EVENT_SEND_ID),RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR,RT_WAITING_FOREVER, &eth_send_event_flag);
		}
	}
    return ERR_OK;
}

static rt_uint8_t temp_buffer[1600];

struct pbuf *rt_boot_eth_rx(rt_device_t dev)
{
    struct pbuf *p = NULL;
    //struct pbuf *q = NULL;
	rt_uint32_t length;
	
	board_eth_recv(temp_buffer,&length);
	if(length)
	{
		p = pbuf_alloc(PBUF_RAW, length, PBUF_POOL);
		
		if (p != NULL)
		{
			rt_memcpy(p->payload,temp_buffer,p->len);
		}
	}
	
    return p;
}

ALIGN(RT_ALIGN_SIZE)
static char fake_eth_thread_stack[0x1000];
struct rt_thread fake_eth_thread;

static void rt_boot_fake_eth_thread_entry(void *parameter)
{
	rt_uint32_t eth_recv_event_flag;
	while(1)
	{
		eth_device_ready(&(rt_boot_eth_device.parent));
		rt_event_recv(&rt_boot_eth_device.eth_recv_event, (1 << ETH_EVENT_RECV_ID),RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR,
					  RT_WAITING_FOREVER, &eth_recv_event_flag);
	}
}

int rt_hw_boot_eth_init(void)
{
    rt_err_t state;

    rt_boot_eth_device.parent.parent.init       = rt_boot_eth_init;
    rt_boot_eth_device.parent.parent.open       = rt_boot_eth_open;
    rt_boot_eth_device.parent.parent.close      = rt_boot_eth_close;
    rt_boot_eth_device.parent.parent.read       = rt_boot_eth_read;
    rt_boot_eth_device.parent.parent.write      = rt_boot_eth_write;
    rt_boot_eth_device.parent.parent.control    = rt_boot_eth_control;
    rt_boot_eth_device.parent.parent.user_data  = RT_NULL;

    rt_boot_eth_device.parent.eth_rx     = rt_boot_eth_rx;
    rt_boot_eth_device.parent.eth_tx     = rt_boot_eth_tx;
	
	rt_boot_eth_device.dev_addr[0] = 22;
    rt_boot_eth_device.dev_addr[1] = 22;
    rt_boot_eth_device.dev_addr[2] = 22;
    rt_boot_eth_device.dev_addr[3] = 22;
    rt_boot_eth_device.dev_addr[4] = 22;
    rt_boot_eth_device.dev_addr[5] = 22;
	
	rt_event_init(&rt_boot_eth_device.eth_recv_event, "eth recv event", RT_IPC_FLAG_FIFO);
	rt_event_init(&rt_boot_eth_device.eth_send_event, "eth send event", RT_IPC_FLAG_FIFO);
	
	board_eth_recv_register_event(&rt_boot_eth_device.eth_recv_event,ETH_EVENT_RECV_ID);
	board_eth_send_register_event(&rt_boot_eth_device.eth_send_event,ETH_EVENT_SEND_ID);

    /* register eth device */
    state = eth_device_init(&(rt_boot_eth_device.parent), "e0");
	
	eth_device_linkchange(&rt_boot_eth_device.parent, RT_TRUE);   //linkup the e0 for lwip to check
	
    rt_thread_init(&fake_eth_thread,
                   "fake_eth_thread",
                   &rt_boot_fake_eth_thread_entry,
                   RT_NULL,
                   &fake_eth_thread_stack[0],
                   sizeof(fake_eth_thread_stack),7,3);
    rt_thread_startup(&fake_eth_thread);
	
    return state;
}
