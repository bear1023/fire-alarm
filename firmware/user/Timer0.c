#include <MyApplication.h>

uint16_t Timer2_CNT;
uint16_t Timer2_LED;
uint16_t Timer2_2MinCNT;
uint8_t TIM2_FlagEND=0;
char FlagFirst;
boolean_t LEDBUFF=TRUE;
void Tim2_IRQHandler(void)
{
    if (TRUE == Bt_GetIntFlag(TIM2,BtUevIrq))
    {
        Bt_ClearIntFlag(TIM2,BtUevIrq);
				Timer2_CNT++;
				Timer2_LED++;
			if(Timer2_LED==100)  //当联网成功后1S闪烁一次
			 {
				Timer2_LED=0;
//				if(LEDBUFF)
//					LEDBUFF=FALSE;
//				else
//					LEDBUFF=TRUE;
//				Gpio_SetIO(0,3);
					if(TIM2_FlagStart==1)				
					{
						Timer2_2MinCNT++;
						if(Timer2_2MinCNT==120)  //两分钟到就睡觉
							{
								TIM2_FlagEND=1;
								FlagFirst=0;
								Timer2_2MinCNT=0;
								TIM2_FlagStart=0;
							}
					}
			 }
		}
}
void LpTim_IRQHandler(void)
{
    if (TRUE == Lptim_GetItStatus(M0P_LPTIMER))
    {
        Wdt_Feed();//printf("Wdt_Feed END\r\n");
        Lptim_ClrItStatus(M0P_LPTIMER);//清除LPTIMER0的中断标志位
    }
}
void App_LPTimerInit(void)
{
    stc_lptim_cfg_t    stcLptCfg;
    DDL_ZERO_STRUCT(stcLptCfg);

    ///< 使能LPTIM0 外设时钟
    Sysctrl_SetPeripheralGate(SysctrlPeripheralLpTim, TRUE);

    stcLptCfg.enGate   = LptimGateLow;
    stcLptCfg.enGatep  = LptimGatePLow;
    stcLptCfg.enTcksel = LptimRcl;
    stcLptCfg.enTogen  = LptimTogEnLow;
    stcLptCfg.enCt     = LptimTimerFun;         //计数器功能
    stcLptCfg.enMd     = LptimMode1;            //工作模式为模式1：无自动重装载16位计数器/定时器
    stcLptCfg.u16Arr   = 0;                     //预装载寄存器值
    Lptim_Init(M0P_LPTIMER, &stcLptCfg);

    Lptim_ClrItStatus(M0P_LPTIMER);        //清除中断标志位
    Lptim_ConfIt(M0P_LPTIMER, TRUE);       //允许LPTIMER中断
    EnableNvic(LPTIM_IRQn, IrqLevel3, TRUE);
		Lptim_Cmd(M0P_LPTIMER, TRUE);   ///< 启动lptimer0运行
}
en_result_t Timer2Init(void)
{
    uint16_t                    u16ArrValue;
    uint16_t                    u16CntValue;
    stc_bt_mode0_cfg_t     stcTim2BaseCfg;
    uint16_t u16Period = 2500;
	   en_result_t       enResult = Error;
    //结构体初始化清零
    DDL_ZERO_STRUCT(stcTim2BaseCfg);
    
    Sysctrl_SetPeripheralGate(SysctrlPeripheralBaseTim, TRUE); //Base Timer外设时钟使能
    
    stcTim2BaseCfg.enWorkMode = BtWorkMode0;              //定时器模式
    stcTim2BaseCfg.enCT       = BtTimer;                  //定时器功能，计数时钟为内部PCLK
    stcTim2BaseCfg.enPRS      = BtPCLKDiv16;              //PCLK/16
    stcTim2BaseCfg.enCntMode  = Bt16bitArrMode;           //自动重载16位计数器/定时器
    stcTim2BaseCfg.bEnTog     = FALSE;
    stcTim2BaseCfg.bEnGate    = FALSE;
    stcTim2BaseCfg.enGateP    = BtGatePositive;
  
    Bt_Mode0_Init(TIM2,&stcTim2BaseCfg);                       //TIM2 的模式0功能初始化
    if (Ok != Bt_Mode0_Init(TIM2, &stcTim2BaseCfg))
    {
        enResult = Error;
    }    
    u16ArrValue = 0x10000 - u16Period ;
    
    Bt_M0_ARRSet(TIM2,u16ArrValue);                            //设置重载值(ARR = 0x10000 - 周期)
    
    u16CntValue = 0x10000 - u16Period;
    
    Bt_M0_Cnt16Set(TIM2,u16CntValue);                          //设置计数初值
    
    Bt_ClearIntFlag(TIM2,BtUevIrq);                          //清中断标志
    Bt_Mode0_EnableIrq(TIM2);                                 //使能TIM2中断(模式0时只有一个中断)
    EnableNvic(TIM2_IRQn, IrqLevel3, TRUE);                 //TIM2 开中断 
		 
		return enResult;
}









