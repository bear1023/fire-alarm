#include <MyApplication.h>

#define     T1_PORT                 (3)
#define     T1_PIN                  (3)



unsigned int Usart2_RxCounter = 0;      //定义一个变量，记录串口2总共接收了多少字节的数据
char Usart2_RxBuff[USART2_RXBUFF_SIZE]; //定义一个数组，用于保存串口2接收到的数据   	

//uint8_t u8TxData[2] = {0x00,0x55};
uint8_t u8RxData = 00;
uint8_t u8TxCnt=0,u8RxCnt=0;


void Uart0_IRQHandler(void)  //与8266通信
{
		if((Uart_GetStatus(M0P_UART0, UartFE))||(Uart_GetStatus(M0P_UART0, UartPE)))  //错误请求
    {
        Uart_ClrStatus(M0P_UART0, UartFE);            //清除帧错误标记
        Uart_ClrStatus(M0P_UART0, UartPE);            //清除奇偶校验错误标记
    }
		while(FALSE == Uart_GetStatus(M0P_UART0,UartTxe)){}
    if(Uart_GetStatus(M0P_UART0, UartRC))         //UART0数据接收
    {
        Uart_ClrStatus(M0P_UART0, UartRC);        //清中断状态位
			  //Usart2_RxBuff[Usart2_RxCounter]=M0P_UART0->SBUF;
				Usart2_RxBuff[Usart2_RxCounter]=Uart_ReceiveData(M0P_UART0);				
				Usart2_RxCounter++;
				//Uart_SendDataPoll(M0P_UART1,Uart_ReceiveData(M0P_UART0));
    }
	  if(Uart_GetStatus(M0P_UART0, UartTC))         //UART0数据发送
    {
        Uart_ClrStatus(M0P_UART0, UartTC);        //清中断状态位
        //u8TxCnt++;
    }
	
}
void Uart1_IRQHandler(void)
{
    if(Uart_GetStatus(M0P_UART1, UartRC))         //UART1数据接收
    {
        Uart_ClrStatus(M0P_UART1, UartRC);        //清中断状态位
        u8RxData = Uart_ReceiveData(M0P_UART1);   //接收数据字节
        u8RxCnt++;
    }

    if(Uart_GetStatus(M0P_UART1, UartTC))         //UART1数据发送
    {
        Uart_ClrStatus(M0P_UART1, UartTC);        //清中断状态位
        u8TxCnt++;
    }
}
void Usart0_PortInit(void)
{
    stc_gpio_cfg_t stcGpioCfg;
    
    DDL_ZERO_STRUCT(stcGpioCfg);
    
    Sysctrl_SetPeripheralGate(SysctrlPeripheralGpio,TRUE); //GPIO外设模块时钟使能
    
    stcGpioCfg.enDir = GpioDirOut;
    Gpio_Init(GpioPortA,GpioPin9,&stcGpioCfg);
    Gpio_SetAfMode(GpioPortA,GpioPin9,GpioAf1); //配置PA09 为UART0 TX
	
    stcGpioCfg.enDir = GpioDirIn;
    Gpio_Init(GpioPortA,GpioPin10,&stcGpioCfg);
    Gpio_SetAfMode(GpioPortA,GpioPin10,GpioAf1);//配置PA10 为UART0 RX
}

