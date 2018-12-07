/*
 * The early init code for MIPS
 * Copyright ZhaoXiaowei 2018
 * Github:github.com/zhaohengbo
 */

#include <kernel/rtthread.h>
#include <kernel/rthw.h>
#include <drivers/rtdevice.h>
#include <global/global.h>
#include <soc/qca953x/qca953x_map.h>
#include <soc/qca953x/qca953x_gpio.h>

void qca953x_pin_write(rt_device_t dev, rt_base_t pin, rt_base_t value)
{
	if (value == PIN_LOW)
	{
		qca_soc_reg_write(QCA_GPIO_CLEAR_REG,(1<<pin));
	}
	else
	{
		qca_soc_reg_write(QCA_GPIO_SET_REG,(1<<pin));
	}
}

int qca953x_pin_read(rt_device_t dev, rt_base_t pin)
{
	int value;
	value = (qca_soc_reg_read(QCA_GPIO_IN_REG) & (1<<pin));
    if (value)
    {
        return PIN_HIGH;
    }
    else
    {
        return PIN_LOW;
    }
}

void qca953x_pin_mode(rt_device_t dev, rt_base_t pin, rt_base_t mode)
{
	if (mode == PIN_MODE_OUTPUT)
		qca_soc_reg_read_set(QCA_GPIO_OE_REG,(1<<pin));
	else if (mode == PIN_MODE_INPUT)	
		qca_soc_reg_read_clear(QCA_GPIO_OE_REG,(1<<pin));
}

const static struct rt_pin_ops _qca953x_pin_ops =
{
	qca953x_pin_mode,
    qca953x_pin_write,
    qca953x_pin_read,
    RT_NULL,
    RT_NULL,
    RT_NULL,
};

rt_err_t qca953x_pin_init(void)
{
    int result;
    result = rt_device_pin_register("pin", &_qca953x_pin_ops, RT_NULL);
    return result;
}

void qca953x_gpio_init(void)
{
	qca953x_pin_init();
}
