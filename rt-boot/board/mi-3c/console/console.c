/*
 */

#include <kernel/rtthread.h>
#include <kernel/rthw.h>
#include <global/global.h>
#include <soc/mt7628/mt7628_mmap.h>
#include <soc/mt7628/mt7628_ls_uart.h>
#include <soc/mt7628/mt7628_irq.h>

void rt_hw_console_init(void)
{
	mt7628_liteuart_init();
}

void rt_hw_console_output(const char *str)
{
    mt7628_liteuart_puts(str);
}

struct rt_device soc_console_dev;

static rt_err_t rt_mt7628_liteuart_open(rt_device_t dev, rt_uint16_t oflag)
{
    return RT_EOK;
}

static rt_err_t rt_mt7628_liteuart_close(rt_device_t dev)
{
    return RT_EOK;
}

static rt_size_t rt_mt7628_liteuart_read(rt_device_t dev, rt_off_t pos, void* buffer, rt_size_t size)
{
	char *ptr = (char*) buffer;
	for(;size > 0;size--)
	{
		if(!mt7628_liteuart_tstc())
			break;
		//*ptr++ = qca_soc_reg_read(QCA_liteuart_RBR_REG)& QCA_liteuart_RBR_RBR_MASK;
	}
	
	//if(size > 0)
		//qca_soc_reg_read_set(QCA_liteuart_IER_REG, QCA_liteuart_IER_ERBFI_MASK);
	
	return (rt_size_t)ptr - (rt_size_t)buffer;
}

static rt_size_t rt_mt7628_liteuart_write(rt_device_t dev, rt_off_t pos, const void* buffer, rt_size_t size)
{
	char *ptr = (char*) buffer;
	for(;size > 0;size--)
	{
		mt7628_liteuart_putc(*ptr++);
	}
	return (rt_size_t)ptr - (rt_size_t)buffer;
}

static void rt_mt7628_liteuart_irq_handler(int vector,void * para)
{
	//qca_soc_reg_read(QCA_liteuart_IIR_REG);
	//if(qca_soc_reg_read(QCA_liteuart_LSR_REG) & QCA_liteuart_LSR_DR_MASK)
	{
		if (soc_console_dev.rx_indicate != RT_NULL)
    	{
			//qca_soc_reg_read_clear(QCA_liteuart_IER_REG, QCA_liteuart_IER_ERBFI_MASK);
        	soc_console_dev.rx_indicate(&soc_console_dev, 1);
    	}
		else
		{
			while(1)
			{
				if(!mt7628_liteuart_tstc())
					break;
				//if(qca_soc_reg_read(QCA_liteuart_RBR_REG)& QCA_liteuart_RBR_RBR_MASK) ;
			}
		}
	}
}

static rt_err_t rt_mt7628_liteuart_init (rt_device_t dev)
{
    mt7628_liteuart_init();
	
	mt7628_interrupt_install(20, rt_mt7628_liteuart_irq_handler,0, "mt7628_liteuart_handler");
	
    mt7628_interrupt_mask(20);

	//qca_soc_reg_read_set(QCA_liteuart_IER_REG, QCA_liteuart_IER_ERBFI_MASK);
	
    return RT_EOK;
}

int soc_console_init(void)
{
	/* device initialization */
	soc_console_dev.type = RT_Device_Class_Char;

	/* device interface */
	soc_console_dev.init 	   = rt_mt7628_liteuart_init;
	soc_console_dev.open 	   = rt_mt7628_liteuart_open;
	soc_console_dev.close      = rt_mt7628_liteuart_close;
	soc_console_dev.read 	   = rt_mt7628_liteuart_read;
	soc_console_dev.write      = rt_mt7628_liteuart_write;
	soc_console_dev.control    = RT_NULL;
	soc_console_dev.user_data  = RT_NULL;
	soc_console_dev.rx_indicate = RT_NULL;
    soc_console_dev.tx_complete = RT_NULL;

	rt_device_register(&soc_console_dev, RT_CONSOLE_DEVICE_NAME, RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_STREAM | RT_DEVICE_FLAG_INT_RX);
	
	return 0;
}
