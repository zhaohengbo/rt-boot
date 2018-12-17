#ifndef __CACHE_H__
#define __CACHE_H__

void arch_cache_init(void);
void arch_dcache_flush(rt_uint32_t start_addr,rt_uint32_t size);
void arch_dcache_invalidate(rt_uint32_t start_addr,rt_uint32_t size);
void arch_icache_flush(rt_uint32_t start_addr,rt_uint32_t size);
void arch_cache_flush(rt_uint32_t start_addr,rt_uint32_t size);

#endif
