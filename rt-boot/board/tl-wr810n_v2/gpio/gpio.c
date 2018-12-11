#include <kernel/rtthread.h>
#include <kernel/rthw.h>
#include <drivers/rtdevice.h>

#include <global/global.h>
#include <main/main.h>

ALIGN(RT_ALIGN_SIZE)
static char gpio_thread_stack[0x200];
struct rt_thread gpio_thread;

static void gpio_thread_entry(void* parameter)
{
	rt_pin_mode(4,1);
	rt_pin_mode(11,1);
	rt_pin_mode(13,1);
	rt_pin_mode(14,1);
	rt_pin_mode(15,1);
	rt_pin_mode(16,1);
	
	rt_pin_mode(12,0);
	while(1)
	{
		if(rt_pin_read(12) == 0)
			boot_break_key_notisfy();
		
		rt_thread_mdelay(100);
		
		rt_pin_write(4,0);
		rt_pin_write(11,0);
		rt_pin_write(13,0);
		rt_pin_write(14,0);
		rt_pin_write(15,0);
		rt_pin_write(16,0);
		
		rt_thread_mdelay(100);
		
		rt_pin_write(4,1);
		rt_pin_write(11,1);
		rt_pin_write(13,1);
		rt_pin_write(14,1);
		rt_pin_write(15,1);
		rt_pin_write(16,1);
		
	}
}

void gpio_thread_init(void)
{
    rt_thread_init(&gpio_thread,
                   "gpio_thread",
                   &gpio_thread_entry,
                   RT_NULL,
                   &gpio_thread_stack[0],
                   sizeof(gpio_thread_stack),9,6);

    rt_thread_startup(&gpio_thread);
}