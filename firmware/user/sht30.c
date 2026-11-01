#include "sht30.h"
#include "uart.h"
#include "ddl.h"
//CRC校验
float humiture[4];
float_data humidity;//存储的湿度数据
float_data temperature;//存储的温度数据
float Temperature;//转换后的温湿度
float Humidity;
char Temp,Humi;
uint8_t status[3];//状态寄存器
uint8_t status_crc;//CRC校验计算
uint8_t crc_check;//crc结果
uint8_t Refresh_SHT30_Data=0;
uint8_t humiture_buff1[20];
uint8_t humiture_buff2[20];

	
void delay1us(uint32_t u32Cnt)
{
    uint32_t u32end;
    
    SysTick->LOAD = 0xFFFFFF;
    SysTick->VAL  = 0;
    SysTick->CTRL = SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_CLKSOURCE_Msk;
    
    while(u32Cnt-- > 0)
    {
        SysTick->VAL = 0;

        u32end = 0x1000000 - SystemCoreClock/1000000;
        while(SysTick->VAL > u32end)
        {
            ;
        }
    }
    
}



//CRC校验
static uint8_t CRC_Check(uint8_t *check_data, uint8_t num, uint8_t check_crc)
{
    uint8_t bit;        // bit mask
    uint8_t crc = 0xFF; // calculated checksum
    uint8_t byteCtr;    // byte counter
    
 // calculates 8-Bit checksum with given polynomial x8+x5+x4+1
    for(byteCtr = 0; byteCtr < num; byteCtr++)
    {
        crc ^= (*(check_data+byteCtr));
        //crc校验，最高位是1就^0x31
        for(bit = 8; bit > 0; --bit)
        {
            if(crc & 0x80)
                crc = (crc << 1) ^ 0x31;
            else
                crc = (crc << 1);
        }
    }
    if(crc==check_crc)
        return 1;
    else 
        return 0;
}

//根据所选获取数据的方式初始化
void SHT30_Init(void)
{
    i2c_GPIO_Config();
    SHT30_General_RESET();//软复位设备
    delay1ms(10);
}

//发送两字节指令，stop确定是否发送停止信号
void SHT30_SendBytes(uint16_t cmd,uint8_t stop)
{
		I2C_SetFunc(M0P_I2C1,I2cStart_En);  // Step5, 发送 Start 信号
		I2C_ClearIrq(M0P_I2C1);
		while(0x08 != I2C_GetState(M0P_I2C1));
		
		I2C_WriteByte(M0P_I2C1,SHT30_ADDR<<1| I2C_WR);  // Step 10-1
		I2C_ClearFunc(M0P_I2C1,I2cStart_En);  // Step 10-2
		I2C_ClearIrq(M0P_I2C1);  // Step 10-3  M0P_I2C->CR_f.SI = 0;
		
			while(0 == I2C_GetIrq(M0P_I2C1) );
		while(0x18 != I2C_GetState(M0P_I2C1));

		I2C_ClearFunc(M0P_I2C1,I2cStart_En);  // Step 10-2
		
		do{
				//I2C_SetFunc(M0P_I2C1,I2cStart_En);
				I2C_WriteByte(M0P_I2C1,cmd>>8);  // Step 13-1
				I2C_ClearIrq(M0P_I2C1);  // Step 13-2    I2Cx_CR.si = 0
				while(0 == I2C_GetIrq(M0P_I2C1) );  // Step 14, 等待 I2Cx_CR.si 变为 1，数据已发送到总线上
		}while(0x28 != I2C_GetState(M0P_I2C1));	
		
		do{
				//I2C_SetFunc(M0P_I2C1,I2cStart_En);
				I2C_WriteByte(M0P_I2C1,cmd&0xFF);  // Step 13-1
				I2C_ClearIrq(M0P_I2C1);  // Step 13-2    I2Cx_CR.si = 0
				while(0 == I2C_GetIrq(M0P_I2C1) );  // Step 14, 等待 I2Cx_CR.si 变为 1，数据已发送到总线上
		}while(0x28 != I2C_GetState(M0P_I2C1));	
		I2C_ClearIrq(M0P_I2C1);  // Step 15-2   I2Cx_CR.si = 0	
		I2C_SetFunc(M0P_I2C1,I2cStop_En);  // Step 15-1  I2Cx_CR.sto = 1
		I2C_ClearIrq(M0P_I2C1);  // Step 15-2   I2Cx_CR.si = 0

		while(0 == I2C_GetIrq(M0P_I2C1));  // Step 16, 等待 I2Cx_CR.si 变为 1，数据已发送到总线上

}

