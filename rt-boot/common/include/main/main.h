#ifndef __SYSTEM_MAIN_H__
#define __SYSTEM_MAIN_H__

int rt_application_init(void);
void boot_break_net_notisfy(void);
void boot_break_uart_notisfy(void);
void boot_break_key_notisfy(void);

#endif