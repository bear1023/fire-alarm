#include "MyApplication.h"
uint8_t TIM2_FlagStart=0;
uint8_t FlagNightPerson=0;
static void App_LowPowerModeGpioSet(void);

void PortB_IRQHandler(void)
{
		uint8_t i;
    if(TRUE == Gpio_GetIrqStatus(GpioPortB, GpioPin4))
    {            
				
        printf("PortB_IRQHandler!!!!!!!!!!!!!!!!!!\r\n");
//        Lptim_ConfIt(M0P_LPTIMER, FALSE); 
        Gpio_ClearIrq(GpioPortB, GpioPin4); 
				NVIC_SystemReset();
    }
		if(TRUE == Gpio_GetIrqStatus(GpioPortB, GpioPin5))
		{
			printf("PortB_IRQHandler!!!!!!!!!!!!!!!!!!\r\n");
			Gpio_ClearIrq(GpioPortB, GpioPin5); 
				for(i=0;i<200;i++)
				{
					delay1ms(10);
					//按键提前释放，退出循环，清除配网标志位
					if(Gpio_GetInputIO(GpioPortB,GpioPin5) == 0)
					{
						FlagWiFiConfig = 0;
						break; //提前跳出for循环
					}
				}
				FlashErase();
				Flash_SectorErase(0xFE00);
				NVIC_SystemReset();
		}
		

} 
static void App_LedInit(void)
{
    stc_gpio_cfg_t stcGpioCfg;
    
    ///< 打开GPIO外设时钟门控
    Sysctrl_SetPeripheralGate(SysctrlPeripheralGpio, TRUE); 
    
    ///< 端口方向配置->输出(其它参数与以上（输入）配置参数一致)
    stcGpioCfg.enDir = GpioDirOut;
    ///< 端口上下拉配置->下拉
    stcGpioCfg.enPu = GpioPuDisable;
    stcGpioCfg.enPd = GpioPdEnable;
    
    ///< LED关闭
    Gpio_ClrIO(GpioPortA, STK_LED_PIN);
    
    ///< GPIO IO LED端口初始化
    Gpio_Init(GpioPortA, STK_LED_PIN, &stcGpioCfg);
    

}
void sleep_GpioInit(void)
{
    stc_gpio_cfg_t stcGpioCfg;
    
    ///< 打开GPIO外设时钟门控
    Sysctrl_SetPeripheralGate(SysctrlPeripheralGpio, TRUE); 
    
    ///< 端口方向配置->输出(其它参数与以上（输入）配置参数一致)
    stcGpioCfg.enDir = GpioDirOut;
    ///< 端口上下拉配置->下拉
    stcGpioCfg.enPu = GpioPuDisable;
    stcGpioCfg.enPd = GpioPdEnable;

		Gpio_Init(GpioPortA, GpioPin15, &stcGpioCfg);
		//Gpio_Init(3, 2, &stcGpioCfg);
		Gpio_SetIO(GpioPortA,GpioPin15);
		//Gpio_ClrIO(WIFI_EN_GPIO_Port,WIFI_EN_Pin);//使能模组
}
void App_UserKeyInit(void)
{
    stc_gpio_cfg_t stcGpioCfg;
    
    ///< 打开GPIO外设时钟门控
    Sysctrl_SetPeripheralGate(SysctrlPeripheralGpio, TRUE);
    
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
    //Gpio_Init(STK_USER_PORT, STK_USER_PIN, &stcGpioCfg); 
	  //Gpio_Init(GpioPortB, GpioPin4, &stcGpioCfg);
		Gpio_Init(GpioPortB, GpioPin5, &stcGpioCfg); 
		//Gpio_EnableIrq(GpioPortB, GpioPin4, GpioIrqRising);   //上升沿触发
		Gpio_EnableIrq(GpioPortB, GpioPin5, GpioIrqRising);   //上升沿触发
		EnableNvic(PORTB_IRQn, IrqLevel3, TRUE);
}


void App_GpioInit(void)
{
    stc_gpio_cfg_t stcGpioCfg;
    
    ///< 打开GPIO外设时钟门控
    Sysctrl_SetPeripheralGate(SysctrlPeripheralGpio, TRUE); 
    
    ///< 端口方向配置->输出(其它参数与以上（输入）配置参数一致)
    stcGpioCfg.enDir = GpioDirOut;
    ///< 端口上下拉配置->下拉
    stcGpioCfg.enPu = GpioPuDisable;
    stcGpioCfg.enPd = GpioPdEnable;
    
    ///< LED关闭
    Gpio_ClrIO(STK_LED_PORT, STK_LED_PIN);
    
    ///< GPIO IO LED端口初始化
    Gpio_Init(STK_LED_PORT, STK_LED_PIN, &stcGpioCfg);
		Gpio_Init(GpioPortA, GpioPin4, &stcGpioCfg);
		Gpio_Init(GpioPortA, GpioPin5, &stcGpioCfg);
		Gpio_Init(GpioPortA, GpioPin15, &stcGpioCfg);

		Gpio_ClrIO(GpioPortA,GpioPin15);
		Gpio_SetIO(GpioPortA,GpioPin4);
		Gpio_SetIO(GpioPortA,GpioPin5);
}

static void App_LowPowerModeGpioSet(void)
{
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

}