//软复位
void SHT30_SOFTRESET(void)
{
    SHT30_SendBytes(0x30A2,1);
}

//通用复位
void SHT30_General_RESET(void)
{

		I2C_SetFunc(M0P_I2C1,I2cStart_En);  // Step5, 发送 Start 信号
		I2C_ClearIrq(M0P_I2C1);
		while(0 == I2C_GetIrq(M0P_I2C1));
		while(0x08 != I2C_GetState(M0P_I2C1));
		
		I2C_WriteByte(M0P_I2C1,SHT30_ADDR<<1| I2C_WR);  // Step 10-1
		I2C_ClearFunc(M0P_I2C1,I2cStart_En);  // Step 10-2
		I2C_ClearIrq(M0P_I2C1);  // Step 10-3  M0P_I2C->CR_f.SI = 0;
		
			while(0 == I2C_GetIrq(M0P_I2C1) );
		while(0x18 != I2C_GetState(M0P_I2C1));

		I2C_ClearFunc(M0P_I2C1,I2cStart_En);  // Step 10-2
		
		do{				
				I2C_WriteByte(M0P_I2C1,0x00);  // Step 13-1
				I2C_ClearIrq(M0P_I2C1);  // Step 13-2    I2Cx_CR.si = 0
				while(0 == I2C_GetIrq(M0P_I2C1) );  // Step 14, 等待 I2Cx_CR.si 变为 1，数据已发送到总线上
		}while(0x28 != I2C_GetState(M0P_I2C1));	
		
//		do{
//				I2C_SetFunc(M0P_I2C1,I2cStart_En);
//				I2C_WriteByte(M0P_I2C1,0x06);  // Step 13-1
//				I2C_ClearIrq(M0P_I2C1);  // Step 13-2    I2Cx_CR.si = 0
//				while(0 == I2C_GetIrq(M0P_I2C1) );  // Step 14, 等待 I2Cx_CR.si 变为 1，数据已发送到总线上
//		}while(0x28 != I2C_GetState(M0P_I2C1));	
		I2C_ClearIrq(M0P_I2C1);  // Step 15-2   I2Cx_CR.si = 0	
		I2C_SetFunc(M0P_I2C1,I2cStop_En);  // Step 15-1  I2Cx_CR.sto = 1
		I2C_ClearIrq(M0P_I2C1);  // Step 15-2   I2Cx_CR.si = 0

		while(0 == I2C_GetIrq(M0P_I2C1));  // Step 16, 等待 I2Cx_CR.si 变为 1，数据已发送到总线上
		
		I2C_SetFunc(M0P_I2C1,I2cStart_En);  // Step5, 发送 Start 信号
		I2C_ClearIrq(M0P_I2C1);
		while(0 == I2C_GetIrq(M0P_I2C1));
		while(0x08 != I2C_GetState(M0P_I2C1));
		
		I2C_WriteByte(M0P_I2C1,SHT30_ADDR<<1| I2C_WR);  // Step 10-1
		I2C_ClearFunc(M0P_I2C1,I2cStart_En);  // Step 10-2
		I2C_ClearIrq(M0P_I2C1);  // Step 10-3  M0P_I2C->CR_f.SI = 0;
		
			while(0 == I2C_GetIrq(M0P_I2C1) );
		while(0x18 != I2C_GetState(M0P_I2C1));

		I2C_ClearFunc(M0P_I2C1,I2cStart_En);  // Step 10-2
		
//		do{
//				I2C_SetFunc(M0P_I2C1,I2cStart_En);
//				I2C_WriteByte(M0P_I2C1,0x00);  // Step 13-1
//				I2C_ClearIrq(M0P_I2C1);  // Step 13-2    I2Cx_CR.si = 0
//				while(0 == I2C_GetIrq(M0P_I2C1) );  // Step 14, 等待 I2Cx_CR.si 变为 1，数据已发送到总线上
//		}while(0x28 != I2C_GetState(M0P_I2C1));	
		
		do{
				I2C_WriteByte(M0P_I2C1,0x06);  // Step 13-1
				I2C_ClearIrq(M0P_I2C1);  // Step 13-2    I2Cx_CR.si = 0
				while(0 == I2C_GetIrq(M0P_I2C1) );  // Step 14, 等待 I2Cx_CR.si 变为 1，数据已发送到总线上
		}while(0x28 != I2C_GetState(M0P_I2C1));	
		I2C_ClearIrq(M0P_I2C1);  // Step 15-2   I2Cx_CR.si = 0	
		I2C_SetFunc(M0P_I2C1,I2cStop_En);  // Step 15-1  I2Cx_CR.sto = 1
		I2C_ClearIrq(M0P_I2C1);  // Step 15-2   I2Cx_CR.si = 0

		while(0 == I2C_GetIrq(M0P_I2C1));  // Step 16, 等待 I2Cx_CR.si 变为 1，数据已发送到总线上
//    i2c_Start();
//    i2c_SendByte(0x00);
//    i2c_WaitAck();
//    i2c_SendByte(0x06);
//    i2c_WaitAck();
}





