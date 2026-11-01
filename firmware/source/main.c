/******************************************************************************
* Copyright (C) 2018, Xiaohua Semiconductor Co.,Ltd All rights reserved.
*
* This software is owned and published by:
* Xiaohua Semiconductor Co.,Ltd ("XHSC").
*
* BY DOWNLOADING, INSTALLING OR USING THIS SOFTWARE, YOU AGREE TO BE BOUND
* BY ALL THE TERMS AND CONDITIONS OF THIS AGREEMENT.
*
* This software contains source code for use with XHSC
* components. This software is licensed by XHSC to be adapted only
* for use in systems utilizing XHSC components. XHSC shall not be
* responsible for misuse or illegal use of this software for devices not
* supported herein. XHSC is providing this software "AS IS" and will
* not be responsible for issues arising from incorrect user implementation
* of the software.
*
* Disclaimer:
* XHSC MAKES NO WARRANTY, EXPRESS OR IMPLIED, ARISING BY LAW OR OTHERWISE,
* REGARDING THE SOFTWARE (INCLUDING ANY ACOOMPANYING WRITTEN MATERIALS),
* ITS PERFORMANCE OR SUITABILITY FOR YOUR INTENDED USE, INCLUDING,
* WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, THE IMPLIED
* WARRANTY OF FITNESS FOR A PARTICULAR PURPOSE OR USE, AND THE IMPLIED
* WARRANTY OF NONINFRINGEMENT.
* XHSC SHALL HAVE NO LIABILITY (WHETHER IN CONTRACT, WARRANTY, TORT,
* NEGLIGENCE OR OTHERWISE) FOR ANY DAMAGES WHATSOEVER (INCLUDING, WITHOUT
* LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION,
* LOSS OF BUSINESS INFORMATION, OR OTHER PECUNIARY LOSS) ARISING FROM USE OR
* INABILITY TO USE THE SOFTWARE, INCLUDING, WITHOUT LIMITATION, ANY DIRECT,
* INDIRECT, INCIDENTAL, SPECIAL OR CONSEQUENTIAL DAMAGES OR LOSS OF DATA,
* SAVINGS OR PROFITS,
* EVEN IF Disclaimer HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* YOU ASSUME ALL RESPONSIBILITIES FOR SELECTION OF THE SOFTWARE TO ACHIEVE YOUR
* INTENDED RESULTS, AND FOR THE INSTALLATION OF, USE OF, AND RESULTS OBTAINED
* FROM, THE SOFTWARE.
*
* This software may be replicated in part or whole for the licensed use,
* with the restriction that this Disclaimer and Copyright notice must be
* included with each copy of this software, whether used in part or whole,
* at all times.
*/
/******************************************************************************/
/** \file main.c
 **
 ** A detailed description is available at
 ** @link Sample Group Some description @endlink
 **
 **   - 2018-05-08  1.0  Lux First version for Device Driver Library of Module.
 **
 ******************************************************************************/

/******************************************************************************
 * Include files
 ******************************************************************************/
#include "MyApplication.h"
/******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/

/******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/******************************************************************************
 * Local variable definitions ('static')                                      *
 ******************************************************************************/

/******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/

/*****************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
static void App_LedInit(void);
static void App_LowPowerModeGpioSet(void);
static void App_SysClkInit(void);
void SystemClkConfig(void);
void WdtInit(void);

/**
 ******************************************************************************
 ** \brief  Main function of project
 **
 ** \return uint32_t return value, if needed
 **
 ** This sample
 **
 ******************************************************************************/

