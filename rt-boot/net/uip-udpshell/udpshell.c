#include <kernel/rtthread.h>
#include <drivers/rtdevice.h>
#include <finsh/finsh.h>
#include <finsh/msh.h>
#include <finsh/shell.h>

#include <net/uip/uip.h>
#include <net/uip-main/uip_main.h>
#include <net/uip-udpshell/udpshell.h>

struct udpshell_session
{
    struct rt_ringbuffer rx_ringbuffer;
    struct rt_ringbuffer tx_ringbuffer;

    rt_mutex_t rx_ringbuffer_lock;
    rt_mutex_t tx_ringbuffer_lock;
	rt_mutex_t deamon_status_lock;
	
    struct rt_device device;
	
	rt_uint8_t status;

	rt_sem_t deamon_notice;
};

struct udpshell_session shell_session;

#define RX_BUFFER_SIZE      256
#define TX_BUFFER_SIZE      4096

static rt_uint8_t udpshell_rx_buffer[RX_BUFFER_SIZE];
static rt_uint8_t udpshell_tx_buffer[TX_BUFFER_SIZE];

static uip_ipaddr_t remote_ipaddr;

void udpshell_appcall(void) {
    if (uip_udp_conn->lport == HTONS(6666)) {
		if (uip_newdata() && (uip_udp_conn->rport == HTONS(6666))) 
		{
			uip_ipaddr_copy(remote_ipaddr, uip_udp_conn->ripaddr);
			
			rt_mutex_take(shell_session.rx_ringbuffer_lock, RT_WAITING_FOREVER);
            rt_ringbuffer_put(&(shell_session.rx_ringbuffer), (rt_uint8_t *)uip_appdata, uip_datalen());
    		rt_mutex_release(shell_session.rx_ringbuffer_lock);
			
			rt_mutex_take(shell_session.deamon_status_lock, RT_WAITING_FOREVER);
			if(!shell_session.status)
			{
				shell_session.status = 1;
    			rt_mutex_release(shell_session.deamon_status_lock);
				rt_sem_release(shell_session.deamon_notice);
			}
			else
			{
				rt_mutex_release(shell_session.deamon_status_lock);
			}
			
			if (shell_session.device.rx_indicate != RT_NULL)
    		{
        		shell_session.device.rx_indicate(&shell_session.device, 1);
    		}
		}
		else if(uip_poll())
        {
			rt_uint8_t *buffer;
			rt_size_t result;
			buffer = rt_calloc(1000,1);
			rt_mutex_take(shell_session.tx_ringbuffer_lock, RT_WAITING_FOREVER);
    		result = rt_ringbuffer_get(&(shell_session.tx_ringbuffer), buffer, 1000);
    		rt_mutex_release(shell_session.tx_ringbuffer_lock);
			if(result != 0)
				uip_udp_send(remote_ipaddr,6666,buffer,result);
			if(rt_ringbuffer_data_len(&(shell_session.tx_ringbuffer)))
				uip_udp_active_poll();
			rt_free(buffer);
		}
	}
}

/* RT-Thread Device Driver Interface */
static rt_err_t udpshell_device_init(rt_device_t dev)
{
    return RT_EOK;
}

static rt_err_t udpshell_device_open(rt_device_t dev, rt_uint16_t oflag)
{
	rt_mutex_take(shell_session.rx_ringbuffer_lock, RT_WAITING_FOREVER);
    rt_ringbuffer_reset(&(shell_session.rx_ringbuffer));
    rt_mutex_release(shell_session.rx_ringbuffer_lock);
	
	rt_mutex_take(shell_session.tx_ringbuffer_lock, RT_WAITING_FOREVER);
    rt_ringbuffer_reset(&(shell_session.tx_ringbuffer));
    rt_mutex_release(shell_session.tx_ringbuffer_lock);
	
    return RT_EOK;
}

static rt_err_t udpshell_device_close(rt_device_t dev)
{
	rt_mutex_take(shell_session.rx_ringbuffer_lock, RT_WAITING_FOREVER);
    rt_ringbuffer_reset(&(shell_session.rx_ringbuffer));
    rt_mutex_release(shell_session.rx_ringbuffer_lock);
	
	rt_mutex_take(shell_session.tx_ringbuffer_lock, RT_WAITING_FOREVER);
    rt_ringbuffer_reset(&(shell_session.tx_ringbuffer));
    rt_mutex_release(shell_session.tx_ringbuffer_lock);
	
    return RT_EOK;
}

static rt_size_t udpshell_device_read(rt_device_t dev, rt_off_t pos, void* buffer, rt_size_t size)
{
    rt_size_t result;
	
    /* read from rx ring buffer */
    rt_mutex_take(shell_session.rx_ringbuffer_lock, RT_WAITING_FOREVER);
    result = rt_ringbuffer_get(&(shell_session.rx_ringbuffer), buffer, size);
    rt_mutex_release(shell_session.rx_ringbuffer_lock);

    return result;
}

static rt_size_t udpshell_device_write (rt_device_t dev, rt_off_t pos, const void* buffer, rt_size_t size)
{
    const rt_uint8_t *ptr;

    ptr = (rt_uint8_t*) buffer;

    rt_mutex_take(shell_session.tx_ringbuffer_lock, RT_WAITING_FOREVER);
    while (size)
    {
        if (*ptr == '\n')
            rt_ringbuffer_putchar(&shell_session.tx_ringbuffer, '\r');

        if (rt_ringbuffer_putchar(&shell_session.tx_ringbuffer, *ptr) == 0) /* overflow */
            break;
        ptr++;
        size--;
    }
    rt_mutex_release(shell_session.tx_ringbuffer_lock);

    uip_udp_active_poll();

    return (rt_uint32_t) ptr - (rt_uint32_t) buffer;
}

