	/* system memory information */
	rt_uint32_t system_memsize;
	rt_uint32_t boot_param_pointer;
	rt_uint32_t rtboot_length;
	rt_uint32_t relocation_ebase;
	rt_uint32_t malloc_start_base;
	rt_uint32_t malloc_end_base;
	rt_uint32_t relocation_base;
	rt_uint32_t start_stack_base;
	/* relocation information */
	rt_uint32_t old_gp;
	rt_uint32_t new_gp;
	rt_uint32_t relocation_offset;
	/* cache information */
	rt_uint32_t cp0_config1_raw;
	rt_uint32_t icache_sets_per_way_raw;
	rt_uint32_t icache_line_size_raw;
	rt_uint32_t icache_line_size;
	rt_uint32_t icache_associativity_level_raw;
	rt_uint32_t icache_associativity_level;
	rt_uint32_t icache_size;
	rt_uint32_t dcache_sets_per_way_raw;
	rt_uint32_t dcache_line_size_raw;
	rt_uint32_t dcache_line_size;
	rt_uint32_t dcache_associativity_level_raw;
	rt_uint32_t dcache_associativity_level;
	rt_uint32_t dcache_size;