int32_t main(void)
{
	delay1ms(2000);
	//SystemClkConfig();
	App_SysClkInit();
	Sysctrl_SetPeripheralGate(SysctrlPeripheralBaseTim, TRUE);
	Timer2Init();  

	//WdtInit();
	Uart1Init(); //串口1用于打印log P23  
	Usart1_PortInit();
	Usart0_PortInit();
	Uart0Init(); //串口0用于与8266通信   P35 36
	
  App_AdcPortInit();//< ADC 采样端口初始化 
  App_AdcInit();    //< ADC 模块初始化  
  App_AdcSglCfg();  ///< ADC 顺序扫描 功能配置
	
	App_GpioInit();
	App_UserKeyInit();
		
//	SHT30_Init();
//	SHT30_Read();
	ESP8266_Init();
	ESP8266_GetMAC();
	
	printf("Error\r\n");
	if(ReadFlashWIFI()==1) 
	{
		delay1ms(500);
		ESP8266_Init();
		delay1ms(500);
		Timer2_CNT=0;
		if(Timer2_CNT> TIMER2_10S)
		{
			//printf("Error\r\n");
			NVIC_SystemReset();		
		}
		ESP8266_ConnectWiFi();
		FlagWiFiConfig=0;
		WiFiConfig();
	}
	else  //联网失败
	{
		FlagWiFiConfig=1;	
		WiFiConfig();
	}
	//ESP8266_ConnectWiFi();
	ConnectMQTT();
	MQTTPOST();
	GetNtpTime();
	//RTC_Init();
	Wdt_Feed();
	App_LPTimerInit();
	//这里开启外部中断
  //Gpio_ClearIrq(GpioPortB, GpioPin4);
  Gpio_EnableIrq(GpioPortB, GpioPin4, GpioIrqRising);   //上升沿触发
  EnableNvic(PORTB_IRQn, IrqLevel3, TRUE);
	//EnableNvic(RTC_IRQn, DDL_IRQ_LEVEL_DEFAULT, FALSE);//失能RTC中断（暂时不用）
	if(FlagDay==1&&FlagNight==0) //如果是白天
	{
		ESP8266_Sleep();  //休眠，每小时唤醒一次就行
	}
	else if (FlagDay==0&&FlagNight==1) //如果是晚上
	{
		ESP8266_Sleep();
		TIM2_FlagStart=1;//开始计数2Min
	}
	sleep_GpioInit();
	printf("Reset!!!!!!!!!!!!!!!!!!\r\n");
  //App_LowPowerModeGpioSet();///< 配置Demo板上所有不使用的IO为高阻输入,避免端口漏电
 
	
	

	while(1)
	{
		Wdt_Feed();//喂狗操作	
		
if(FlagOneHour==1)//如果一小时达到
	{
			FlagOneHour=0;
			printf("FlagOneHour START\r\n");

			//同步波特率
			Bt_M0_Stop(TIM2);
			Timer2_CNT =0;
			Bt_M0_Run(TIM2);
			Usart2_RxCounter=0;  		//WiFi接收数据量变量清零    
				
			memset(Usart2_RxBuff,0,USART2_RXBUFF_SIZE);     //清空WiFi接收缓冲区 
			Gpio_ClrIO(WIFI_RST_GPIO_Port,WIFI_RST_Pin);
			delay1ms(100);
			Gpio_SetIO(WIFI_RST_GPIO_Port,WIFI_RST_Pin);
			delay1ms(500);
			do
			{	
				//延时100ms，等待接收完成
				delay1ms(1000);		
				//打印信息
				printf("%s",Usart2_RxBuff);	
				//超时处理
				if(Timer2_CNT> TIMER2_10S)
				{
					printf("Error\r\n");
					NVIC_SystemReset();
					break;
				}
			}
			while(strstr((const char*)Usart2_RxBuff,"ready") == NULL);
			delay1ms(500);
			ESP8266_ConnectWiFi();
			Wdt_Feed();
			ConnectMQTT();
			MQTTPOST();
			ESP8266_Sleep();
			printf("FlagOneHour END\r\n");
			EnableNvic(RTC_IRQn, DDL_IRQ_LEVEL_DEFAULT, TRUE);
	}
else if((FlagNight==1)&&(TIM2_FlagStart==1)&&(FlagNightPerson==1)&&(FlagFirst==0))//在晚上、正在计数2分钟、触发中断、两分钟的第一次 要复位一下
	{
	Wdt_Feed();

	FlagFirst=1;
	//同步波特率
	Bt_M0_Stop(TIM2);
	Timer2_2MinCNT=0;
	Timer2_CNT = 0;
	Bt_M0_Run(TIM2);
	Usart2_RxCounter=0;                           //WiFi接收数据量变量清零                        
	memset(Usart2_RxBuff,0,USART2_RXBUFF_SIZE);     //清空WiFi接收缓冲区 
	Gpio_ClrIO(WIFI_RST_GPIO_Port,WIFI_RST_Pin);
	delay1ms(100);
	Gpio_SetIO(WIFI_RST_GPIO_Port,WIFI_RST_Pin);
	delay1ms(500);
	do
	{	
		//延时100ms，等待接收完成
		delay1ms(1000);		
		//打印信息
		printf("%s",Usart2_RxBuff);	
		//超时处理
		if(Timer2_CNT> TIMER2_10S)
		{
			printf("Error\r\n");
			NVIC_SystemReset();
			break;
		}
	}
	while(strstr((const char*)Usart2_RxBuff,"ready") == NULL);
	delay1ms(500);
	ESP8266_ConnectWiFi();

	ConnectMQTT();
	MQTTPOST();
	Wdt_Feed();

	}
else if((FlagNight==1)&&(TIM2_FlagStart==1)&&(FlagNightPerson==1)&&(FlagFirst==1))//在晚上、正在计数2分钟、触发中断、两分钟的以内直接上报
	{
	Wdt_Feed();
	Bt_M0_Stop(TIM2);
	Timer2_2MinCNT=0;
	Timer2_CNT = 0;
	Bt_M0_Run(TIM2);
	MQTTPOST();
	Wdt_Feed();	
	}			
else if (((FlagNightPerson==0)&&(FlagNight==0))||((FlagNightPerson==0)&&(FlagNight==1)))//白天或者晚上没有人 十秒喂狗一次
	{	
		Wdt_Feed();//喂狗操作	
		printf("Wdt_Feed \r\n");
		delay1ms(6000);
	}
if (TIM2_FlagEND==1)  //两分钟到达
	{
		ESP8266_Sleep();
		TIM2_FlagEND=0;
	}
	ESP8266_Sleep();
	 Lpm_GotoDeepSleep(TRUE);   ///< 进入低功耗模式——深度休眠（使能唤醒后退出中断自动休眠特性）
		
	}	
}
static void App_SysClkInit(void)
{
    stc_sysctrl_clk_cfg_t  stcClkCfg;

    //CLK INIT
    stcClkCfg.enClkSrc  = SysctrlClkRCH;
    stcClkCfg.enHClkDiv = SysctrlHclkDiv1;
    stcClkCfg.enPClkDiv = SysctrlPclkDiv1;
    Sysctrl_ClkInit(&stcClkCfg);

    //使能RCL
    Sysctrl_ClkSourceEnable(SysctrlClkRCL, TRUE);
}

