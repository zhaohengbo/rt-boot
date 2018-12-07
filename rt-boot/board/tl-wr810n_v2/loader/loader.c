/*
 * Copyright (C) 2016 Piotr Dymacz <piotr@dymacz.pl>
 * Copyright (C) 2000-2006 Wolfgang Denk, DENX Software Engineering, <wd@denx.de>
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#include <kernel/rtthread.h>
#include <init/init.h>
#include <loader/image.h>

#include <arch/loader.h>
#include <arch/cache.h>
#include <loader/loader.h>

#define CFG_BOOTM_LEN		(16 << 20)
#define CFG_LOAD_ADDR	0x9F020000

int board_linux_loader(image_header_t *hdr)
{
	arch_linux_loader(hdr,"console=ttyS0,115200 root=31:02 "\
              "rootfstype=squashfs init=/sbin/init "\
              "mtdparts=ath-nor0:128k(u-boot),1024k(kernel),2816k(rootfs),64k(config),64k(art)");
	return 0;
}

void board_autoboot_linux(void)
{
	arch_dcache_invalidate(0x9F000000,0x1000000);
	
	if(linux_loader(CFG_LOAD_ADDR,CFG_BOOTM_LEN))
		return;
	
	rt_kprintf("Shutdown RTOS kernel...\n");
	
	system_deinit();
	
	rt_kprintf("Starting Linux Kernel...\n");
	arch_linux_jump(0);
}