static rt_err_t udpshell_device_control(rt_device_t dev, int cmd, void *args)
{
    return RT_EOK;
}

ALIGN(RT_ALIGN_SIZE)
static char udpshell_deamon_thread_stack[0x200];
struct rt_thread udpshell_deamon_thread;

static void rt_thread_udpshell_deamon_thread_entry(void *parameter)
{
    shell_session.device.type = RT_Device_Class_Char;
    shell_session.device.init = udpshell_device_init;
    shell_session.device.open = udpshell_device_open;
    shell_session.device.close = udpshell_device_close;
    shell_session.device.read = udpshell_device_read;
    shell_session.device.write = udpshell_device_write;
    shell_session.device.control = udpshell_device_control;

	rt_device_register(&shell_session.device, "udpshell", RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_STREAM);
	
	while(1)
	{
		rt_uint8_t echo_mode;
		
		while(1)
		{
			rt_mutex_take(shell_session.deamon_status_lock, RT_WAITING_FOREVER);
			if(!shell_session.status)
			{
				rt_mutex_release(shell_session.deamon_status_lock);
				rt_sem_take(shell_session.deamon_notice, RT_WAITING_FOREVER);
			}
			else
			{
				rt_mutex_release(shell_session.deamon_status_lock);
				break;
			}
		}
		
		echo_mode = finsh_get_echo();
		rt_console_set_device("udpshell");
		finsh_set_device("udpshell");
		finsh_set_echo(0);
		
		rt_kprintf("\nNow Starting UDP Shell...\n");
		rt_kprintf(FINSH_PROMPT);
		
		while(1)
		{
			rt_mutex_take(shell_session.deamon_status_lock, RT_WAITING_FOREVER);
			if(shell_session.status)
			{
				rt_mutex_release(shell_session.deamon_status_lock);
				rt_sem_take(shell_session.deamon_notice, RT_WAITING_FOREVER);
			}
			else
			{
				rt_mutex_release(shell_session.deamon_status_lock);
				break;
			}
		}
		
		rt_console_set_device(RT_CONSOLE_DEVICE_NAME);
		finsh_set_device(RT_CONSOLE_DEVICE_NAME);
		
		rt_kprintf("\nNow Recover TTL Shell...\n");
		rt_kprintf(FINSH_PROMPT);
		finsh_set_echo(echo_mode);
	}
}

static void udpshell(uint8_t argc, char **argv) 
{
	if (argc < 2) 
	{
		goto udpshell_too_few_arg;
	}
	else 
	{
		const char *operator = argv[1];
		if(!rt_strcmp(operator, "start"))
		{
			rt_mutex_take(shell_session.deamon_status_lock, RT_WAITING_FOREVER);
			if(!shell_session.status)
			{
				shell_session.status = 1;
    			rt_mutex_release(shell_session.deamon_status_lock);
				rt_kprintf("Starting UDP Shell...\n");
				rt_sem_release(shell_session.deamon_notice);
			}
			else
			{
				rt_mutex_release(shell_session.deamon_status_lock);
				rt_kprintf("Already in UDP Shell!\n");
			}
		}
		else if(!rt_strcmp(operator, "exit"))
		{
			rt_mutex_take(shell_session.deamon_status_lock, RT_WAITING_FOREVER);
			if(shell_session.status)
			{
				shell_session.status = 0;
    			rt_mutex_release(shell_session.deamon_status_lock);
				rt_kprintf("Exit UDP Shell\n");
				rt_sem_release(shell_session.deamon_notice);
			}
			else
			{
				rt_mutex_release(shell_session.deamon_status_lock);
				rt_kprintf("UDP Shell isn't started!\n");
			}
		}
	}
	return;
udpshell_too_few_arg:
	rt_kprintf("Too few arg!\n");
	goto udpshell_reprint;
udpshell_reprint:
	rt_kprintf("Try udpshell start\n");
	rt_kprintf("Try udpshell exit\n");
}

MSH_CMD_EXPORT(udpshell, UDP Shell operation.);

static void udpshell_deamon_init(void)
{
    rt_thread_init(&udpshell_deamon_thread,
                   "udpshell_deamon_thread",
                   &rt_thread_udpshell_deamon_thread_entry,
                   RT_NULL,
                   &udpshell_deamon_thread_stack[0],
                   sizeof(udpshell_deamon_thread_stack),8,4);
    rt_thread_startup(&udpshell_deamon_thread);
}

void udpshell_init(void) {
	struct uip_udp_conn *udpshell_conn;
	
	rt_ringbuffer_init(&shell_session.rx_ringbuffer, udpshell_rx_buffer, RX_BUFFER_SIZE);
	rt_ringbuffer_init(&shell_session.tx_ringbuffer, udpshell_tx_buffer, TX_BUFFER_SIZE);
	
	/* create tx ringbuffer lock */
    shell_session.tx_ringbuffer_lock = rt_mutex_create("udpshell_tx", RT_IPC_FLAG_FIFO);
    /* create rx ringbuffer lock */
    shell_session.rx_ringbuffer_lock = rt_mutex_create("udpshell_rx", RT_IPC_FLAG_FIFO);
	
	shell_session.deamon_status_lock = rt_mutex_create("udpshell_deamon", RT_IPC_FLAG_FIFO);
	
	shell_session.deamon_notice = rt_sem_create("udpshell_deamon", 0, RT_IPC_FLAG_FIFO);
	
	uip_ipaddr(remote_ipaddr, 255,255,255,255);
	
    udpshell_conn = uip_udp_new(HTONS(6666));
	if(udpshell_conn != RT_NULL)
	{
		register_udp_appcall(udpshell_appcall);
        udpshell_deamon_init();
	}
}
