#ifndef __SYSTEM_INIT_H__
#define __SYSTEM_INIT_H__

void system_early_init(rt_uint32_t bootflag);
void system_late_init(void);
void system_deinit(void);

#endif