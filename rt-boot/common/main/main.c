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

#include <net/lwip-dhcpd/dhcp_server.h>
#include <net/lwip-telnetd/telnetd.h>
#include <net/lwip-httpd/httpd.h>
#include <net/lwip-breakd/breakd.h>

#include <dfs/dfs.h>
#include <dfs/dfs_fs.h>
#include <dfs/filesystems/dfs_romfs.h>
#include <dfs/filesystems/dfs_ramfs.h>

ALIGN(RT_ALIGN_SIZE)
static char main_thread_stack[0x400];
struct rt_thread main_thread;
void rt_thread_main_thread_entry(void* parameter)
{
	struct dfs_ramfs* ramfs;
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
	http_server();
	break_server();
#ifdef RT_USING_FINSH
    /* init finsh */
    finsh_system_init();
    finsh_set_device(RT_CONSOLE_DEVICE_NAME);
	telnet_server();
#endif
}

int rt_application_init(void)
{
	rt_thread_init(&main_thread,
                   "main_thread",
                   &rt_thread_main_thread_entry,
                   RT_NULL,
                   &main_thread_stack[0],
                   sizeof(main_thread_stack),12,5);
	
    rt_thread_startup(&main_thread);
    return 0;
}
