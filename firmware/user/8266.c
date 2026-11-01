#include "MyApplication.h"

bj_time BjTime;
stc_rtc_time_t  stcReadTime;
uint8_t FlagOneHour=0;
uint8_t FlagNight=0;
uint8_t FlagDay=0;
uint8_t FlagWhile=1;
volatile uint8_t FlagWiFiConfig; //默认需要配网

ReadFlash FlashData;	
struct tm *timeinfo;

char WIFISSID[20];
char WIFIPASSWORD[20];
char MQTTCLIENTID[17];

struct tm *timeinfo;


void Rtc_IRQHandler(void)
{

    if(Rtc_GetPridItStatus() == TRUE)
    {
				
        Rtc_ClearPrdfItStatus();             //清除中断标志位
				FlagOneHour=1;
				//printf("rtc中断\r\n");
				Rtc_ReadDateTime(&stcReadTime);
				//printf("日期:%d%d-%d%d-%d%d   时间:%d%d:%d%d:%d%d\r\n",(stcReadTime.u8Year&0xF0)>>4,(stcReadTime.u8Year&0x0F),(stcReadTime.u8Month&0xF0)>>4,(stcReadTime.u8Month&0x0F),(stcReadTime.u8Day&0xF0)>>4,(stcReadTime.u8Day&0x0F),(stcReadTime.u8Hour&0xF0)>>4,(stcReadTime.u8Hour&0x0F),(stcReadTime.u8Minute&0xF0)>>4,(stcReadTime.u8Minute&0x0F),(stcReadTime.u8Second&0xF0)>>4,(stcReadTime.u8Second&0x0F));
				
				
				if((stcReadTime.u8Hour>=Time_WrokStart)||(stcReadTime.u8Hour<Time_WrokEnd))
				{
					FlagNight=1;
					FlagDay=0;
					//printf("FlagDay=0\r\n");	
				}
				else 
				{
					FlagNight=0;
					FlagDay=1;	
					//printf("FlagDay=1\r\n");	
				}
				NVIC_SystemReset();
    }
		
		if(Rtc_GetAlmfItStatus() == TRUE)
		{
			Rtc_ClearAlmfItStatus(); 
			FlagOneHour=1;
			printf("rtc-interruptint\r\n");
				Rtc_ReadDateTime(&stcReadTime);
				printf("date:%d%d-%d%d-%d%d   time:%d%d:%d%d:%d%d\r\n",(stcReadTime.u8Year&0xF0)>>4,(stcReadTime.u8Year&0x0F),(stcReadTime.u8Month&0xF0)>>4,(stcReadTime.u8Month&0x0F),(stcReadTime.u8Day&0xF0)>>4,(stcReadTime.u8Day&0x0F),(stcReadTime.u8Hour&0xF0)>>4,(stcReadTime.u8Hour&0x0F),(stcReadTime.u8Minute&0xF0)>>4,(stcReadTime.u8Minute&0x0F),(stcReadTime.u8Second&0xF0)>>4,(stcReadTime.u8Second&0x0F));
				
				
				if((stcReadTime.u8Hour>=Time_WrokStart)||(stcReadTime.u8Hour<Time_WrokEnd))
				{
					FlagNight=1;
					FlagDay=0;
					printf("FlagDay=0\r\n");	
				}
				else 
				{
					FlagNight=0;
					FlagDay=1;	
					printf("FlagDay=1\r\n");	
				}
		}
}

