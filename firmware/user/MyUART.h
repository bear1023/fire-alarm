#ifndef __MYUART_H
#define __MYUART_H

#include "ddl.h"
#include "lptim.h"
#include "lpm.h"
#define USART2_RXBUFF_SIZE   1024              //定义串口2 接收缓冲区大小 1024字节

extern unsigned int Usart2_RxCounter;          //外部声明，其他文件可以调用该变量
extern char Usart2_RxBuff[USART2_RXBUFF_SIZE]; //外部声明，其他文件可以调用该变量

void App_LpUartDeepSleepCfg(void);
void App_LpUartPortCfg(void);
void App_LpUartClkCfg(void);
void App_LpUartCfg(void);
void Uart1Init(void);
void Usart1_PortInit(void);
void Uart0Init(void);
void Usart0_PortInit(void);
void RxIntCallback(void);
void ErrIntCallback(void);
#endif
