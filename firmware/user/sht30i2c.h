#ifndef __IIC_H
#define __IIC_H

//#include "stm32f10x.h"
#include <stdbool.h>
#include <stdlib.h>
#include <inttypes.h>
#include <i2c.h>

#define IIC_CLOCK           RCC_APB2Periph_GPIOA
#define IIC_GPIO            GpioPortA
#define IIC_SCL_PIN         GpioPin11
#define IIC_SDA_PIN         GpioPin12

/* 定义I2C总线连接的GPIO端口, 用户只需要修改下面4行代码即可任意改变SCL和SDA的引脚 */
#define I2C_SCL_1()     Gpio_WriteOutputIO(IIC_GPIO,IIC_SCL_PIN,1)//PAout(6)=1
#define I2C_SCL_0()     Gpio_WriteOutputIO(IIC_GPIO,IIC_SCL_PIN,0)//PAout(6)=0
#define I2C_SDA_1()			Gpio_WriteOutputIO(IIC_GPIO,IIC_SDA_PIN,1)//PAout(7)=1
#define I2C_SDA_0()     Gpio_WriteOutputIO(IIC_GPIO,IIC_SDA_PIN,0)//Aout(7)=0
#define I2C_SDA_READ()  Gpio_GetInputIO(IIC_GPIO,IIC_SDA_PIN)

#define I2C_WR	0		/* 写控制bit */
#define I2C_RD	1		/* 读控制bit */
#define I2C_SLAVEADDR      0x44
#define CRC8_POLYNOMIAL 0x31
typedef enum
{
    /* 软件复位命令 */

    SOFT_RESET_CMD = 0x30A2,    
    /*
    单次测量模式
    命名格式：Repeatability_CS_CMD
    CS： Clock stretching
    */
    HIGH_ENABLED_CMD    = 0x2C06,
    MEDIUM_ENABLED_CMD  = 0x2C0D,
    LOW_ENABLED_CMD     = 0x2C10,
    HIGH_DISABLED_CMD   = 0x2400,
    MEDIUM_DISABLED_CMD = 0x240B,
    LOW_DISABLED_CMD    = 0x2416,

    /*
    周期测量模式
    命名格式：Repeatability_MPS_CMD
    MPS：measurement per second
    */
    HIGH_0_5_CMD   = 0x2032,
    MEDIUM_0_5_CMD = 0x2024,
    LOW_0_5_CMD    = 0x202F,
    HIGH_1_CMD     = 0x2130,
    MEDIUM_1_CMD   = 0x2126,
    LOW_1_CMD      = 0x212D,
    HIGH_2_CMD     = 0x2236,
    MEDIUM_2_CMD   = 0x2220,
    LOW_2_CMD      = 0x222B,
    HIGH_4_CMD     = 0x2334,
    MEDIUM_4_CMD   = 0x2322,
    LOW_4_CMD      = 0x2329,
    HIGH_10_CMD    = 0x2737,
    MEDIUM_10_CMD  = 0x2721,
    LOW_10_CMD     = 0x272A,
    /* 周期测量模式读取数据命令 */
    READOUT_FOR_PERIODIC_MODE = 0xE000,
} SHT30_CMD;

void i2c_GPIO_Config(void);

en_result_t I2C_MasterWriteData(M0P_I2C_TypeDef* I2CX,uint8_t u8Addr,uint8_t *pu8Data,uint32_t u32Len);
en_result_t I2C_MasterReadData(M0P_I2C_TypeDef* I2CX,uint8_t u8Addr,uint8_t *pu8Data,uint32_t u32Len);





#endif

