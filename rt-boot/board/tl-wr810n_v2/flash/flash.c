/*
 * Qualcomm/Atheros High-Speed UART driver
 *
 * Copyright (C) 2015 Piotr Dymacz <piotr@dymacz.pl>
 * Copyright (C) 2014 Mantas Pucka <mantas@8devices.com>
 * Copyright (C) 2008-2010 Atheros Communications Inc.
 *
 * Values for UART_SCALE and UART_STEP:
 * https://www.mail-archive.com/openwrt-devel@lists.openwrt.org/msg22371.html
 *
 * Partially based on:
 * Linux/drivers/tty/serial/qca953x_uart.c
 *
 * SPDX-License-Identifier:GPL-2.0
 */

#include <kernel/rtthread.h>
#include <kernel/rthw.h>
#include <drivers/rtdevice.h>
#include <drivers/drivers/spi_flash.h>
#include <drivers/drivers/spi_flash_sfud.h>

#include <dfs/dfs.h>
#include <dfs/dfs_fs.h>
#include <dfs/dfs_posix.h>
#include <dfs/filesystems/dfs_romfs.h>
#include <dfs/filesystems/dfs_romfs.h>

#include <global/global.h>
#include <soc/qca953x/qca953x_map.h>

#include <arch/cache.h>

#define SPI_BUS_NAME                "SPI0"
#define SPI_FLASH_DEVICE_NAME       "SPI00"
#define SPI_FLASH_CHIP              "SPI-FLASH0"

static struct rt_spi_device spi_flash_device;
static rt_spi_flash_device_t flash_chip_device;
static const sfud_flash *sfud_dev;

static char flash_info[64];

static struct romfs_dirent _romfs_flash[] = {
	{ROMFS_DIRENT_FILE, "firmware.bin", (rt_uint8_t *)0, 0},
	{ROMFS_DIRENT_FILE, "uboot.bin", (rt_uint8_t *)0, 0},
	{ROMFS_DIRENT_FILE, "art.bin", (rt_uint8_t *)0, 0},
	{ROMFS_DIRENT_FILE, "full.bin", (rt_uint8_t *)0, 0},
};

const struct romfs_dirent romfs_flash_root = {
    ROMFS_DIRENT_DIR, "/", (rt_uint8_t *)_romfs_flash, sizeof(_romfs_flash)/sizeof(_romfs_flash[0])
};

ALIGN(RT_ALIGN_SIZE)
static char board_flash_thread_stack[0x1000];
struct rt_thread board_flash_thread;
static struct rt_event board_flash_event;
static rt_int32_t board_flash_status;

#define BOARD_FLASH_EVENT_FIRMWARE 0
#define BOARD_FLASH_EVENT_UBOOT 1
#define BOARD_FLASH_EVENT_ART 2
#define BOARD_FLASH_EVENT_FULL 3

rt_uint32_t board_get_env_flag(void)
{
	return 1;
}

rt_uint32_t board_get_env_length(void)
{
	return 0x2000;
}

void board_get_env_read(rt_uint8_t * buffer)
{
	if (sfud_dev == RT_NULL)
	{
		arch_dcache_invalidate(0x9F01D000,0x2000);
		rt_memcpy(buffer,(void *)0x9F01D000,0x2000);
	}
	else
	{
		sfud_read(sfud_dev, 0x1D000, 0x2000, buffer);
	}
}

void board_get_env_write(rt_uint8_t * buffer)
{
	if (sfud_dev == RT_NULL)
		return;
	sfud_erase(sfud_dev, 0x1D000, 0x2000);
	sfud_write(sfud_dev, 0x1D000, 0x2000, buffer);
}

rt_int32_t board_flash_get_status(void)
{
	return board_flash_status;
}

void board_flash_firmware_notisfy(void)
{
	if(board_flash_status > 0)
		board_flash_status = 0;
	rt_event_send(&board_flash_event, (1 << BOARD_FLASH_EVENT_FIRMWARE));
}

void board_flash_uboot_notisfy(void)
{
	if(board_flash_status > 0)
		board_flash_status = 0;
	rt_event_send(&board_flash_event, (1 << BOARD_FLASH_EVENT_UBOOT));
}

void board_flash_art_notisfy(void)
{
	if(board_flash_status > 0)
		board_flash_status = 0;
	rt_event_send(&board_flash_event, (1 << BOARD_FLASH_EVENT_ART));
}