void WiFi_printf(uint8_t* p_Str) 
{	
	
	uint16_t i;
	uint8_t*  Buff=p_Str;
	uint16_t BuffLength = strlen((const char *)Buff);
	for(i=0;i<BuffLength;i++)
	{
		Uart_ClrStatus(M0P_UART0, UartTC); 
		Uart_SendDataIt(M0P_UART0,*Buff);	
		Buff++;
		Uart_SendDataPoll(M0P_UART1,Uart_ReceiveData(M0P_UART0));
	}
}
void SendAT(uint8_t * AT_Command,uint8_t * Respond_Str)
{
	uint8_t* const  Ptr_AT_Command   = AT_Command;
	uint8_t* const  Ptr_Respond_Str  = Respond_Str;
	
	Usart2_RxCounter=0;                           //WiFi接收数据量变量清零                        
	memset(Usart2_RxBuff,0,USART2_RXBUFF_SIZE);     //清空WiFi接收缓冲区		
	Bt_M0_Stop(TIM2);//等待模块回应数据，超时错误处理
	Timer2_CNT = 0;
	Bt_M0_Run(TIM2);	
	//发送AT指令
	WiFi_printf(Ptr_AT_Command);	
	//打印信息
	#ifdef UsartPrintf
	printf("%s",Ptr_AT_Command);
	#endif
	delay1ms(1000);
	while(strstr((const char*)Usart2_RxBuff,"\r\n") == NULL)
	{
		if(Timer2_CNT> TIMER2_10S)
		{
			//ESP8266.Error();
			printf("SendATError!!!\r\n");
			NVIC_SystemReset();
			break;
		}
	}
	//延时10ms，接收完全部字节
	delay1ms(1000);

	//模块回应数据处理处理
	if(strstr((const char*)Usart2_RxBuff,(const char*)Ptr_Respond_Str) == NULL)
	{
			printf("Error!!!\r\n");
			printf("%s",Usart2_RxBuff);
			#ifdef UsartPrintf
			printf("%s",Usart2_RxBuff);
			#endif
			NVIC_SystemReset();
	}
	printf("SendAT!!!\r\n");
	//打印信息
	printf("%s",Usart2_RxBuff);

}
void ESP8266_Sleep()
{
 SendAT((uint8_t*)"AT+GSLP=10800000\r\n",(uint8_t*)"OK"); //WIFI模块设置为STA模式
}

/*

	* @name   Init
	* @brief  ESP8266初始化
	* @param  None
	* @retval None      
*/

void ESP8266_Init() 
{
	Gpio_SetIO(WIFI_EN_GPIO_Port,WIFI_EN_Pin);//使能模组
	//延时500ms，等待WIFI模块稳定，准备好接收AT指令
	delay1ms(500);
	
	//复位模组
	Gpio_ClrIO(WIFI_RST_GPIO_Port,WIFI_RST_Pin);
	delay1ms(100);
	Gpio_SetIO(WIFI_RST_GPIO_Port,WIFI_RST_Pin);
	delay1ms(500);


	//同步波特率
	Bt_M0_Stop(TIM2);
	Timer2_CNT = 0;
	Bt_M0_Run(TIM2);
	Usart2_RxCounter=0;                           //WiFi接收数据量变量清零                        
	memset(Usart2_RxBuff,0,USART2_RXBUFF_SIZE);     //清空WiFi接收缓冲区 
	
	printf("AT\r\n");
	do
	{	
		//.送AT指令
		
		WiFi_printf((uint8_t*)"AT\r\n");
		
		//延时100ms，等待接收完成
		delay1ms(100);
		//printf("WiFi_printf");
		//打印信息
		#ifdef UsartPrintf
		printf("%s",Usart2_RxBuff);
		#endif
		//超时处理
		if(Timer2_CNT> TIMER2_10S)
		{
			printf("Error\r\n");
			NVIC_SystemReset();
			break;
		}
	}
	while(strstr((const char*)Usart2_RxBuff,"OK") == NULL);
	printf("OKK");
}

