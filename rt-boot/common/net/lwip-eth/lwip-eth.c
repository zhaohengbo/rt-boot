
#include <kernel/rtthread.h>
#include <drivers/rtdevice.h>
#include <net/lwip/netif/ethernetif.h>

#include <board/network.h>

#define ETH_EVENT_RECV_ID 0
#define ETH_EVENT_SEND_ID 0

#define MAX_ADDR_LEN 6
struct lwip_eth
{
	/* inherit from ethernet device */
	struct eth_device parent;
	rt_uint8_t dev_addr[MAX_ADDR_LEN];
};

static struct lwip_eth lwip_eth_device;

static rt_err_t lwip_eth_init(rt_device_t dev)
{
	return RT_EOK;
}

static rt_err_t lwip_eth_open(rt_device_t dev, rt_uint16_t oflag)
{
	return RT_EOK;
}

static rt_err_t lwip_eth_close(rt_device_t dev)
{
	return RT_EOK;
}

static rt_size_t lwip_eth_read(rt_device_t dev, rt_off_t pos, void* buffer, rt_size_t size)
{
	rt_set_errno(-RT_ENOSYS);
	return 0;
}

static rt_size_t lwip_eth_write (rt_device_t dev, rt_off_t pos, const void* buffer, rt_size_t size)
{
	rt_set_errno(-RT_ENOSYS);
	return 0;
}

static rt_err_t lwip_eth_control(rt_device_t dev, int cmd, void *args)
{
    switch (cmd)
    {
    case NIOCTL_GADDR:
        if (args) rt_memcpy(args, lwip_eth_device.dev_addr, 6);
        else return -RT_ERROR;
        break;

    default :
        break;
    }
    return RT_EOK;
}

rt_err_t lwip_eth_tx( rt_device_t dev, struct pbuf* p)
{
	struct pbuf* q;
    /* copy frame from pbufs to driver buffers */
    for(q = p; q != NULL; q = q->next)
    {
		board_eth_send(q->payload,q->len);
	}
    return ERR_OK;
}

static rt_uint8_t temp_buffer[1600];

struct pbuf *lwip_eth_rx(rt_device_t dev)
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

int lwip_eth_device_init(void)
{
    rt_err_t state;

    lwip_eth_device.parent.parent.init       = lwip_eth_init;
    lwip_eth_device.parent.parent.open       = lwip_eth_open;
    lwip_eth_device.parent.parent.close      = lwip_eth_close;
    lwip_eth_device.parent.parent.read       = lwip_eth_read;
    lwip_eth_device.parent.parent.write      = lwip_eth_write;
    lwip_eth_device.parent.parent.control    = lwip_eth_control;
    lwip_eth_device.parent.parent.user_data  = RT_NULL;

    lwip_eth_device.parent.eth_rx     = lwip_eth_rx;
    lwip_eth_device.parent.eth_tx     = lwip_eth_tx;
	
    lwip_eth_device.dev_addr[0] = 22;
    lwip_eth_device.dev_addr[1] = 22;
    lwip_eth_device.dev_addr[2] = 22;
    lwip_eth_device.dev_addr[3] = 22;
    lwip_eth_device.dev_addr[4] = 22;
    lwip_eth_device.dev_addr[5] = 22;

	board_network_start();
    board_eth_recv_register_cb((void *)eth_device_ready,&(lwip_eth_device.parent));
	
    /* register eth device */
    state = eth_device_init(&(lwip_eth_device.parent), "e0");
	
    eth_device_linkchange(&lwip_eth_device.parent, RT_TRUE);   //linkup the e0 for lwip to check
	
    return state;
}
