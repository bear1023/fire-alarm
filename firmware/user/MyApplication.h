#ifndef __MYAPPLICATION_H_
#define __MYAPPLICATION_H_

#include "MyUART.h"
#include "timer3.h"
#include "ddl.h"
#include "uart.h"
#include "flash.h"
#include "bt.h"
#include "lpm.h"
#include "rtc.h"
#include "gpio.h"
#include "Timer0.h"
#include "8266.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "lptim.h"
#include "MyGPIO.h"
#include "MyADC.h"
#include "MyFlash.h"
#include "time.h"
#include "wdt.h"
#include "adc.h"
//#include "dht11.h"
#include "sysctrl.h"
#include "lpuart.h"
#include "sht30.h"
#include "bgr.h"
 typedef struct{
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
}bj_time;
extern bj_time BjTime;

#define WIFI_RST_GPIO_Port (GpioPortA)
#define WIFI_RST_Pin			 (GpioPin4)
#define WIFI_EN_GPIO_Port (GpioPortA)
#define WIFI_EN_Pin			 (GpioPin5)
#define  WIFI_SSID  			  (uint8_t*)"YOUR_WIFI_SSID"
#define  WIFI_PASSWORD      (uint8_t*)"YOUR_WIFI_PASSWORD"
//#define UsartPrintf 1
	
#define		Time_WrokStart  0x09 	
#define		Time_WrokEnd		0x06

#define  	MQTT_CLIENTID  (uint8_t*) "YOUR_CLIENT_ID"  //注意：Keil 下 clientid 长度受限于缓冲区
#define 	MQTTADDRESS   (uint8_t*)"YOUR_MQTT_BROKER_IP"
#define 	MQTTUSERNAME  (uint8_t*)"YOUR_MQTT_USERNAME"
#define 	MQTTPASSWORD  (uint8_t*)"YOUR_MQTT_PASSWORD"
#define LptETR_PORT  GpioPortB
#define LptETR_PIN   GpioPin4
#define CAL_PPM  10  // 校准环境中PPM值
#define RL	     10  // RL阻值
#define R0	     26  // R0阻值
#endif

