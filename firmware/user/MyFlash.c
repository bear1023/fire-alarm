#include "Myapplication.h"

static volatile uint32_t u32FlashTestFlag   = 0;


/*******************************************************************************
 * FLASH 中断服务函数
 ******************************************************************************/
 uint8_t           u32TestBuffer[128] = {0};
 
 void FLASHIint(void)
 {
    if (TRUE == Flash_GetIntFlag(FlashSlockInt))
    {
        Flash_ClearIntFlag(FlashSlockInt);
        u32FlashTestFlag |= 0x01;
        Flash_DisableIrq(FlashSlockInt);
    }    
		if (TRUE == Flash_GetIntFlag(FlashPCInt))
    {
        Flash_ClearIntFlag(FlashPCInt);
        u32FlashTestFlag |= 0x02;
        Flash_DisableIrq(FlashPCInt);
    }
 }

en_result_t FlashWrite(char* str,uint8_t strlong) // 参数指针 填入一个数组 ,  数组长度
{
    en_result_t       enResult = Error;  //枚举一个返回结果
    uint32_t          u32Addr  = 0xFE00;  //写入的地址 最后一页
		uint8_t i;
    	
	  while(Ok != Flash_Init(1, TRUE))
    {
        while(1);
    }
		
    ///< STEP-2 FLASH 操作区解锁(Sector124~127)
    Flash_LockSet(0x80000000);
    
//< ==> 8位编程     
    ///< STEP-3 FLASH目标扇区擦除
    while(Ok != Flash_OpModeConfig(FlashSectorEraseMode));
    if(Ok != Flash_SectorErase(u32Addr))
    {
        while(1);
    }
		
    ///< STEP-4 FLASH 字节写、校验    
    while(Ok != Flash_OpModeConfig(FlashWriteMode));
    if (Ok != Flash_Write8(u32Addr, (uint8_t *)str, strlong))
    {
        while(1);
    }		 		
		    ///< FLASH 读模式
    ///< 注意！！！当不对FLASH进行操作时，应当将FLASH操作状态设置为【读模式】
    while(Ok != Flash_OpModeConfig(FlashReadMode));
    ///< FLASH 全部加锁
    while(Ok != Flash_LockAll());
	
		return enResult;
}
void FlashErase() // 参数指针 填入一个数组 ,  数组长度
{
	  uint32_t           u32Buffer[128] = {0xFF};
    en_result_t       enResult = Error;  //枚举一个返回结果
    uint32_t          u32Addr  = 0xFE00;  //写入的地址 最后一页
		uint8_t i;
    	
	  while(Ok != Flash_Init(1, TRUE))
    {
        while(1);
    }
		
    ///< STEP-2 FLASH 操作区解锁(Sector124~127)
    Flash_LockSet(0x80000000);
    
//< ==> 8位编程     
    ///< STEP-3 FLASH目标扇区擦除
    while(Ok != Flash_OpModeConfig(FlashSectorEraseMode));
    if(Ok != Flash_SectorErase(u32Addr))
    {
        while(1);
    }
		
    ///< STEP-4 FLASH 字节写、校验    
    while(Ok != Flash_OpModeConfig(FlashWriteMode));
    if (Ok != Flash_Write8(u32Addr, (uint8_t *)u32Buffer, 512))
    {
        while(1);
    }		 		
		    ///< FLASH 读模式
    ///< 注意！！！当不对FLASH进行操作时，应当将FLASH操作状态设置为【读模式】
    while(Ok != Flash_OpModeConfig(FlashReadMode));
    ///< FLASH 全部加锁
    while(Ok != Flash_LockAll());
	
}
ReadFlash FlashRead()
{
	ReadFlash data;
	uint32_t u32Addr  = 0xFE00;  //读取的地址 最后一页0x7e00
	Memory_Clr((uint8_t*)data.str,50);	
	data.Strlong=0;
	do
	{
	data.str[data.Strlong]=*((volatile uint8_t*)u32Addr);  //取该地址的内容
	//printf("readflash:str:%s\r\n",data.str);
	data.Strlong++;
	u32Addr++;	
	}
	while(*((volatile uint8_t*)u32Addr)!=0xff);  //如果为ff则说明为空
	printf("readflash:str:%s\r\n",(uint8_t*)data.str);
	printf("readflash:strlong:%d\r\n",data.Strlong);
	return data;
}
