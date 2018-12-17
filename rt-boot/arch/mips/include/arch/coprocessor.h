#ifndef __ARCH_COPROCESSOR_H__
#define __ARCH_COPROCESSOR_H__

#define MIPS32R2 2
#define MIPS32R0 0

rt_uint32_t mips_cpu_version(void);
void mips_cp0_init(void);

#endif
