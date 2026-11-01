#include "i2c.h"
#include "sht30i2c.h"





/*
*********************************************************************************************************
*	函 数 名: i2c_GPIO_Config
*	功能说明: 配置I2C总线的GPIO，采用模拟IO的方式实现
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void i2c_GPIO_Config(void)
{
    stc_gpio_cfg_t stcGpioCfg;
    
    DDL_ZERO_STRUCT(stcGpioCfg);                 ///< 初始化结构体变量的值为0
    
    Sysctrl_SetPeripheralGate(SysctrlPeripheralGpio,TRUE);  ///< 开启GPIO时钟门控 
        
    stcGpioCfg.enDir = GpioDirOut;               ///< 端口方向配置->输出      
    stcGpioCfg.enOD = GpioOdEnable;              ///< 端口开漏输出配置->开漏输出使能      
    stcGpioCfg.enPu = GpioPuEnable;              ///< 端口上拉配置->使能
    stcGpioCfg.enPd = GpioPdDisable;             ///< 端口下拉配置->禁止
    stcGpioCfg.bOutputVal = TRUE;                ///< 默认高电平
   
    Gpio_Init(GpioPortA, GpioPin11,&stcGpioCfg); ///< 初始化PA11
    Gpio_Init(GpioPortA, GpioPin12,&stcGpioCfg); ///< 初始化PA12
    
    Gpio_SetAfMode(GpioPortA, GpioPin11,GpioAf3);///< 配置PA11为SCL
    Gpio_SetAfMode(GpioPortA, GpioPin12,GpioAf3);///< 配置PA12为SDA     
    stc_i2c_cfg_t stcI2cCfg;
    
    DDL_ZERO_STRUCT(stcI2cCfg);                             ///< 初始化结构体变量的值为0
    
    Sysctrl_SetPeripheralGate(SysctrlPeripheralI2c1,TRUE);  ///< 开启I2C1时钟门控   
        
    stcI2cCfg.u32Pclk = Sysctrl_GetPClkFreq();              ///< 获取PCLK时钟       
    stcI2cCfg.u32Baud = 100000;                             ///< 波特率100kHz
    stcI2cCfg.enMode  = I2cMasterMode;                      ///< I2C主机模式
    stcI2cCfg.u8SlaveAddr = 0x55;                           ///< 从地址，主模式无效
    stcI2cCfg.bGc = FALSE;                                  ///< 广播地址应答使能关闭，主模式无效
    I2C_Init(M0P_I2C1, &stcI2cCfg);                         ///< 模块初始化    
}



en_result_t I2C_MasterReadData(M0P_I2C_TypeDef* I2CX,uint8_t u8Addr,uint8_t *pu8Data,uint32_t u32Len)
{
    en_result_t enRet = Error;
    uint8_t u8i=0,u8State;
    
    I2C_SetFunc(I2CX,I2cStart_En);
    
    while(1)
    {
        while(0 == I2C_GetIrq(I2CX))
        {}
        u8State = I2C_GetState(I2CX);
        switch(u8State)
        {
            case 0x08:                                 ///< 已发送起始条件，将发送SLA+W
                I2C_ClearFunc(I2CX,I2cStart_En);
                I2C_WriteByte(I2CX,I2C_SLAVEADDR<<1| I2C_RD);
								//printf("i2c 0x08\r\n");
                break;
            case 0x18:                                 ///< 已发送SLA+W,并接收到ACK
                //I2C_WriteByte(I2CX,u8Addr);            ///< 发送从机内存地址
								//printf("i2c 0x18\r\n");
                break;
            case 0x28:                                 ///< 已发送数据，接收到ACK, 此处是已发送从机内存地址u8Addr并接收到ACK
                I2C_SetFunc(I2CX,I2cStart_En);         ///< 发送重复起始条件
								//printf("i2c 0x28\r\n");
                break;
						case 0x40:                                 ///< 已发送SLA+R，并接收到ACK
                if(u32Len>1)
                {
                    I2C_SetFunc(I2CX,I2cAck_En);       ///< 使能主机应答功能
									//printf("i2c I2cAck_En\r\n");
                }
                break;
						case 0x50:                                 ///< 已接收数据字节，并已返回ACK信号
                pu8Data[u8i++] = I2C_ReadByte(I2CX);
                if(u8i==u32Len-1)
                {
                    I2C_ClearFunc(I2CX,I2cAck_En);     ///< 已接收到倒数第二个字节，关闭ACK应答功能
									//printf("I2cAck_En 0x50\r\n");
                }
								//printf("i2c 0x50\r\n");
                break;            
            case 0x58:                                 ///< 已接收到最后一个数据，NACK已返回
                pu8Data[u8i++] = I2C_ReadByte(I2CX);
                I2C_SetFunc(I2CX,I2cStop_En);          ///< 发送停止条件
								//printf("i2c 0x58\r\n");
                break;
						case 0x10:                                 ///< 已发送重复起始条件
                I2C_ClearFunc(I2CX,I2cStart_En);
                I2C_WriteByte(I2CX,I2C_SLAVEADDR<<1| I2C_RD);///< 发送SLA+R，开始从从机读取数据
								//printf("i2c 0x10\r\n");
                break;
            case 0x38:                                 ///< 在发送地址或数据时，仲裁丢失
                I2C_SetFunc(I2CX,I2cStart_En);         ///< 当总线空闲时发起起始条件
								//printf("i2c 0x38\r\n");
                break;
            case 0x48:                                 ///< 发送SLA+R后，收到一个NACK
                I2C_SetFunc(I2CX,I2cStop_En);          ///< 发送停止条件
                I2C_SetFunc(I2CX,I2cStart_En);         ///< 发送起始条件
								//printf("i2c 0x48\r\n");
                break;
            default:
                I2C_SetFunc(I2CX,I2cStart_En);         ///< 其他错误状态，重新发送起始条件
                break;
        }
        I2C_ClearIrq(I2CX);                            ///< 清除中断状态标志位
        if(u8i==u32Len)                                ///< 数据全部读取完成，跳出while循环
        {
                break;
        }
    }
    enRet = Ok;
    return enRet;
}
