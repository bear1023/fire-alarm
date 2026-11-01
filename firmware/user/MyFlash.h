#ifndef __MYFLASH_H
#define __MYFLASH_H

#include "ddl.h"

typedef struct
{
    /* data */
    char str[50];
    char  Strlong;
} ReadFlash;

extern ReadFlash data;
void FLASHInit(void);
en_result_t FlashWrite(char* str,uint8_t strlong); // 参数指针 填入一个数组 ,  数组长度
void FlashErase();
ReadFlash FlashRead(void);
#endif
