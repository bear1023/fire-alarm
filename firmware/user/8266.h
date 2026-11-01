#ifndef __8266_H
#define __8266_H

#include "ddl.h"

extern uint8_t FlagOneHour;
extern uint8_t FlagNight;
extern uint8_t FlagDay;
extern uint8_t FlagWhile;
extern volatile uint8_t FlagWiFiConfig;

extern   char WIFISSID[20];
extern   char WIFIPASSWORD[20];
extern   char MQTTCLIENTID[17];

void WiFi_printf(uint8_t* p_Str) ;
void ESP8266_Init(void);
void ESP8266_ConnectWiFi(void);
void ConnectMQTT(void); 
void MQTTPOST(void);
void GetNtpTime(void);
void RTC_Init(void);
void ESP8266_Sleep(void);
int bcd_decimal_code( int bcd);
int decimal_bcd_code(int decimal);
void Memory_Clr(uint8_t* pucBuffer,uint16_t LEN);
char* ESP8266_GetMAC(void);
void WiFiConfig(void); //配网
uint8_t ReadFlashWIFI(void);//从flash中读取wifi信息
#endif
