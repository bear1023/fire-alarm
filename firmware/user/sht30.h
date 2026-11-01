#ifndef __SHT30_H
#define __SHT30_H

#include "sht30i2c.h"
//#include "stm32f10x.h"
#define write 0
#define read  1
#define SHT30_ADDR      0x44
#define SHT30_ADDR_READ  (0x44<<1)+1
extern char Temp,Humi;
extern float Temperature;//转换后的温湿度
extern float Humidity;
extern uint8_t humiture_buff1[20];

void SHT30_Init(void);
void IIC_ACK(void);
void IIC_NACK(void);
uint8_t IIC_wait_ACK(void);
void IIC_Start(void);
void IIC_Stop(void);
void IIC_SendByte(uint8_t byte);
uint8_t IIC_RcvByte(void);
void SHT30_read_result(uint8_t addr);
typedef union 
{
    float fdata;
    unsigned char cdata[4];
}float_data;//定义联合体存储float数据，float类型的存储符合IEEE标准，可用于传输数据

void SHT30_Init(void);
void SHT30_SOFTRESET(void);
void SHT30_General_RESET(void);
void SHT30_Stop_Break(void);
void SHT30_reset(void);
void SHT30_Read(void);
void SHT30_SendBytes(uint16_t cmd,uint8_t stop);
void SHT30_ART(void);
void SHT30_Single_Shot(uint8_t *buffer);
void SHT30_Periodic(uint8_t *buffer);
void SHT30_Heater(uint8_t enable);
static uint8_t SHT30_Send_Cmd(SHT30_CMD cmd);
#endif