void ESP8266_ConnectWiFi()
{
	char buff[50];
	printf("ESP8266_ConnectWiFi ready\r\n");
	SendAT((uint8_t*)"ATE0\r\n",(uint8_t*)"OK");            //关闭回显
	printf("ESP8266_ConnectWiFi OK\r\n");
	SendAT((uint8_t*)"AT+CWMODE=1\r\n",(uint8_t*)"OK"); //WIFI模块设置为STA模式
//	 ESP8266_Init();
	sprintf(buff, "AT+CWJAP=\"%s\",\"%s\"\r\n",WIFISSID,WIFIPASSWORD);
	//printf("%s\r\n",buff);
	Bt_M0_Stop(TIM2);
	Timer2_CNT = 0;
	Bt_M0_Run(TIM2);
	Usart2_RxCounter=0;                           //WiFi接收数据量变量清零                        
	memset(Usart2_RxBuff,0,USART2_RXBUFF_SIZE);     //清空WiFi接收缓冲区
		//发送指令
	WiFi_printf((uint8_t*)buff);
	printf("%s\r\n",buff);
	printf("Error ESP8266_ConnectWiFi\r\n");
	do
	{	

		//延时100ms，等待接收完成
		delay1ms(1000);
		
		//打印信息
		printf("%s",Usart2_RxBuff);
		
		//超时处理
		if(Timer2_CNT> TIMER2_20S)
		{
				NVIC_SystemReset();

			break;
		}
	}
	while(strstr((const char*)Usart2_RxBuff,"WIFI GOT IP") == NULL);
	
}


