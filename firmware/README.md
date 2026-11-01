# 固件说明

## 目录

| 目录 | 内容 | 是否随仓库提供 |
| --- | --- | --- |
| `source/` | **工程实际编译的** `main.c` + `ddl_device.h` | ✅ |
| `user/` | 外设与业务代码（8266 / ADC / Flash / GPIO / UART / Timer0 / SHT30） | ✅ |
| `mcu/common/` | 芯片启动、中断向量、CMSIS 设备头（华大） | ✅（厂商文件） |
| `MDK/` | Keil 工程（2 个 Target 配置）+ 启动汇编 | ✅ |
| `driver/` | 华大 DDL 驱动库（`ddl.c`/`sysctrl.c`/`gpio.c`/`adc.c`/`uart.c`/`i2c.c`/`rtc.c`/`lpm.c`/`wdt.c`/`flash.c`/`bt.c`/`lptim.c`/`lpuart.c`/`bgr.c`/`reset.c`） | ❌ 需自行安装 HC32L13x 器件包 |
| `MDK/output/` | `.o/.axf/.hex/.map` 构建产物 | ❌ 已剔除 |

## 工程实际编译的 9 个用户 .c（约 2 300 行）

| 文件 | 行数 | 作用 |
| --- | --- | --- |
| `source/main.c` | 348 | 初始化 → 联网/配网 → MQTT/NTP → 开中断 → 深睡主循环 |
| `user/8266.c` | 775 | AT 指令、MQTT 组包、NTP 对时、网页配网、RTC 中断 |
| `user/MyUART.c` | 284 | UART0（ESP）/ UART1（日志）初始化与中断、`fputc` 重定向 |
| `user/MyGPIO.c` | 158 | PORTB 中断（PIR / 配网键）、GPIO 初始化、休眠前 IO 处理 |
| `user/MyADC.C` | 125 | ADC 顺序扫描 + 中断 + 多点平均、MQ 传感器电压/PPM 换算 |
| `user/MyFlash.c` | 116 | 0xFE00 页擦写，Wi-Fi 参数存取 |
| `user/Timer0.c` | 115 | TIM2 10 ms 节拍、2 min 窗口计时、LPTIM 喂狗 |
| `user/sht30.c` | 250 | SHT30 命令表、CRC8（多项式 0x31）、温湿度换算 |
| `user/sht30i2c.c` | 125 | I2C1 初始化与读取状态机（100 kHz，0x44） |

## 关键实现

### 1. 中断驱动的"重启式"事件流

```c
// user/MyGPIO.c
void PortB_IRQHandler(void)
{
    printf("PortB_IRQHandler!!!\r\n");
    if (/* PB4 = PIR */) NVIC_SystemReset();   // 醒来即重启，重新跑完整初始化
    if (/* PB5 = 配网键 */) { FlashErase(); Flash_SectorErase(0xFE00); NVIC_SystemReset(); }
}
```

### 2. AT 指令交互（`SendAT`）

```c
Timer2_CNT = 0;
WiFi_printf(cmd);
while (Timer2_CNT < 1000) {                 // 10 ms 计数 → 10 s 超时
    if (strstr((char*)Usart2_RxBuff, expected)) return TRUE;
}
NVIC_SystemReset();                          // 超时即重启，不重试
```

### 3. 配网（`WiFiConfig`）

ESP8266 开 softAP + `CIPSERVER=1,80`，MCU 手工拼 HTTP 响应（含 CORS 预检），从 `+IPD` 数据里按引号序号抠出 ssid/password，写 Flash 0xFE00，回 `{"msg":"succ"}` 后复位。

### 4. 低功耗

| 项 | 实现 |
| --- | --- |
| 深睡 | `Lpm_GotoDeepSleep(TRUE)`，`while(1)` 每轮末尾调用 |
| 唤醒 | RTC 1 h、PB4 PIR、PB5 按键、LPTIM |
| ESP | `AT+GSLP=10800000`（3 h）+ 硬件 EN 脚断电 |
| 未用 IO | `sleep_GpioInit()` 配高阻输入 |

## 编译前必须补的东西

1. Keil 装 `HDSC.HC32L13x.1.2.0` 器件包；
2. 把华大 DDL 的 `driver/`（`inc` + `src`）放回 `firmware/` 同目录（工程里是 `..\driver\src\xxx.c`）；
3. 按实际板子把 Device 改成 `HC32L110C6PA`，并把 Flash 算法换成 32 KB 版本；
4. 填 `user/MyApplication.h` 里的 Wi-Fi / MQTT 占位符。

## 已知问题

- `user/` 里原本还有一批实验代码（ADPD188BI 的 ADI no-OS 驱动、DHT11、GPS、ADS1115、STM32 软件 I2C），都**没进工程、也不可编译**，整理时已移除，说明见仓库根 README 第 9 节；
- `user/main.c` 是上一代 L110 版本（调用已不存在的 `GPIOInit()`），同样已移除；
- 两个 `.uvprojx` 只差器件包和编译器版本，实际构建用的是 `hc32l130-4.22.uvprojx`。
