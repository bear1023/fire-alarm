#include <MyApplication.h>
volatile uint32_t u32AdcRestult;
volatile uint32_t u32AdcRestult1;
volatile uint32_t u32AdcRestultAcc;
float ppm;
void Adc_IRQHandler(void)
{    
    if(TRUE == Adc_GetIrqStatus(AdcMskIrqSqr))
    {
        Adc_ClrIrqStatus(AdcMskIrqSqr);
        
        u32AdcRestult   = Adc_GetSqrResult(AdcSQRCH0MUX);  //获取顺序扫描通道0
        u32AdcRestult1   = Adc_GetSqrResult(AdcSQRCH1MUX);  //获取顺序扫描通道1
        u32AdcRestultAcc = Adc_GetAccResult();
        
        Adc_SQR_Stop();
    }
}

///< ADC 采样端口初始化
void App_AdcPortInit(void)
{    
    ///< 开启 GPIO外设时钟
    Sysctrl_SetPeripheralGate(SysctrlPeripheralGpio, TRUE);
    
    Gpio_SetAnalogMode(GpioPortA, GpioPin1);        //PA01 (AIN0)
		Gpio_SetAnalogMode(GpioPortA, GpioPin0);        //PA00 (AIN1)
}


///< ADC模块 初始化
void App_AdcInit(void)
{
    stc_adc_cfg_t              stcAdcCfg;

    DDL_ZERO_STRUCT(stcAdcCfg);
    
    ///< 开启ADC/BGR外设时钟
    Sysctrl_SetPeripheralGate(SysctrlPeripheralAdcBgr, TRUE); 
    
    Bgr_BgrEnable();        ///< 开启BGR
    
    ///< ADC 初始化配置
    stcAdcCfg.enAdcMode         = AdcScanMode;              ///<采样模式-扫描
    stcAdcCfg.enAdcClkDiv       = AdcMskClkDiv1;            ///<采样分频-1
    stcAdcCfg.enAdcSampCycleSel = AdcMskSampCycle8Clk;      ///<采样周期数-8
    stcAdcCfg.enAdcRefVolSel    = AdcMskRefVolSelAVDD;      ///<参考电压选择-VCC
    stcAdcCfg.enAdcOpBuf        = AdcMskBufDisable;         ///<OP BUF配置-关
    stcAdcCfg.enInRef           = AdcMskInRefDisable;       ///<内部参考电压使能-关
    stcAdcCfg.enAdcAlign        = AdcAlignRight;               ///<转换结果对齐方式-右
    Adc_Init(&stcAdcCfg);
}

///< ADC 单次扫描模式 配置
void App_AdcSglCfg(void)
{
    ///< ADC 采样通道配置
    stc_adc_sqr_cfg_t          stcAdcSqrCfg;
    
    DDL_ZERO_STRUCT(stcAdcSqrCfg);
        
    ///< 顺序扫描模式功能及通道配置
    ///< 注意：扫描模式下，当配置转换次数为n时，转换通道的配置范围必须为[SQRCH(0)MUX,SQRCH(n-1)MUX]
    stcAdcSqrCfg.bSqrDmaTrig = FALSE;
    stcAdcSqrCfg.enResultAcc = AdcResultAccEnable;
    stcAdcSqrCfg.u8SqrCnt    = 2;
    Adc_SqrModeCfg(&stcAdcSqrCfg);

    Adc_CfgSqrChannel(AdcSQRCH0MUX, AdcExInputCH1);
		Adc_CfgSqrChannel(AdcSQRCH1MUX, AdcExInputCH0);

    ///< ADC 中断使能
    Adc_EnableIrq();
    EnableNvic(ADC_IRQn, IrqLevel3, TRUE);
    
    ///< 启动顺序扫描采样
       Adc_SQR_Start();

}

int filter()
{
  uint16_t adcbuff[1];
 char count=0;char i=0;char j=0;
 int Value_buf[5]; //取十个点
	int temp=0;
 int sum=0;
	memset(Value_buf,0,5);     //清空缓冲区
 for(count=0;count<5;count++)
	{
	  Adc_SQR_Start(); 
      //adcbuff[0]= u32AdcRestult;
		Value_buf[count]= u32AdcRestult;
		delay1ms(300);
	}
// for(j=0;j<(5-1);j++)
//  for(i=0;i<(5-j);i++)
//   if(Value_buf[i]>Value_buf[i+1])
//   {
//     temp = Value_buf[i];
//     Value_buf[i]= Value_buf[i+1];
//      Value_buf[i+1]=temp;
//   }
   for(count =0;count<5;count++)
    sum += Value_buf[count];
   return sum/(5);
}
float Smog_Get_Vol(void)
{
	int32_t adc_value = 0;//这是从MQ-7传感器模块电压输出的ADC转换中获得的原始数字值，该值的范围为0到4095，将模拟电压表示为数字值
	float voltage = 0;//MQ-7传感器模块的电压输出，与一氧化碳的浓度成正比
	
	adc_value = u32AdcRestult1;//#define SMOG_ADC_CHX	ADC_Channel_4	定义烟雾传感器所在的ADC通道编号
	delay1ms(5);
	
    voltage  = (3.3/4096.0)*(adc_value);
	
	return voltage;
}
float Smog_GetPPM()
{
	double RS = (3.3f - Smog_Get_Vol()) / Smog_Get_Vol() * RL;
	 ppm = 98.322f * pow(RS/R0, -1.458f);
	return  ppm;
}
