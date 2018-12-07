/*
 * Copyright (C) 2016 Piotr Dymacz <piotr@dymacz.pl>
 * Copyright (C) 2000-2006 Wolfgang Denk, DENX Software Engineering, <wd@denx.de>
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#include <kernel/rtthread.h>
#include <loader/image.h>
#include <loader/tplink_image.h>
#include <libs/crc32/crc32.h>
#include <libs/lzma/LzmaWrapper.h>

#include <board/loader.h>

#include <arch/cache.h>

static void tpl_to_uboot_header(image_header_t *hdr,
				tplink_image_header_t *tpl_hdr)
{
	rt_memset(hdr, 0, sizeof(image_header_t));

	/* Set only needed values */
	hdr->ih_hcrc = 0;
	hdr->ih_dcrc = 0;

	hdr->ih_ep   = imtohl(tpl_hdr->ih_kernel_ep);
	hdr->ih_size = imtohl(tpl_hdr->ih_kernel_len);
	hdr->ih_load = imtohl(tpl_hdr->ih_kernel_load);

	hdr->ih_os   = IH_OS_LINUX;
	hdr->ih_arch = IH_CPU_MIPS;
	hdr->ih_type = IH_TYPE_KERNEL;
	hdr->ih_comp = IH_COMP_LZMA;
}

static int ih_data_crc(rt_uint32_t addr, image_header_t *hdr)
{
	rt_kprintf("Data CRC...");
	if (crc32(0,(rt_uint8_t *)addr, imtohl(hdr->ih_size))
		!= imtohl(hdr->ih_dcrc))
	{
		rt_kprintf("ERROR\n");
		return 1;
	}
	else
	{
		rt_kprintf("OK!\n");
		return 0;
	}
}

static int ih_header_crc(image_header_t *hdr)
{
	rt_uint32_t crc,crc_temp;

	rt_kprintf("Header CRC...");	
	crc_temp = imtohl(hdr->ih_hcrc);
	hdr->ih_hcrc = 0;
	crc = crc32(0,(rt_uint8_t *)hdr, sizeof(image_header_t));
	hdr->ih_hcrc = crc_temp;
	if (crc_temp != crc) 
	{
		rt_kprintf("ERROR\n");
		return 1;
	} 
	else 
	{
		rt_kprintf("OK!\n");
		return 0;
	}	
}

int linux_loader(rt_uint32_t load_addr,rt_uint32_t unc_len)
{
	rt_uint32_t *len_ptr;
	rt_uint32_t addr, data, len;
	int i,tpl_type = 0;
	image_header_t header;
	image_header_t *hdr = &header;
	tplink_image_header_t *tpl_hdr;
	
	addr = load_addr;

	rt_kprintf("Booting image from 0x%08lX...\n", addr);

	/* Check what header type we have */
	rt_memmove(&data, (char *)addr, sizeof(rt_uint32_t));

	switch (imtohl(data)) {
	case TPL_IH_VERSION_V1:
	case TPL_IH_VERSION_V2:
		tpl_type = 1;
		tpl_hdr = (tplink_image_header_t *)addr;
		/* Convert to general format */
		tpl_to_uboot_header(hdr, tpl_hdr);
		rt_kprintf("found tplink image header\n");
		break;
	case IH_MAGIC:
		rt_memmove(hdr, (char *)addr, sizeof(image_header_t));
		rt_kprintf("found magic image header\n");
		break;
	case TPL_IH_VERSION_V3:
	default:
		rt_kprintf("unsupported image header : 0x%08x\n",data);
		return 1;
	}

	if(!tpl_type)
	{
		if (ih_header_crc(hdr) != 0) {
			rt_kprintf("header checksum mismatch!\n");
			return 1;
		}
		/* And data if enabled */
		data = addr + sizeof(image_header_t);
		if (ih_data_crc(data, hdr) != 0) {
			rt_kprintf("data checksum mismatch!\n");
			return 1;
		}
	}
	else
	{
		data = addr + sizeof(tplink_image_header_t);
	}

	len = imtohl(hdr->ih_size);
	len_ptr = (rt_uint32_t *)data;

	/* Image type... */
	switch (hdr->ih_type) {
	case IH_TYPE_KERNEL:
		break;
	case IH_TYPE_MULTI:
		/* OS kernel is always in first image */
		len = imtohl(len_ptr[0]);
		data += 8;
		/* Move over list to first image */
		for (i = 1; len_ptr[i]; ++i)
			data += 4;
		break;
	default:
		rt_kprintf("unsupported image type!\n");
		return 1;
	}

	/* Compression type... */
	switch (hdr->ih_comp) {
	case IH_COMP_LZMA:
        rt_kprintf("Uncompressing... ");
        i = lzma_inflate((rt_uint8_t *)data, len,
            (rt_uint8_t *)imtohl(hdr->ih_load), (int *)&unc_len);
		if (i != LZMA_RESULT_OK) {
			rt_kprintf("LZMA error '%d'!\n", i);
			return 1;
		}
		rt_kprintf("OK!\n");
		break;
	default:
        rt_kprintf("unsupported compression type!\n");
		return 1;
	}
	
	arch_cache_flush(hdr->ih_load,unc_len);
	
    board_linux_loader(hdr);

	return 0;
}

void autoboot_linux(void)
{
	board_autoboot_linux();
}
MSH_CMD_EXPORT(autoboot_linux, try boot firmware);
