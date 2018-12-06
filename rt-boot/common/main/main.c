/*
 * The early init code for MIPS
 * Copyright ZhaoXiaowei 2018
 * Github:github.com/zhaohengbo
 */

#include <kernel/rtthread.h>
#include <finsh/shell.h>
#include <global/global.h>
#include <board/spi.h>
#include <board/flash.h>
#include <board/console.h>
#include <loader/image.h>
#include <board/loader.h>

#include <net/lwip-dhcpd/dhcp_server.h>
#include <net/lwip-telnetd/telnetd.h>
#include <net/lwip-httpd/httpd.h>
#include <net/lwip-breakd/breakd.h>

#include <dfs/dfs.h>
#include <dfs/dfs_fs.h>
#include <dfs/filesystems/dfs_romfs.h>
#include <dfs/filesystems/dfs_ramfs.h>

static struct rt_event boot_break_event;

#define BOOT_BREAK_BY_UART 0
#define BOOT_BREAK_BY_NET 1

void board_break_net_notisfy(void)
{
	rt_event_send(&boot_break_event, (1 << BOOT_BREAK_BY_NET));
}

ALIGN(RT_ALIGN_SIZE)
static char main_thread_stack[0x400];
struct rt_thread main_thread;
void rt_thread_main_thread_entry(void* parameter)
{
	struct dfs_ramfs* ramfs;
	rt_uint32_t boot_break_event_flag = 0;
	
	rt_uint32_t break_flag = 0;
	
	rt_uint32_t delay_count=5;
	
	soc_spi_init();
	soc_flash_init();
	dfs_init();
	dfs_ramfs_init();
	dfs_romfs_init();
	if (dfs_mount(RT_NULL, "/", "rom", 0, &romfs_root) == 0)
		rt_kprintf("ROM File System initialized!\n");
	else
		rt_kprintf("ROM File System initialzation failed!\n");
	ramfs =(struct dfs_ramfs*) dfs_ramfs_create((rt_uint8_t *)rtboot_data.ramfs_start_base, rtboot_data.ramfs_end_base - rtboot_data.ramfs_start_base);
	if(ramfs != RT_NULL)
	{
		if (dfs_mount(RT_NULL, "/ram", "ram", 0, ramfs) == 0)
			rt_kprintf("RAM File System initialized!\n");
		else
			rt_kprintf("RAM File System initialzation failed!\n");
	}
	extern int rt_hw_boot_eth_init(void);
	extern int lwip_system_init(void);
	lwip_system_init();
	rt_hw_boot_eth_init();
	dhcpd_start("e0");
	break_server();
	
	while(delay_count--)
	{
		rt_kprintf("Auto Boot Wait Delay %d...\n",delay_count);
		rt_event_recv(&boot_break_event, 
					  	(1 << BOOT_BREAK_BY_NET)
				  	,RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR
				 	,RT_TICK_PER_SECOND, &boot_break_event_flag);
		if(boot_break_event_flag & (1 << BOOT_BREAK_BY_NET))
		{
			rt_kprintf("Break By Net\n");
			break_flag = 1;
			break;
		}
		
		if(rt_hw_console_tstc())
		{
			rt_kprintf("Break By Console\n");
			break_flag = 1;
			break;
		}
		
	}
	
	if(!break_flag)
	{
		board_boot_linux();
		rt_kprintf("Break By Boot-Failed\n");
	}
	
	http_server();
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