void board_flash_full_notisfy(void)
{
	if(board_flash_status > 0)
		board_flash_status = 0;
	rt_event_send(&board_flash_event, (1 << BOARD_FLASH_EVENT_FULL));
}

void board_flash_error_notisfy(void)
{
	board_flash_status = 404;
}

static void board_flash_thread_entry(void* parameter)
{
	rt_uint32_t board_flash_event_flag;
	int flash_size;
	
	int firmware_start = 0;
	int firmware_length = 0;
    int uboot_start = 0;
    int uboot_length = 0;
    int art_start = 0;
    int art_length = 0;
    int full_start = 0;
    int full_length = 0;
	
	if (sfud_dev == RT_NULL)
	{
		board_flash_status = -1;
		return;
	}
	
	flash_size = sfud_dev->chip.capacity / 1024 / 1024;
	
	switch(flash_size)
	{
		case 16:
			firmware_start = 0x20000;
			firmware_length = 0xFC0000;
            uboot_start = 0x00000;
            uboot_length = 0x20000;
            art_start = 0xFE0000;
            art_length = 0x20000;
            full_start = 0x0;
            full_length = 0x1000000;
			break;
		case 8:
			firmware_start = 0x20000;
			firmware_length = 0x7C0000;
            uboot_start = 0x00000;
            uboot_length = 0x20000;
            art_start = 0x7E0000;
            art_length = 0x20000;
            full_start = 0x0;
            full_length = 0x800000;
			break;
		case 4:
			firmware_start = 0x20000;
			firmware_length = 0x3C0000;
            uboot_start = 0x00000;
            uboot_length = 0x20000;
            art_start = 0x3E0000;
            art_length = 0x20000;
            full_start = 0x0;
            full_length = 0x400000;
			break;
		case 2:
			firmware_start = 0x20000;
			firmware_length = 0x1C0000;
            uboot_start = 0x00000;
            uboot_length = 0x20000;
            art_start = 0x1E0000;
            art_length = 0x20000;
            full_start = 0x0;
            full_length = 0x200000;
			break;
		default:
			board_flash_status = -1;
			return;
	}
	while(1)
	{
		int flash_start = 0;
		int flash_length = 0;
		const char *open_file = "";
		rt_event_recv(&board_flash_event, 
					  (1 << BOARD_FLASH_EVENT_FIRMWARE) |
					  (1 << BOARD_FLASH_EVENT_UBOOT) |
					  (1 << BOARD_FLASH_EVENT_ART) | 
					  (1 << BOARD_FLASH_EVENT_FULL) 
					  ,RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR,
					  RT_WAITING_FOREVER, &board_flash_event_flag);
		if(board_flash_event_flag & (1 << BOARD_FLASH_EVENT_FIRMWARE))
		{
			flash_start = firmware_start;
			flash_length = firmware_length;
			open_file = "/ram/firmware.bin";
		}
		else if(board_flash_event_flag & (1 << BOARD_FLASH_EVENT_UBOOT))
		{
			flash_start = uboot_start;
			flash_length = uboot_length;
			open_file = "/ram/uboot.bin";
		}
		else if(board_flash_event_flag & (1 << BOARD_FLASH_EVENT_ART))
		{
			flash_start = art_start;
			flash_length = art_length;
			open_file = "/ram/art.bin";
		}
		else if(board_flash_event_flag & (1 << BOARD_FLASH_EVENT_FULL))
		{
			flash_start = full_start;
			flash_length = full_length;
			open_file = "/ram/full.bin";
		}
		else
		{
			continue;
		}
		
		{
			rt_uint8_t *buffer;
			int fd;
			int read_length;
			int write_off;
			board_flash_status = 0;
			buffer = rt_malloc(0x1000);
			if(buffer)
			{
				fd = open(open_file, 0, O_RDONLY);
				if(fd < 0)
				{
					board_flash_status = -1;
				}
				else
				{
					write_off = flash_start;
					
					while (write_off < (flash_start + flash_length))
					{
						sfud_erase(sfud_dev, write_off, 0x1000);
						
						board_flash_status = ((write_off - flash_start)*100 / flash_length) / 2;
						
						write_off+=0x1000;
					}
					
					write_off = flash_start;
					
					while (1)
    				{
        				read_length = read(fd, buffer, 0x1000);
						
						if(read_length <= 0) break;
						
						sfud_write(sfud_dev,write_off,read_length,buffer);
						write_off+=read_length;
						
						board_flash_status = ((write_off - flash_start)*100 / flash_length) / 2 + 50;
    				}
					board_flash_status = 100;
					close(fd);
				}
				
				rt_free(buffer);
			}
		}
		
	}
}

