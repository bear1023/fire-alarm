#ifndef __MYADC_H
#define __MYADC_H

#include "ddl.h"
extern float ppm;
void App_AdcPortInit(void);
void App_AdcInit(void);
void App_AdcSglCfg(void);
int filter(void);
float Smog_GetPPM();

#endif
