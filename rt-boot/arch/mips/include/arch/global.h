#ifndef __ARCH_GLOBAL_H__
#define __ARCH_GLOBAL_H__

extern rt_uint32_t rtboot_data_end;
extern rt_uint32_t rtboot_end;
extern rt_uint32_t __got_end;
extern rt_uint32_t __got_start;
extern rt_uint32_t __fsymtab_end;
extern rt_uint32_t __fsymtab_start;
extern rt_uint32_t __vsymtab_end;
extern rt_uint32_t __vsymtab_start;
extern rt_uint32_t __rtobject_end;
extern rt_uint32_t __rtobject_start;

#define TOTAL_MALLOC_LEN	(1024 * 1024)
#define CFG_BOOTPARAMS_LEN	(512 * 1024)
#define CFG_MONITOR_BASE	TEXT_BASE
#define CFG_SDRAM_BASE		0x80000000

#endif