void Usart1_PortInit(void)
{
    stc_gpio_cfg_t stcGpioCfg;

    DDL_ZERO_STRUCT(stcGpioCfg);

    Sysctrl_SetPeripheralGate(SysctrlPeripheralGpio,TRUE); //使能GPIO模块时钟

    ///<TX
    stcGpioCfg.enDir = GpioDirOut;
    Gpio_Init(GpioPortA, GpioPin2, &stcGpioCfg);
    Gpio_SetAfMode(GpioPortA, GpioPin2, GpioAf1);          //配置PA02 端口为URART1_TX

    ///<RX
    stcGpioCfg.enDir = GpioDirIn;
    Gpio_Init(GpioPortA, GpioPin3, &stcGpioCfg);
    Gpio_SetAfMode(GpioPortA, GpioPin3, GpioAf1);          //配置PA03 端口为URART1_RX
}
void Uart0Init(void)
{
    stc_uart_cfg_t    stcCfg;

    DDL_ZERO_STRUCT(stcCfg);

    ///< 开启外设时钟
    Sysctrl_SetPeripheralGate(SysctrlPeripheralUart0,TRUE);///<使能uart0模块时钟

    ///<UART Init
    stcCfg.enRunMode        = UartMskMode1;          ///<模式3
    stcCfg.enStopBit        = UartMsk1bit;           ///<1bit停止位
    stcCfg.enMmdorCk        = UartMskEven;           ///<偶检验
    stcCfg.stcBaud.u32Baud  = 9600;                  ///<波特率9600
    stcCfg.stcBaud.enClkDiv = UartMsk8Or16Div;       ///<通道采样分频配置
    stcCfg.stcBaud.u32Pclk  = Sysctrl_GetPClkFreq(); ///<获得外设时钟（PCLK）频率值
    Uart_Init(M0P_UART0, &stcCfg);                   ///<串口初始化

    ///<UART中断使能
    Uart_ClrStatus(M0P_UART0,UartRC);                ///<清接收请求
    Uart_ClrStatus(M0P_UART0,UartTC);                ///<清接收请求
    Uart_EnableIrq(M0P_UART0,UartRxIrq);             ///<使能串口接收中断
    Uart_EnableIrq(M0P_UART0,UartTxIrq);             ///<使能串口接收中断
    EnableNvic(UART0_IRQn, IrqLevel3, TRUE);       ///<系统中断使能

}
void Uart1Init(void)
{
    stc_uart_cfg_t    stcCfg;

    DDL_ZERO_STRUCT(stcCfg);

    ///< 开启外设时钟
    Sysctrl_SetPeripheralGate(SysctrlPeripheralUart1,TRUE);///<使能uart1模块时钟

    ///<UART Init
    stcCfg.enRunMode        = UartMskMode1;          ///<模式3
    stcCfg.enStopBit        = UartMsk1bit;           ///<1bit停止位
    stcCfg.enMmdorCk        = UartMskEven;           ///<偶检验
    stcCfg.stcBaud.u32Baud  = 9600;                  ///<波特率9600
    stcCfg.stcBaud.enClkDiv = UartMsk8Or16Div;       ///<通道采样分频配置
    stcCfg.stcBaud.u32Pclk  = Sysctrl_GetPClkFreq(); ///<获得外设时钟（PCLK）频率值
    Uart_Init(M0P_UART1, &stcCfg);                   ///<串口初始化

    ///<UART中断使能
    Uart_ClrStatus(M0P_UART1,UartRC);                ///<清接收请求
    Uart_ClrStatus(M0P_UART1,UartTC);                ///<清接收请求
    Uart_EnableIrq(M0P_UART1,UartRxIrq);             ///<使能串口接收中断
    Uart_EnableIrq(M0P_UART1,UartTxIrq);             ///<使能串口接收中断
		
}


/*
	* @name   fputc
	* @brief  fputc映射物理串口
	* @param  ch -> 待发送字符
	* @retval ch -> 已发送字符      
*/
int fputc(int ch,FILE* f)
{
	Uart_SendDataPoll(M0P_UART1,ch);
	//LPUart_SendData(M0P_LPUART1, ch);
	//Uart_SendDataPollTimeOut(M0P_UART0,ch,1000);			

	return ch;
}
void App_LpUartClkCfg(void)
{
    Sysctrl_SetRCLTrim(SysctrlRclFreq38400);        ///< 配置RCL时钟为38.4kHz
    Sysctrl_ClkSourceEnable(SysctrlClkRCL, TRUE);   ///< 使能RCL时钟
}

///< LPUART配置
void App_LpUartCfg(void)
{
    stc_lpuart_cfg_t  stcCfg;

    DDL_ZERO_STRUCT(stcCfg);                        ///< 结构体变量初始值置零

    ///<外设模块时钟使能
    Sysctrl_SetPeripheralGate(SysctrlPeripheralLpUart1,TRUE);

    ///<LPUART 初始化
    stcCfg.enStopBit = LPUart1bit;                   ///<1停止位
    stcCfg.enMmdorCk = LPUartEven;                   ///<偶校验
    stcCfg.stcBaud.enSclkSel = LPUartMskRcl;         ///<传输时钟源
    stcCfg.stcBaud.u32Sclk = 38400;                  ///<RCL时钟频率 38400Hz
    stcCfg.stcBaud.enSclkDiv = LPUartMsk4Or8Div;     ///<采样分频
    stcCfg.stcBaud.u32Baud = 9600;                   ///<波特率
    stcCfg.enRunMode = LPUartMskMode3;               ///<工作模式
    LPUart_Init(M0P_LPUART1, &stcCfg);

    ///<LPUART 中断使能
    LPUart_ClrStatus(M0P_LPUART1,LPUartRC);         ///<清接收中断请求
    LPUart_ClrStatus(M0P_LPUART1,LPUartTC);         ///<清发送中断请求
    LPUart_DisableIrq(M0P_LPUART1,LPUartRxIrq);     ///<禁止接收中断
    LPUart_EnableIrq(M0P_LPUART1,LPUartTxIrq);      ///<使能发送中断
    EnableNvic(LPUART1_IRQn,IrqLevel3,TRUE);        ///<系统中断使能
}


