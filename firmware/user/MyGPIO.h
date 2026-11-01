#ifndef __MYGPIO_H
#define __MYGPIO_H

#include "ddl.h"
void GPIOInit(void);
void App_GpioInit(void);
void sleep_GpioInit(void);
void App_UserKeyInit(void);
extern uint8_t TIM2_FlagStart;
extern uint8_t FlagNightPerson;

#endif
