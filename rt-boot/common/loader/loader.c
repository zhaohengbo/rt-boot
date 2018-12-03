/*
 * Copyright (C) 2016 Piotr Dymacz <piotr@dymacz.pl>
 * Copyright (C) 2000-2006 Wolfgang Denk, DENX Software Engineering, <wd@denx.de>
 *
 * SPDX-License-Identifier: GPL-2.0
 */

/*
 * Boot support
 */

extern void do_bootm_linux(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

/* U-Boot type image header */
image_header_t header;

/* Default load address */
u32 load_addr = CFG_LOAD_ADDR;

#define TPL_ALIGN_SIZE		21
#define UBOOT_ALIGN_SIZE	14

/*
 * Verifies U-Boot image data checksum
 */
static int ih_data_crc(u32 addr, image_header_t *hdr, int tpl_type, int verify)
{
	int ret = 0;

	printf("   %-*s ", UBOOT_ALIGN_SIZE, "Data CRC...");

	if (tpl_type == 0 && verify == 1) {
		if (tinf_crc32((u8 *)addr, ntohl(hdr->ih_size))
		    != ntohl(hdr->ih_dcrc)) {
			puts("ERROR\n\n");
			ret = 1;
		} else {
			puts("OK!\n");
		}
	} else {
		puts("skipped\n");
	}

	return ret;
}

/*
 * Verifies U-Boot image header checksum
 */
static int ih_header_crc(image_header_t *hdr, int tpl_type)
{
	u32 crc;
	int ret = 0;

	printf("   %-*s ", UBOOT_ALIGN_SIZE, "Header CRC...");

	if (tpl_type == 0) {
		crc = ntohl(hdr->ih_hcrc);
		hdr->ih_hcrc = 0;

		if (tinf_crc32((u8 *)hdr, sizeof(image_header_t)) != crc) {
			puts("ERROR\n\n");
			ret = 1;
		} else {
			puts("OK!\n");
		}

		hdr->ih_hcrc = crc;
	} else {
		puts("skipped\n");
	}

	return ret;
}

/*
 * Converts TP-Link header to stanard
 * U-Boot image format header
 */
static void tpl_to_uboot_header(image_header_t *hdr,
				tplink_image_header_t *tpl_hdr)
{
	memset(hdr, 0, sizeof(image_header_t));

	/* Set only needed values */
	hdr->ih_hcrc = 0;
	hdr->ih_dcrc = 0;

	hdr->ih_ep   = htonl(tpl_hdr->ih_kernel_ep);
	hdr->ih_size = htonl(tpl_hdr->ih_kernel_len);
	hdr->ih_load = htonl(tpl_hdr->ih_kernel_load);

	hdr->ih_os   = IH_OS_LINUX;
	hdr->ih_arch = IH_CPU_MIPS;
	hdr->ih_type = IH_TYPE_KERNEL;
	hdr->ih_comp = IH_COMP_LZMA;
}

int firmware_loader(int argc, char *argv[])
{
	char *s;
	u32 *len_ptr;
	u32 addr, data, len;
	int i, tpl_type;
	u32 unc_len = CFG_BOOTM_LEN;
	image_header_t *hdr = &header;
	tplink_image_header_t *tpl_hdr;

	if (argc < 2) {
		addr = load_addr;
	} else {
		addr = simple_strtoul(argv[1], NULL, 16);
	}

	rt_kprintf("Booting image from 0x%08lX...\n", addr);

	/* Check what header type we have */
	rt_memmove(&data, (char *)addr, sizeof(u32));
	tpl_type = 0;

	switch (ntohl(data)) {
	case TPL_IH_VERSION_V1:
	case TPL_IH_VERSION_V2:
		tpl_type = 1;
		tpl_hdr = (tplink_image_header_t *)addr;
		tpl_to_uboot_header(hdr, tpl_hdr);
		break;
	case IH_MAGIC:
		rt_memmove(&header, (char *)addr, sizeof(image_header_t));
		break;
	case TPL_IH_VERSION_V3:
	default:
		rt_kprintf("unsupported image header\n");
		return 1;
	}

	/* And data if enabled */
	if (tpl_type) {
		data = addr + sizeof(tplink_image_header_t);
	} else {
		data = addr + sizeof(image_header_t);
	}
	
	rt_kprintf("\n");

	len = ntohl(hdr->ih_size);
	len_ptr = (u32 *)data;

	/* We support only MIPS */
	if (hdr->ih_arch != IH_CPU_MIPS) {
		rt_kprintf("unsupported architecture!\n");
		return 1;
	}

	/* Image type... */
	switch (hdr->ih_type) {
	case IH_TYPE_KERNEL:
		break;
	case IH_TYPE_MULTI:
		/* OS kernel is always in first image */
		len = ntohl(len_ptr[0]);
		data += 8;
		/* Move over list to first image */
		for (i = 1; len_ptr[i]; ++i)
			data += 4;
		break;
	default:
		rt_kprintf("unsupported image type!\n");
		return 1;
	}
	
	mips_cache_flush();
	mips_icache_flush_ix();

	/* Compression type... */
	switch (hdr->ih_comp) {
	case IH_COMP_LZMA:
		rt_kprintf("Uncompressing %s... ", ih_img_type(hdr->ih_type));
		/* Try to extract LZMA data... */
		i = lzma_inflate((u8 *)data, len,
			(u8 *)ntohl(hdr->ih_load), (int *)&unc_len);
		/* TODO: more verbose LZMA errors */
		if (i != LZMA_RESULT_OK) {
			rt_kprintf("LZMA error '%d'!\n", i);
			return 1;
		}
		rt_kprintf("OK!\n");
		break;
	default:
		rt_kprintf("unsupported compression type '%s'!\n",
			   ih_comp_type(hdr->ih_comp));

		return 1;
	}

	do_bootm_linux(argc, argv);

	return 1;
}