void SystemClkConfig()
{

    //CLK初始化   
    ///< 时钟切换  

    //P24 输出HCLK
   //Gpio_SetFunc_HCLKOUT_P24();
}
void WdtInit()
{
    Sysctrl_SetPeripheralGate(SysctrlPeripheralWdt,TRUE);
    ///< WDT 初始化
    Wdt_Init(WdtResetEn, WdtT52s4);
    
    Wdt_Start();
	
}

static void App_LowPowerModeGpioSet(void)
{
    ///< 打开GPIO外设时钟门控
    Sysctrl_SetPeripheralGate(SysctrlPeripheralGpio, TRUE);

    //swd as gpio
   // Sysctrl_SetFunc(SysctrlSWDUseIOEn, TRUE);

    ///< 配置为数字端口
    M0P_GPIO->PAADS = 0;
    //M0P_GPIO->PBADS = 0;
    M0P_GPIO->PCADS = 0;
    M0P_GPIO->PDADS = 0;

    ///< 配置为端口输入（除LED端口外）
    M0P_GPIO->PADIR = 0XFFFF;
    //M0P_GPIO->PBDIR = 0XFFFF;
    M0P_GPIO->PCDIR = 0XFFFF;
    M0P_GPIO->PDDIR = 0XFFFF;

    ///< 输入下拉（除KEY端口以外）
    M0P_GPIO->PAPD = 0xFFFF;
    //M0P_GPIO->PBPD = 0xFFFF;
    M0P_GPIO->PCPD = 0xFFFF;
    M0P_GPIO->PDPD = 0xFFEF;

}

/******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/


