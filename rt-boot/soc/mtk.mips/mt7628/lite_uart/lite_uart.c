/*
 */

#include <kernel/rtthread.h>
#include <global/global.h>
#include <soc/mt7628/mt7628_mmap.h>
#include <soc/mt7628/mt7628_lite_uart.h>

static void mt7628_liteuart_setbrg(rt_uint32_t baudrate)
{
	unsigned int clock_divisor = 0;
	
	clock_divisor = (40*1000*1000/ SERIAL_CLOCK_DIVISOR / baudrate);
	
	IER(CFG_RT2880_CONSOLE) = 0;		/* Disable for now */
	FCR(CFG_RT2880_CONSOLE) = 0;		/* No fifos enabled */

	/* set baud rate */
	LCR(CFG_RT2880_CONSOLE) = LCR_WLS0 | LCR_WLS1 | LCR_DLAB;
	DLL(CFG_RT2880_CONSOLE) = clock_divisor & 0xff;
	DLM(CFG_RT2880_CONSOLE) = clock_divisor >> 8;
	LCR(CFG_RT2880_CONSOLE) = LCR_WLS0 | LCR_WLS1;
}

void mt7628_liteuart_init(void)
{
	__REG(RALINK_SYS_RST_REG) |= RALINK_UART_RST | RALINK_UARTL_RST;
	/* RST Control change from W1C to W1W0 to reset, update 20080812 */
	__REG(RALINK_SYS_RST_REG) &= ~(RALINK_UART_RST | RALINK_UARTL_RST);
	
	mt7628_liteuart_setbrg(rtboot_data.system_baudrate);
}

void mt7628_liteuart_putc(const char c)
{
	/* wait for room in the tx FIFO on UART */
	while ((LSR(CFG_RT2880_CONSOLE) & LSR_TEMT) == 0);

	TBR(CFG_RT2880_CONSOLE) = c;

	/* If \n, also do \r */
	if (c == '\n')
		mt7628_liteuart_putc('\r');
}

int mt7628_liteuart_getc(void)
{
	while (!(LSR(CFG_RT2880_CONSOLE) & LSR_DR));
	return (char) RBR(CFG_RT2880_CONSOLE) & 0xff;
}

int mt7628_liteuart_tstc(void)
{
	return LSR(CFG_RT2880_CONSOLE) & LSR_DR;
}

void mt7628_liteuart_puts(const char *s)
{
	while (*s)
		mt7628_liteuart_putc(*s++);
}
