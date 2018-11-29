#include <kernel/rtthread.h>
#include <kernel/rthw.h>
#include <global/global.h>

#include <arch/mipsregs.h>

void arch_timer_udelay(rt_uint32_t us)
{
	rt_uint32_t delay_count;
	rt_uint32_t start = read_c0_count();

	delay_count = us * (rtboot_data.system_frequency/1000000);

	while ((rt_uint32_t)((read_c0_count() - start)) < delay_count)
		/* NOP */;	
}

/**
 * This is the timer interrupt service routine.
 */
void arch_timer_handler(int vector,void * para)
{
	unsigned int count;
	count = read_c0_count();
	count += rtboot_data.system_frequency/RT_TICK_PER_SECOND;
	write_c0_compare(count);

	/* increase a OS tick */
	rt_tick_increase();
}

/**
 * This function will initial OS timer
 */
void arch_timer_init(void)
{
	rt_hw_interrupt_install(7, arch_timer_handler,0, "sys_tick_irq");
	write_c0_compare(rtboot_data.system_frequency/RT_TICK_PER_SECOND);
	write_c0_count(0);
	rt_hw_interrupt_mask(7);
}