static void board_flash_thread_init(void)
{
	rt_event_init(&board_flash_event, "board flash event", RT_IPC_FLAG_FIFO);
    rt_thread_init(&board_flash_thread,
                   "board_flash_thread",
                   &board_flash_thread_entry,
                   RT_NULL,
                   &board_flash_thread_stack[0],
                   sizeof(board_flash_thread_stack),9,6);

    rt_thread_startup(&board_flash_thread);
}

void board_flash_fs_init(void)
{
	int flash_size;
	flash_size = sfud_dev->chip.capacity / 1024 / 1024;
	if (sfud_dev == RT_NULL)
	{
		return;
	}
	switch(flash_size)
	{
		case 16:
			_romfs_flash[0].data = (rt_uint8_t *)0x9F020000;
			_romfs_flash[0].size = 0xFC0000;
			_romfs_flash[1].data = (rt_uint8_t *)0x9F000000;
			_romfs_flash[1].size = 0x20000;
			_romfs_flash[2].data = (rt_uint8_t *)0x9FFE0000;
			_romfs_flash[2].size = 0x20000;
			_romfs_flash[3].data = (rt_uint8_t *)0x9F000000;
			_romfs_flash[3].size = 0x1000000;
			break;
		case 8:
			_romfs_flash[0].data = (rt_uint8_t *)0x9F020000;
			_romfs_flash[0].size = 0x7C0000;
			_romfs_flash[1].data = (rt_uint8_t *)0x9F000000;
			_romfs_flash[1].size = 0x20000;
			_romfs_flash[2].data = (rt_uint8_t *)0x9F7E0000;
			_romfs_flash[2].size = 0x20000;
			_romfs_flash[3].data = (rt_uint8_t *)0x9F000000;
			_romfs_flash[3].size = 0x800000;
			break;
		case 4:
			_romfs_flash[0].data = (rt_uint8_t *)0x9F020000;
			_romfs_flash[0].size = 0x3C0000;
			_romfs_flash[1].data = (rt_uint8_t *)0x9F000000;
			_romfs_flash[1].size = 0x20000;
			_romfs_flash[2].data = (rt_uint8_t *)0x9F3E0000;
			_romfs_flash[2].size = 0x20000;
			_romfs_flash[3].data = (rt_uint8_t *)0x9F000000;
			_romfs_flash[3].size = 0x400000;
			break;
		case 2:
			_romfs_flash[0].data = (rt_uint8_t *)0x9F020000;
			_romfs_flash[0].size = 0x1C0000;
			_romfs_flash[1].data = (rt_uint8_t *)0x9F000000;
			_romfs_flash[1].size = 0x20000;
			_romfs_flash[2].data = (rt_uint8_t *)0x9F1E0000;
			_romfs_flash[2].size = 0x20000;
			_romfs_flash[3].data = (rt_uint8_t *)0x9F000000;
			_romfs_flash[3].size = 0x200000;
			break;
		default:
			return;
	}
	dfs_mount(RT_NULL, "/rom/map", "rom", 0, &romfs_flash_root);
}

static rt_err_t rt_hw_spi_flash_attach(void)
{
    /* attach cs */
	rt_err_t result;

    result = rt_spi_bus_attach_device(&spi_flash_device, SPI_FLASH_DEVICE_NAME, SPI_BUS_NAME, (void*)0);
	if (result != RT_EOK)
	{
		return result;
	}

	return RT_EOK;
}

rt_err_t board_flash_init(void)
{
	rt_err_t result;
	result = rt_hw_spi_flash_attach();
	if (result != RT_EOK)
	{
		return result;
	}
    flash_chip_device = rt_sfud_flash_probe(SPI_FLASH_CHIP, SPI_FLASH_DEVICE_NAME);
    if (flash_chip_device == RT_NULL)
		return RT_ERROR;
	sfud_dev = (sfud_flash_t)flash_chip_device->user_data;

	rt_sprintf(flash_info,"%dMB[SPI-FLASH]",sfud_dev->chip.capacity / 1024 / 1024);
	rtboot_data.flash_info = flash_info;
	
	board_flash_thread_init();
	return RT_EOK;
}
