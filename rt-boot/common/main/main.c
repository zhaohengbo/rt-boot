/*
 * The early init code for MIPS
 * Copyright ZhaoXiaowei 2018
 * Github:github.com/zhaohengbo
 */

#include <kernel/rtthread.h>
#include <kernel/rthw.h>
#include <finsh/shell.h>
#include <global/global.h>
#include <board/spi.h>
#include <board/flash.h>
#include <board/gpio.h>
#include <loader/image.h>
#include <loader/loader.h>

#include <net/lwip-dhcpd/dhcp_server.h>
#include <net/lwip-telnetd/telnetd.h>
#include <net/lwip-breakd/breakd.h>
#include <net/lwip-eth/lwip-eth.h>
#include <net/webnet/wn_main.h>

#include <dfs/dfs.h>
#include <dfs/dfs_fs.h>
#include <dfs/filesystems/dfs_romfs.h>
#include <dfs/filesystems/dfs_ramfs.h>

#include <env/env.h>

static struct rt_event boot_break_event;

#define BOOT_BREAK_BY_UART 0
#define BOOT_BREAK_BY_NET 1
#define BOOT_BREAK_BY_KEY 2

void boot_break_net_notisfy(void)
{
	rt_event_send(&boot_break_event, (1 << BOOT_BREAK_BY_NET));
}

void boot_break_uart_notisfy(void)
{
	rt_event_send(&boot_break_event, (1 << BOOT_BREAK_BY_UART));
}

void boot_break_key_notisfy(void)
{
	rt_event_send(&boot_break_event, (1 << BOOT_BREAK_BY_KEY));
}

static void sysfs_init(void)
{
	struct dfs_ramfs* ramfs;
	
	rt_uint32_t ramfs_start,ramfs_size;
	rt_uint32_t ramfs_end = rtboot_data.end_stack_base;
	
	ramfs_size = (ramfs_end - rtboot_data.system_memstart) / 2;
	ramfs_start = ramfs_end - ramfs_size;
	
	dfs_init();
	dfs_ramfs_init();
	dfs_romfs_init();
	if (dfs_mount(RT_NULL, "/", "rom", 0, &romfs_root) == 0)
		rt_kprintf("ROM File System initialized!\n");
	else
		rt_kprintf("ROM File System initialzation failed!\n");
	ramfs =(struct dfs_ramfs*) dfs_ramfs_create((rt_uint8_t *)ramfs_start, ramfs_size);
	if(ramfs != RT_NULL)
	{
		if (dfs_mount(RT_NULL, "/tmp", "ram", 0, ramfs) == 0)
			rt_kprintf("RAM File System initialized!\n");
		else
			rt_kprintf("RAM File System initialzation failed!\n");
	}
	board_flash_fs_init();
}

static void boot_break(void)
{
	rt_device_t dev = RT_NULL;
	rt_uint32_t boot_break_event_flag = 0;
	rt_uint32_t break_flag = 0;
	rt_uint32_t delay_count=5;
	
	dev = rt_device_find(RT_CONSOLE_DEVICE_NAME);
	if (dev != RT_NULL)
    {
        if (rt_device_open(dev, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX | \
                       RT_DEVICE_FLAG_STREAM) == RT_EOK)
		{
			rt_device_set_rx_indicate(dev, (rt_err_t (*)(struct rt_device *, rt_size_t))boot_break_uart_notisfy);
		}
    }
	
	while(delay_count--)
	{
		rt_kprintf("Auto Boot Wait Delay %d...\n",delay_count);
		rt_event_recv(&boot_break_event, 
					  (1 << BOOT_BREAK_BY_UART) |
					  (1 << BOOT_BREAK_BY_NET) |
					  (1 << BOOT_BREAK_BY_KEY)
				  	,RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR
				 	,RT_TICK_PER_SECOND/2, &boot_break_event_flag);
		if(boot_break_event_flag & (1 << BOOT_BREAK_BY_NET))
		{
			rt_kprintf("Break By Net!\n");
			break_flag = 1;
			break;
		}
		
		if(boot_break_event_flag & (1 << BOOT_BREAK_BY_UART))
		{
			rt_kprintf("Break By Console!\n");
			break_flag = 1;
			break;
		}
		
		if(boot_break_event_flag & (1 << BOOT_BREAK_BY_KEY))
		{
			rt_kprintf("Break By Key!\n");
			break_flag = 1;
			break;
		}
		
	}
	
	if(!break_flag)
	{
		rt_kprintf("Try Boot Firmware...\n");
		autoboot_linux();
		rt_kprintf("Break By Boot-Failed!\n");
	}
	
	if(dev != RT_NULL)
	{
		rt_device_close(dev);
		rt_device_set_rx_indicate(dev, RT_NULL);
	}
}

ALIGN(RT_ALIGN_SIZE)
static char main_thread_stack[0x400];
struct rt_thread main_thread;
void rt_thread_main_thread_entry(void* parameter)
{
	gpio_thread_init();
	board_spi_init();
	board_flash_init();
	env_init();
	
	rt_kprintf("\nSystem Info:\n");
	
	rt_kprintf("Board: %s\n",rtboot_data.board_name);
	rt_kprintf("Soc: %s\n",rtboot_data.soc_name);
	rt_kprintf("Ethernet: %s\n",rtboot_data.eth_name);
	rt_kprintf("Dram: %s\n",rtboot_data.ram_size);
	rt_kprintf("Flash: %s\n",rtboot_data.flash_info);
	rt_kprintf("Clocks: %s\n\n",rtboot_data.clock_info);
	
	lwip_system_init();
	lwip_eth_device_init();
	dhcpd_start("e0");
	break_server();

	boot_break();
	
	rt_kprintf("Starting Recovery File System...\n");
	sysfs_init();
	rt_kprintf("Starting Http Recover Server...\n");
	httpd_init();
	rt_kprintf("Starting Build-in Shell...\n\n");
#ifdef RT_USING_FINSH
    /* init finsh */
    finsh_system_init();
    finsh_set_device(RT_CONSOLE_DEVICE_NAME);
	telnet_server();
#endif
}

int rt_application_init(void)
{
	rt_event_init(&boot_break_event, "boot break event", RT_IPC_FLAG_FIFO);
	rt_thread_init(&main_thread,
                   "main_thread",
                   &rt_thread_main_thread_entry,
                   RT_NULL,
                   &main_thread_stack[0],
                   sizeof(main_thread_stack),12,5);
	
    rt_thread_startup(&main_thread);
    return 0;
}
