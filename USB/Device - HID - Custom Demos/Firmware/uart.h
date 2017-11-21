/*********************************************************************
* FileName:        uart.h
********************************************************************/

#ifndef UART_H
#define UART_H

extern char RcvUartBuf[4];
extern char TransUartBuf[40];
extern unsigned char g_btready;
extern unsigned char g_bt_init;

extern void uart_init(void);
extern void bt_config(void);
extern void uart_put_str(char* tx_buff);

#endif