void ConnectMQTT() 
{
	//char mac[17];
	char Buff[96];
//	char ATend[]="\",0,0,\"\"\r\n";
//	char ATsign[]=",";
	memset(Buff,0,96); 
//	strcat(Buff,(const char*)"AT+MQTTUSERCFG=0,1,\"");
//	strcat(Buff,MQTTCLIENTID);
//	strcat(Buff,"\"");
//	strcat(Buff,ATsign);
//	strcat(Buff,(const char*)MQTTUSERNAME);
//	strcat(Buff,ATsign);
//	strcat(Buff,(const char*)MQTTPASSWORD);
//	strcat(Buff,ATsign);
//	strcat(Buff,(const char*)ATend);
	
	sprintf(Buff, "AT+MQTTUSERCFG=0,1,\"%.17s\",\"%s\",\"%s\",0,0,\"\"\r\n",ESP8266_GetMAC(),MQTTUSERNAME,MQTTPASSWORD);  //UserName
	printf("%s\r\n",Buff);
	SendAT((uint8_t*)Buff,(uint8_t*)"OK"); 
	Memory_Clr((uint8_t*)Buff,100);
	
	//SendAT((uint8_t*)"AT+MQTTCONNCFG=0,130,0,\"123\",\"123\",0,0\r\n",(uint8_t*)"OK");  //设置保活时间
	
	sprintf(Buff,"AT+MQTTCONN=0,\"%s\",1883,0\r\n",MQTTADDRESS);
	Bt_M0_Stop(TIM2);
	Timer2_CNT = 0;
	Bt_M0_Run(TIM2);
	Usart2_RxCounter=0;                           //WiFi接收数据量变量清零                        
	memset(Usart2_RxBuff,0,USART2_RXBUFF_SIZE);     //清空WiFi接收缓冲区
	do
	{
		//发送指令
		WiFi_printf((uint8_t*)Buff);
		//延时100ms，等待接收完成
		delay1ms(2000);
		//打印信息
		printf("%s",Usart2_RxBuff);
		
		//超时处理
		if(Timer2_CNT> TIMER2_20S)
		{
			//ESP8266.Error();
			NVIC_SystemReset();
			break;
		}
	}
	while(strstr((const char*)Usart2_RxBuff,"OK") == NULL);
	
	sprintf(Buff,"AT+MQTTSUB=0,\"/thing/event/property/post\",1\r\n");
	SendAT((uint8_t*)Buff,(uint8_t*)"OK");  //订阅主题
	Memory_Clr((uint8_t*)Buff,100);
}
void MQTTPOST() 
{
	float buff=0.0;
	int intbuff;
	char PersonFlag[1];
	char MQTTPOSTBuff[176];
	char MQTTPUBRAWBuff[72];
	FlagWiFiConfig=0;
	if(Gpio_GetInputIO(GpioPortB,GpioPin4)||(FlagNightPerson==1))
	{
		PersonFlag[0]='1';
		FlagNightPerson=0;
	}
	else PersonFlag[0]='0';
	
	intbuff=  filter();	
	buff=  (float)(intbuff*(float)(3.3/4096));
	printf("ADC:%d,%f\r\n",intbuff,buff);
	
	//printf("Smog_GetPPM=%lf\r\n",Smog_GetPPM());
	buff=((buff*2-3.6)/(0.6))*100;
	if((int)buff>100) 
		buff=0;
	printf("ADC:%d\r\n",(int)buff);
	
	memset(MQTTPOSTBuff,0,176);     //清空WiFi接收缓冲区
	sprintf(MQTTPOSTBuff,"{\"Belong\": \"A\",\
\"NodeName\": \"%.17s\", \
\"Timestamp\": \"1624448722000\", \
\"Temperature\":%.1f,\
\"Humidity\":%.1f,\
\"HumanDetection\":%.1s,\
\"FlameState\": 0, \
\"BatteryPercentage\": %d }\r\n",(const char*)MQTTCLIENTID,Temperature,Humidity,PersonFlag,(int)buff);
	//ESP8266_GetMAC();
	//sprintf(MQTTPOSTBuff,"{\r\n",DeviceName,Temp,Humi,PersonFlag,(char)buff);
	memset(MQTTPUBRAWBuff,0,72);     //清空WiFi接收缓冲区
	sprintf(MQTTPUBRAWBuff,"AT+MQTTPUBRAW=0,\"/sys/thing/event/property/post\",%d,0,0\r\n",strlen(MQTTPOSTBuff));

	Bt_M0_Stop(TIM2);
	Timer2_CNT = 0;
	Bt_M0_Run(TIM2);
	Usart2_RxCounter=0;                           //WiFi接收数据量变量清零                        
	memset(Usart2_RxBuff,0,USART2_RXBUFF_SIZE);     //清空WiFi接收缓冲区

	WiFi_printf((uint8_t*)MQTTPUBRAWBuff);	//发送指令
	
	do
	{
		//延时100ms，等待接收完成
		delay1ms(1000);
		
		//打印信息
		printf("%s",Usart2_RxBuff);
		
		//超时处理
		if(Timer2_CNT> TIMER2_10S)
		{
			//ESP8266.Error();
			NVIC_SystemReset();
			break;
		}
	}
	while(strstr((const char*)Usart2_RxBuff,">") == NULL);
	
	delay1ms(2000);
	Bt_M0_Stop(TIM2);
	Timer2_CNT = 0;
	Bt_M0_Run(TIM2);
	Usart2_RxCounter=0;                           //WiFi接收数据量变量清零                        
	memset(Usart2_RxBuff,0,USART2_RXBUFF_SIZE);     //清空WiFi接收缓冲区
	//printf("%s",MQTTPOSTBuff);
			//发送指令

	//printf("%s\r\n",MQTTPOSTBuff);
	WiFi_printf((uint8_t*)MQTTPOSTBuff);
	delay1ms(1000);
	do
	{
		

		//延时100ms，等待接收完成
		delay1ms(1000);
		
		//打印信息
		//printf("%s",Usart2_RxBuff);
		
		//超时处理
		if(Timer2_CNT> TIMER2_10S)
		{
			//ESP8266.Error();
			NVIC_SystemReset();
			break;
		}
	}
	while(strstr((const char*)Usart2_RxBuff,"MQTTPUB:OK") == NULL);
	delay1ms(1000);
	
}
void GetNtpTime()
{	

	
		time_t timestamp;
		char *buff=NULL;
		int TimeChuo_int=0;
	  char Timechuo[11];
    char GetTimeBuff[96];
    char SubTopic[48]; //订阅Topic用的

	

	  sprintf(SubTopic, "AT+MQTTSUB=0,\"/ntp/%.17s/response\",1\r\n",ESP8266_GetMAC());
		sprintf(GetTimeBuff,"AT+MQTTPUB=0,\"/ntp/request\",\"{\\\"deviceSendTime\\\":\\\"1592361428000\\\"}\",0,0\r\n");
	printf("AT+MQTTSUB=0,\"/ntp/%17s/response\",1\r\n",MQTTCLIENTID);
		printf("%s",SubTopic);
		SendAT((uint8_t*)SubTopic,(uint8_t*)"OK");
		Bt_M0_Stop(TIM2);
		Timer2_CNT = 0;
		Bt_M0_Run(TIM2);
		Usart2_RxCounter=0;                      //WiFi接收数据量变量清零                        
		memset(Usart2_RxBuff,0,USART2_RXBUFF_SIZE);     //清空WiFi接收缓冲区
		printf("%s",GetTimeBuff);
	do
	{
		
		//发送指令
		WiFi_printf((uint8_t*)GetTimeBuff);
		//延时100ms，等待接收完成
		delay1ms(1000);
		
		//打印信息
		printf("%s",Usart2_RxBuff);
		
		//超时处理
		if(Timer2_CNT> TIMER2_10S)
		{
			NVIC_SystemReset();
			//ESP8266.Error();
			break;
		}
	}
	while((buff=strstr((const char*)Usart2_RxBuff,"\"serverSendTime\":")) == NULL);
		buff+=19;
		strncpy(Timechuo,buff,10);
		buff=NULL;
		printf("%.10s\r\n",Timechuo);	
		TimeChuo_int= atoi((const char*)Timechuo);//+1000000000;
		printf("%d\r\n",TimeChuo_int);
		timestamp=TimeChuo_int;
		timestamp += 8 * 60 * 60;
    /* 调用系统函数 */
		timeinfo = localtime(&timestamp);

		BjTime.year = timeinfo->tm_year;  /* 自1900年算起 */
		BjTime.month = timeinfo->tm_mon;  /* 从1月算起，范围0-11 */

		BjTime.day = timeinfo->tm_mday;   /* 日: 1-31 */
		BjTime.hour = timeinfo->tm_hour;  /* 小时:0-23点,UTC+0时间 */
		BjTime.minute = timeinfo->tm_min; /* 分钟:0-59 */
		BjTime.second = timeinfo->tm_sec; /* 0-60，偶尔出现的闰秒 */

			/* 时间校正 */
		BjTime.year += 1900;
		BjTime.month += 1;
	  //printf("日期:%d-%d-%d   时间:%d:%d:%d\r\n",  BjTime.year, BjTime.month, BjTime.day,BjTime.hour, BjTime.minute, BjTime.second);

}
void RTC_Init()
{
    stc_rtc_initstruct_t RtcInitStruct;
    stc_rtc_alarmtime_t RtcAlmStruct;

    DDL_ZERO_STRUCT(RtcInitStruct);                      //变量初始值置零
    DDL_ZERO_STRUCT(RtcAlmStruct);
    
		Sysctrl_ClkSourceEnable(SysctrlClkXTL, TRUE); 
		Sysctrl_SetPeripheralGate(SysctrlPeripheralRtc,TRUE);//RTC模块时钟打开
 
    RtcInitStruct.rtcAmpm = RtcPm;                       //24小时制
    RtcInitStruct.rtcClksrc = RtcClkXtl;                 //外部低速时钟
    RtcInitStruct.rtcPrdsel.rtcPrdsel = RtcPrds;         //周期中断类型PRDS
    RtcInitStruct.rtcPrdsel.rtcPrds = Rtc1H;           //1h周期中断
    RtcInitStruct.rtcTime.u8Second = bcd_decimal_code(BjTime.second);               //配置RTC时间2019年4月17日10:01:55
    RtcInitStruct.rtcTime.u8Minute = bcd_decimal_code(BjTime.minute);
    RtcInitStruct.rtcTime.u8Hour   = bcd_decimal_code(BjTime.hour);
    RtcInitStruct.rtcTime.u8Day    = bcd_decimal_code(BjTime.day);
    RtcInitStruct.rtcTime.u8DayOfWeek = Check_BCD_Format(BjTime.day,1,30);					//不一样
    RtcInitStruct.rtcTime.u8Month  = bcd_decimal_code(BjTime.month);
    RtcInitStruct.rtcTime.u8Year   = bcd_decimal_code(BjTime.year-2000);
    RtcInitStruct.rtcCompen = RtcCompenEnable;           // 使能时钟误差补偿
    RtcInitStruct.rtcCompValue = 0;                      //补偿值  根据实际情况进行补偿
    Rtc_Init(&RtcInitStruct);
    Rtc_AlmIeCmd(FALSE);               
    Rtc_AlmIeCmd(TRUE);                                  //使能闹钟中断
		Rtc_Cmd(FALSE);
    EnableNvic(RTC_IRQn, IrqLevel3, TRUE);               //使能RTC中断向量
    Rtc_Cmd(TRUE);                                       //使能RTC开始计数
		
		Rtc_ReadDateTime(&stcReadTime);
		printf("date:%d%d-%d%d-%d%d   time:%d%d:%d%d:%d%d\r\n",(stcReadTime.u8Year&0xF0)>>4,(stcReadTime.u8Year&0x0F),(stcReadTime.u8Month&0xF0)>>4,(stcReadTime.u8Month&0x0F),(stcReadTime.u8Day&0xF0)>>4,(stcReadTime.u8Day&0x0F),(stcReadTime.u8Hour&0xF0)>>4,(stcReadTime.u8Hour&0x0F),(stcReadTime.u8Minute&0xF0)>>4,(stcReadTime.u8Minute&0x0F),(stcReadTime.u8Second&0xF0)>>4,(stcReadTime.u8Second&0x0F));
	
	if((stcReadTime.u8Hour>=Time_WrokStart)||(stcReadTime.u8Hour<Time_WrokEnd))
	{
		FlagNight=1;
		FlagDay=0;
		printf("FlagDay=0\r\n");	
	}
	else 
	{
		FlagNight=0;
		FlagDay=1;	
		printf("FlagDay=1\r\n");	
	}
	
}

