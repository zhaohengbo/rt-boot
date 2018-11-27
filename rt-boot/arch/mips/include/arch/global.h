#ifndef __ARCH_GLOBAL_H__
#define __ARCH_GLOBAL_H__

extern rt_uint32_t rtboot_data_end;
extern rt_uint32_t rtboot_end;
extern rt_uint32_t __got_end;
extern rt_uint32_t __got_start;
extern rt_uint32_t __rtobject_end;
extern rt_uint32_t __rtobject_start;
extern rt_uint32_t __fsymtab_end;
extern rt_uint32_t __fsymtab_start;
extern rt_uint32_t __vsymtab_end;
extern rt_uint32_t __vsymtab_start;
extern rt_uint32_t __lwip_tcp_pcb_list_end;
extern rt_uint32_t __lwip_tcp_pcb_list_start;
extern rt_uint32_t __lwip_tcp_state_str_end;
extern rt_uint32_t __lwip_tcp_state_str_start;
extern rt_uint32_t __lwip_mempool_end;
extern rt_uint32_t __lwip_mempool_start;
extern rt_uint32_t __lwip_mempool_desc_end;
extern rt_uint32_t __lwip_mempool_desc_start;
extern rt_uint32_t __lwip_cyclic_timer_end;
extern rt_uint32_t __lwip_cyclic_timer_start;
extern rt_uint32_t __lwip_err_str_end;
extern rt_uint32_t __lwip_err_str_start;
extern rt_uint32_t __lwip_altcp_tcp_function_end;
extern rt_uint32_t __lwip_altcp_tcp_function_start;

#define TOTAL_MALLOC_LEN	(1024 * 1024)
#define CFG_BOOTPARAMS_LEN	(512 * 1024)
#define CFG_MONITOR_BASE	TEXT_BASE
#define CFG_SDRAM_BASE		0x80000000

#endif