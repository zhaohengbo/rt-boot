/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 */

#include <kernel/rtthread.h>
#include <dfs/filesystems/dfs_romfs.h>

const static struct romfs_dirent _ram[] =
{
	
};

const static struct romfs_dirent _dev[] =
{
	
};

RT_WEAK const struct romfs_dirent _root_dirent[] =
{
    {ROMFS_DIRENT_DIR, "ram", (rt_uint8_t *)_ram, sizeof(_ram)/sizeof(_ram[0])},
	{ROMFS_DIRENT_DIR, "dev", (rt_uint8_t *)_dev, sizeof(_dev)/sizeof(_dev[0])},
};

RT_WEAK const struct romfs_dirent romfs_root =
{
    ROMFS_DIRENT_DIR, "/", (rt_uint8_t *)_root_dirent, sizeof(_root_dirent)/sizeof(_root_dirent[0])
};