int bcd_decimal_code( int bcd)
{
    return (bcd + (bcd/10) * 6);
}
int decimal_bcd_code(int decimal)
{
    return (decimal - (decimal>>4) * 6);
}
void Memory_Clr(uint8_t* pucBuffer,uint16_t LEN)
{
	uint16_t i;
	
	for(i=0;i<LEN;i++)
	{
		*(pucBuffer+i) = (uint8_t)0;
	}
}
char* ESP8266_GetMAC()
{

	char *pp=NULL;
	uint8_t i;

	//同步波特率
	Bt_M0_Stop(TIM2);
	Timer2_CNT = 0;
	Bt_M0_Run(TIM2);
	Usart2_RxCounter=0;                           //WiFi接收数据量变量清零                        
	memset(Usart2_RxBuff,0,USART2_RXBUFF_SIZE);     //清空WiFi接收缓冲区 
	do
	{	
		//发送AT指令
		WiFi_printf((uint8_t*)"AT+CIPSTAMAC?\r\n");  //发送 Wi-Fi Station 的 MAC 地址。
		//延时100ms，等待接收完成
		delay1ms(1000);
		
		//打印信息
		#ifdef UsartPrintf
		//printf("%s",Usart2_RxBuff);
		#endif
		//超时处理
		if(Timer2_CNT> TIMER2_10S)
		{
			//printf("Error\r\n");
			NVIC_SystemReset();
			break;
		}
	}
	while(strstr((const char*)Usart2_RxBuff,"+CIPSTAMAC:") == NULL);
	pp=strstr((const char*)Usart2_RxBuff,"+CIPSTAMAC:");
	pp+=12;
	memset(MQTTCLIENTID,0,15);  
	for(i=0;i<15;i++)
	{
	MQTTCLIENTID[i]=*pp;
	pp++;
	}
	pp=NULL;
	return MQTTCLIENTID;
}
void WiFiConfig() //配网
{	
	uint8_t num=0;
	uint8_t i=0;
	char *p=NULL;   //匹配字符的指针

	char Buff[196];
if(FlagWiFiConfig)		//连不上网络则需要配网
	{	
	SendAT((uint8_t*)"AT+CWMODE=3\r\n",(uint8_t*)"OK"); 
	SendAT((uint8_t*)"AT+CIPMUX=1\r\n",(uint8_t*)"OK");   //支持多连接
	SendAT((uint8_t*)"AT+CIPSERVERMAXCONN=1\r\n",(uint8_t*)"OK"); 
	SendAT((uint8_t*)"AT+CIPSERVER=1,80\r\n",(uint8_t*)"OK");
	SendAT((uint8_t*)"AT+CIPSTO=20\r\n",(uint8_t*)"OK");		//20s无接受断开
	/*到这里设置了8266为 HTTP服务器端，开始等待客户端POST报文*/	
	Bt_M0_Stop(TIM2);
	Timer2_CNT = 0;
	Bt_M0_Run(TIM2);  //开始定时
	Usart2_RxCounter=0;                           //WiFi接收数据量变量清零                        
	memset(Usart2_RxBuff,0,USART2_RXBUFF_SIZE);     //清空WiFi接收缓冲区 
	do
	{	
		delay1ms(1000);//延时1s，等待接收
		Wdt_Feed();//喂狗操作		
		//超时处理
		if(Timer2_CNT> TIMER2_3min) //定时三分钟，等待接收预检请求报文
		{
			printf("配网失败\r\n");
			NVIC_SystemReset();
			break;
		}
	}
	while(strstr((const char*)Usart2_RxBuff,"OPTIONS") == NULL);
//	1printf("%s\r\n",Usart2_RxBuff);			//打印信息
	/*到这里开始响应预检请求*/
	SendAT((uint8_t*)"AT+CIPSENDEX=0,2048\r\n",(uint8_t*)">");
	Memory_Clr((uint8_t*)Buff,196);	
		sprintf(Buff,"HTTP/1.1 200 OK\r\n\
Content-Length: 0\r\n\
Access-Control-Allow-Origin: *\r\n\
Access-Control-Allow-Headers: Content-Type\r\n\
Access-Control-Allow-Methods: POST, GET, PUT,DELETE, OPTIONS\r\n\
\r\n\
\\0"); 
	/*HTTP/1.1 200 OK：表示响应的版本和状态码，这里是200 OK。
Content-Length: 0：表示响应体的长度为0，即空响应体。
Access-Control-Allow-Origin: *：表示允许所有域进行跨域请求。
Access-Control-Allow-Headers: Content-Type：表示允许请求中包含Content-Type头部字段。
Access-Control-Allow-Methods: POST, GET, PUT, DELETE, OPTIONS：表示允许的请求方法，包括POST、GET、PUT、DELETE和OPTIONS。

通过这样的格式化操作，可以将这些信息填充到指定的字符串中，得到最终的HTTP响应消息。
	chatgpt说的*/
	printf("%s\r\n",Buff);
	Bt_M0_Stop(TIM2);
	Timer2_CNT = 0;
	Bt_M0_Run(TIM2);  //开始定时
	Usart2_RxCounter=0;                           //WiFi接收数据量变量清零                        
	memset(Usart2_RxBuff,0,USART2_RXBUFF_SIZE);     //清空WiFi接收缓冲区 
	WiFi_printf((uint8_t*)Buff);
	do
	{	
		
		delay1ms(1000);//延时1s，等待接收
		Wdt_Feed();//喂狗操作		
		//超时处理
		if(Timer2_CNT> TIMER2_20S) //定时三分钟，等待接收预检请求报文
		{
			printf("配网失败\r\n");
			NVIC_SystemReset();
			break;
		}
	}
	while(strstr((const char*)Usart2_RxBuff,"+IPD") == NULL);
	//printf("%s\r\n",Usart2_RxBuff);			//打印信息
	/*到这里开始解析报文*/
	p=strstr((const char*)Usart2_RxBuff,"\"ssid\":");  //指向"ssid:" 第一个引号的地址
	/*"ssid":"abc","pass":"1234567890"}*/
	Memory_Clr((uint8_t*)WIFISSID,20);	
	Memory_Clr((uint8_t*)WIFIPASSWORD,20);	
	num=0;
	while(num<8) //找到第八个“ 停止  
	{
		if(*p=='"')
				num++;		
		if(num==3)  
		{
			p++;//abc","pass":"1234567890"}
			//1printf("%s\r\n",p);			//打印信息
			while(*p!='"')
			{
				WIFISSID[i]=*p;
				p++;
				i++;
			}	
			i=0;
			//printf("ssid:%s\r\n",WIFISSID);			//打印信息
			num++;  //数到第四个“
		}
		if(num==7)
		{
			p++;
			while(*p!='"')
			{
				WIFIPASSWORD[i]=*p;
				p++;
				i++;
			}
		//	printf("%s\r\n",WIFIPASSWORD);			//打印信息
			num++;
		}
		p++;
	}
	Memory_Clr((uint8_t*)Buff,196);	
	sprintf(Buff,"\"ssid\":\"%s\",\"pass\":\"%s\"",WIFISSID,WIFIPASSWORD); 
	printf("%s\r\n",Buff);			//打印信息
	//printf("%d\r\n",strlen(Buff));			//打印信息
	FlashWrite(Buff,strlen(Buff));
	//FlashRead();	

	ESP8266_ConnectWiFi();
	
	/*响应请求*/
	delay1ms(1000);
	SendAT((uint8_t*)"AT+CIPSENDEX=0,2048\r\n",(uint8_t*)">");
	Memory_Clr((uint8_t*)Buff,196);	
	sprintf(Buff,"HTTP/1.1 200 OK\r\n\
Content-Length:15\r\n\
Access-Control-Allow-Origin: *\r\n\
Access-Control-Allow-Headers: Content-Type\r\n\
Access-Control-Allow-Methods: POST, GET, PUT, DELETE, OPTIONS\r\n\
\r\n\
{\"msg\": \"succ\"}\\0"); 
	//1printf("%s\r\n",Buff);
	WiFi_printf((uint8_t*)Buff);
		do
	{	
		delay1ms(1000);//延时1s，等待接收
		Wdt_Feed();//喂狗操作		
		//超时处理
		if(Timer2_CNT> TIMER2_20S) //定时三分钟，等待接收预检请求报文
		{
			printf("配网失败\r\n");
			NVIC_SystemReset();
			break;
		}
	}
	while(strstr((const char*)Usart2_RxBuff,"SEND OK") == NULL);
	
	SendAT((uint8_t*)"AT+CWMODE=1\r\n",(uint8_t*)"OK"); //WIFI模块设置为STA模式
//	ESP8266_Init();
//	ESP8266_ConnectWiFi();
//	FlagWiFiConfig=0;
		NVIC_SystemReset();
	}			
}
uint8_t ReadFlashWIFI()//从flash中读取wifi信息
{
	uint8_t num=0;
	uint8_t i=0;
	uint8_t j=0;
	char *p;   //匹配字符的指针
	
	FlashData=FlashRead();	
	//1printf("%s,%d\r\n",FlashData.str,FlashData.Strlong);	//ZHAN ZHAN.499.85.net,长度
	Memory_Clr((uint8_t*)WIFISSID,20);	
	Memory_Clr((uint8_t*)WIFIPASSWORD,20);	

	p=FlashData.str;
	num=0;
	i=0;
		while(num<8) //找到第八个“ 停止  
	{
		if(*p=='"')
				num++;	
		else 
		{
			j++;
			if(j>70)
				return 0;				
		}
			
		if(num==3)  
		{
			p++;//abc","pass":"1234567890"}
			//1printf("%s\r\n",p);			//打印信息
			while(*p!='"')
			{
				WIFISSID[i]=*p;
				p++;
				i++;
			}	
			i=0;
			//1printf("ssid:%s\r\n",WIFISSID);			//打印信息
			num++;  //数到第四个“
		}
		if(num==7)
		{
			p++;
			while(*p!='"')
			{
				WIFIPASSWORD[i]=*p;
				p++;
				i++;
			}
			//1printf("%s\r\n",WIFIPASSWORD);			//打印信息
			num++;
		}
		p++;
	}
	
	//printf("ssid:%s\r\n",WIFISSID);			//打印信息
	//printf("password:%s\r\n",WIFIPASSWORD);			//打印信息	
	return 1;
}