///< 端口配置
void App_LpUartPortCfg(void)
{
    stc_gpio_cfg_t stcGpioCfg;

    DDL_ZERO_STRUCT(stcGpioCfg);                            ///< 结构体变量初始值置零

    Sysctrl_SetPeripheralGate(SysctrlPeripheralGpio,TRUE);  ///< 使能GPIO时钟

    ///<深度休眠模式外部端口配置(STK)
    //App_LpUartDeepSleepCfg();

    ///<TX
    stcGpioCfg.enDir =  GpioDirOut;
    Gpio_Init(GpioPortA,GpioPin0,&stcGpioCfg);
    Gpio_SetAfMode(GpioPortA,GpioPin0,GpioAf2); //配置PA00为LPUART1_TX

    //<RX
    stcGpioCfg.enDir =  GpioDirIn;
    Gpio_Init(GpioPortA,GpioPin1,&stcGpioCfg);
    Gpio_SetAfMode(GpioPortA,GpioPin1,GpioAf2); //配置PA01为LPUART1_RX
}

///<深度休眠模式外部端口配置(STK)
void App_LpUartDeepSleepCfg(void)
{
    stc_gpio_cfg_t stcGpioCfg;

    ///< 打开GPIO外设时钟门控
    Sysctrl_SetPeripheralGate(SysctrlPeripheralGpio, TRUE);

    //swd as gpio
    Sysctrl_SetFunc(SysctrlSWDUseIOEn, TRUE);

    ///< 配置为数字端口
    M0P_GPIO->PAADS = 0;
    M0P_GPIO->PBADS = 0;
    M0P_GPIO->PCADS = 0;
    M0P_GPIO->PDADS = 0;

    ///< 配置为端口输入（除LED端口外）
    M0P_GPIO->PADIR = 0XFFFF;
    M0P_GPIO->PBDIR = 0XFFFF;
    M0P_GPIO->PCDIR = 0XFFFF;
    M0P_GPIO->PDDIR = 0XFFDF;

    ///< 输入下拉（除KEY端口以外）
    M0P_GPIO->PAPD = 0xFFFF;
    M0P_GPIO->PBPD = 0xFFFF;
    M0P_GPIO->PCPD = 0xFFFF;
    M0P_GPIO->PDPD = 0xFFEF;

    ///< 端口方向配置->输入
    stcGpioCfg.enDir = GpioDirIn;
    ///< 端口驱动能力配置->高驱动能力
    stcGpioCfg.enDrv = GpioDrvL;
    ///< 端口上下拉配置->无
    stcGpioCfg.enPu = GpioPuDisable;
    stcGpioCfg.enPd = GpioPdDisable;
    ///< 端口开漏输出配置->开漏输出关闭
    stcGpioCfg.enOD = GpioOdDisable;
    ///< 端口输入/输出值寄存器总线控制模式配置->AHB
    stcGpioCfg.enCtrlMode = GpioAHB;
    ///< GPIO IO USER KEY初始化
    Gpio_Init(STK_USER_PORT, STK_USER_PIN, &stcGpioCfg);

}

///<LPUART1 中断服务函数
void LpUart1_IRQHandler(void)
{
    if(LPUart_GetStatus(M0P_LPUART1, LPUartTC))       ///接收数据中断
    {
        LPUart_ClrStatus(M0P_LPUART1, LPUartTC);      ///<清发送中断请求

        LPUart_DisableIrq(M0P_LPUART1,LPUartTxIrq);   ///<禁止发送中断
        LPUart_EnableIrq(M0P_LPUART1,LPUartRxIrq);    ///<使能接收中断
    }

    if(LPUart_GetStatus(M0P_LPUART1, LPUartRC))       ///接收数据中断
    {
        LPUart_ClrStatus(M0P_LPUART1, LPUartRC);      ///<清接收中断请求
        u8RxData = LPUart_ReceiveData(M0P_LPUART1);   ///读取数据

        LPUart_DisableIrq(M0P_LPUART1,LPUartRxIrq);   ///<禁止接收中断
        LPUart_EnableIrq(M0P_LPUART1,LPUartTxIrq);    ///<使能发送中断

        LPUart_SendDataIt(M0P_LPUART1, ~u8RxData);    ///把接收数据取反，并发送
    }

}