//单次读取数据
void SHT30_Single_Shot(uint8_t *buffer)
{
		SHT30_SendBytes(0x2C06,1);
		I2C_MasterReadData(M0P_I2C1,0x44,buffer,6);	
}




//获取SHT3x温湿度
void SHT30_Read(void)
{     
		uint16_t tem,hum;//拼接温湿度数据
    uint8_t buff[6];//获取raw数据   
    uint8_t crcT,crcH;//温度和湿度的CRC校验


    SHT30_Single_Shot(buff);
    //SHT30_Periodic(buff);
    
    tem = ((buff[0]<<8) | buff[1]);//温度拼接
    hum = ((buff[3]<<8) | buff[4]);//湿度拼接

    //计算温度和湿度CRC校验码
    crcT = CRC_Check(buff,2,buff[2]);   //温度
    crcH = CRC_Check(buff+3,2,buff[5]); //湿度
    printf("temperature=%.1f.C\thumidity=%.1f%%RH\r\n",(float)tem,(float)hum);
//		Temperature= (175.0*(float)tem/65535.0-45.0) ;  // T = -45 + 175 * tem / (2^16-1)
//    Humidity= (100.0*(float)hum/65535.0);           // RH = hum*100 / (2^16-1)
//		printf("temperature=%.1f℃\thumidity=%.1f%%RH\r\n",Temperature,Humidity);
    if(crcT&&crcH)//判断CRC校验是否对
    {
        //根据手册计算公式计算
        Temperature= (175.0*(float)tem/65535.0-45.0) ;  // T = -45 + 175 * tem / (2^16-1)
        Humidity= (100.0*(float)hum/65535.0);           // RH = hum*100 / (2^16-1)
        if((Temperature>=-20)&&(Temperature<=125)&&(Humidity>=0)&&(Humidity<=100))//过滤超出测量范围的错误数据
        {
            temperature.fdata=Temperature;
            humidity.fdata=Humidity;
        }
				 printf("temperature=%.1f.C\thumidity=%.1f%%RH\r\n",Temperature,Humidity);
    }
		tem =0;
		hum =0;
}




 

 

