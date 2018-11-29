#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <arch/global.h>
#include <soc/global.h>

typedef struct __system_boot_data{
	#include <arch/global_data_arch_part.h>
	#include <soc/global_data_soc_part.h>
	rt_uint32_t system_frequency;
	rt_uint32_t system_baudrate;
}system_boot_data;

extern volatile system_boot_data rtboot_data;

#endif