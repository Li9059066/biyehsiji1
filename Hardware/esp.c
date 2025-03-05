/**
 *	Copyright (c) [2025] [小王嵌入式][wang2869902214@outlook.com]
 *	[stm32f103c8t6-esp8266-onenet-mqtt] is licensed under Mulan PSL v2.
 *	You can use this software according to the terms and conditions of the Mulan
 *	PSL v2.
 *	You may obtain a copy of Mulan PSL v2 at:
 *			 http://license.coscl.org.cn/MulanPSL2
 *	THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
 *	KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 *	NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *	See the Mulan PSL v2 for more details.
 */

/*STM32芯片驱动头文件*/
#include "stm32f10x.h" 
#include "stm32f10x_iwdg.h"  // 添加这一行

/*自己的驱动头文件*/
#include "delay.h"
#include "OLED.h"
#include "OELD_Data.h"
#include "FMQ.h"
#include "MQ2.h"
#include "dht11.h"
#include "key.h"
#include "ad.h"
#include "PWM.h"
#include "beep.h"
#include "MyUSART.H"
#include "esp.h"
#include "Servo.h"
#include "MQ7.h"
#include "stepmotor.h"

/*C语言头文件*/
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

/****************************************************************************************************
 *@版权声明：本代码为博主原创代码，遵循 Mulan PSL v2开源协议，转载或使用请附上原代码出处和本声明。 *
 *@CSDN文章：stm32与esp8266连接onenet的mqtt服务器进行数据上报与数据下发————送外卖的CV工程师        *
 *@博客教程链接：https://blog.csdn.net/Wang2869902214/article/details/142501323                    *
 *@哔站B站教程链接：                                                                               *
 *@gitee地址：	 https://gitee.com/Wang2869902214/stm32f103c8t6-esp8266-onenet-mqtt.git             *
 *@更新日期：2025-02-03                                                                            *
 *@版本：V1.0                                                                                      *
 *@QQ群聊：STM32连接onenet交流1群 983362248                                                        *
 *@所需资料全部可以在群里下载，欢迎大家前来探讨~！                                                 *
 *@版权声明：本代码为博主原创代码，遵循 Mulan PSL v2开源协议，转载或使用请附上原代码出处和本声明。 *
 ****************************************************************************************************/

/**
 * @简要  	wifi信息和onenet云平台产品信息
 * @注意	需要根据自己的信息填写，连接2.4GHz频段的无线网络，token根据token生成工具填写规则生成
 */
#define ONENET_MQTT_SET_ENABLE 1                                                                                                         // 使能属性设置功能  	1-开启	0-关闭 	如不需使用属性设置功能，可以不使用cJson减少内存开销，以及加快运行速度
#define WIFI_SSID "207"                                                                                                                 // WIFI用户名
#define WIFI_PASSWORD "12345678"                                                                                                        // WIFI密码
#define ONENET_MQTT_PRODUCT_ID "7ingkW90fd"                                                                                              // OneNET MQTT产品ID
#define ONENET_MQTT_DEVICE_NAME "123"                                                                                                    // OneNET MQTT设备名称
#define ONENET_MQTT_TOKEN "version=2018-10-31&res=products%2F7ingkW90fd&et=1803715797&method=sha1&sign=%2BdmILYSTZUAL2aGPlzmmXFj8Cuo%3D" // 设备token

/**
 * @简要  	连接onenet以及下发数据和上报数据的AT指令集
 * @注意	这里一般无需更改，如果需要其他AT指令参数，可以在下方对应的位置更改参数
 */
#define WIRELESS_WIFI_INFO "AT+CWJAP=\"" WIFI_SSID "\",\"" WIFI_PASSWORD "\"\r\n"                                                                           // AT指令：连接2.4GHz wifi
#define ONENET_MQTT_SERVER_INFO "AT+MQTTCONN=0,\"mqtts.heclouds.com\",1883,1\r\n"                                                                           // AT指令：连接onenet的mqtt服务器
#define ONENET_MQTT_USERCFG_INFO "AT+MQTTUSERCFG=0,1,\"" ONENET_MQTT_DEVICE_NAME "\",\"" ONENET_MQTT_PRODUCT_ID "\",\"" ONENET_MQTT_TOKEN "\",0,0,\"\"\r\n" // AT指令：配置MQTT客户端的用户参数
#define ONENET_MQTT_PUBTOPIC "AT+MQTTPUBRAW=0,\"$sys/" ONENET_MQTT_PRODUCT_ID "/" ONENET_MQTT_DEVICE_NAME "/thing/property/post\""                          // AT指令：设备属性上报请求（发布）
#define ONENET_MQTT_PUB_SET "AT+MQTTPUB=0,\"$sys/" ONENET_MQTT_PRODUCT_ID "/" ONENET_MQTT_DEVICE_NAME "/thing/property/set_reply\""                         // AT指令：设备属性设置响应（发布）
#define ONENET_MQTT_REPLY_TOPIC "AT+MQTTSUB=0,\"$sys/" ONENET_MQTT_PRODUCT_ID "/" ONENET_MQTT_DEVICE_NAME "/thing/property/post/reply\",0\r\n"              // AT指令：设备上报响应请求（订阅）
#if ONENET_MQTT_SET_ENABLE
#include "cJson.h"                                                                                                                    //如果不使用属性设置，可以不用cJson文件代码
#define ONENET_MQTT_SET_TOPIC "AT+MQTTSUB=0,\"$sys/" ONENET_MQTT_PRODUCT_ID "/" ONENET_MQTT_DEVICE_NAME "/thing/property/set\",0\r\n" // AT指令：设备属性设置请求（订阅）
#define ONENET_MQTT_REVEIVE_SET_TOPIC "\"$sys/" ONENET_MQTT_PRODUCT_ID "/" ONENET_MQTT_DEVICE_NAME "/thing/property/set\""            // 属性设置请求接收判断
#define ONENET_MQTT_REVEIVE_REPLY_TOPIC "\"$sys/" ONENET_MQTT_PRODUCT_ID "/" ONENET_MQTT_DEVICE_NAME "/thing/property/post/reply\""   // 属性上报请求接收判断
#endif

/**
 * @简要  	onenet产品的属性结构体
 * @注意	这里需要按照onenet产品中创建的属性值来创建并初始化OneNET_MQTT_Data结构体内容，数据类型和名称要一致
 */
OneNET_MQTT_Data func1 = {"temperature", TYPE_FLOAT, {.float_value = 0.0f}};
OneNET_MQTT_Data func2 = {"humidity", TYPE_FLOAT, {.float_value = 0.0f}};
OneNET_MQTT_Data func3 = {"yan1", TYPE_INT, {.int_value = 0}};
OneNET_MQTT_Data func4 = {"feng", TYPE_INT, {.int_value = 0}};
OneNET_MQTT_Data func5 = {"yan", TYPE_INT, {.int_value = 0}};
OneNET_MQTT_Data func6 = {"yan2", TYPE_INT, {.int_value = 0}};
OneNET_MQTT_Data func7 = {"wen1", TYPE_FLOAT, {.float_value = 0.0f}};
OneNET_MQTT_Data func8 = {"wen2", TYPE_FLOAT, {.float_value = 0.0f}};
OneNET_MQTT_Data func9 = {"shui", TYPE_INT, {.int_value = 0}};
OneNET_MQTT_Data func10 = {"bao", TYPE_INT, {.int_value = 0}};
OneNET_MQTT_Data func11 = {"window", TYPE_INT, {.int_value = 0}};
OneNET_MQTT_Data func12 = {"rana", TYPE_INT, {.int_value = 0}};
OneNET_MQTT_Data func14 = {"ran1", TYPE_INT, {.int_value = 0}};
OneNET_MQTT_Data func15 = {"ran2", TYPE_INT, {.int_value = 0}};
OneNET_MQTT_Data func16 = {"fire", TYPE_INT, {.int_value = 0}};
OneNET_MQTT_Data func17 = {"wena", TYPE_INT, {.int_value = 0}};
OneNET_MQTT_Data func18 = {"ran", TYPE_INT, {.int_value = 0}};
uint8_t auto_fan_state=0;
uint8_t auto_pump_state=0;
/**
 * @简要  	onenet产品的属性值指针数组和数组大小
 * @注意	这里只适用数据上报函数，这里填写需要上报数据的属性结构体指针
 */
OneNET_MQTT_Data *onenet_data_array[] = {
	&func1,
    &func2,
    &func3,  
    &func4,
    &func9, 
    &func11,
	&func12,
	&func16,
	&func17,
	&func18,
	&func5,
};

#define DATA_ARRAY_SIZE (sizeof(onenet_data_array) / sizeof(onenet_data_array[0]))

/**
 * @简要  	替换外部函数名字
 * @注意	这里后面的函数名字需要替换你自己的函数名字
 */
#define wireless_serial_init Serial_Init
#define wireless_delay_ms Delay_ms
#define wireless_send_data Serial_SendString
#define wireless_log_print printf

/**
 * @简要  	无线模块串口接收数组缓存
 * @注意	根据实际需求修改缓存大小
 */
#define RX_BUFFER_SIZE 328                // 数组缓存大小
uint8_t W_RxBuffer[RX_BUFFER_SIZE] = {0}; // 接收缓存数组
uint16_t W_RxDataCnt = 0;                 // 数组下标

/**
 * @简要  	无线模块状态结构体
 * @注意	无
 */
Wireless_TypeDef WirelessStatus = {0, 0};

/**
 * @简要  清除无线模块缓存大小
 * @参数  无
 * @注意	需要在本次接收数据后，不再适用数据后，再清除缓存
 * @返回值 无
 */
void wireless_clear_buffer(void)
{
  memset(W_RxBuffer, 0, sizeof(W_RxBuffer));
  W_RxDataCnt = 0;
}

/**
  * @简要  判断MQTT服务器连接情况
  * @参数  	无
  * @注意	连接或者断开mqtt服务器时，无线模块的串口会收到下面数据
  * @数据示例  	1.mqtt恢复连接：+MQTTCONNECTED:0,1,"mqtts.heclouds.com","1883","",1
        2.mqtt断开连接：+MQTTDISCONNECTED:0
  * @返回值 mqtt服务器连接情况
  */
uint8_t wireless_mqtt_connect_status(void)
{
  if (strstr((char *)W_RxBuffer, "+MQTTDISCONNECTED:0") != NULL)
    return !W_OK;
  if (strstr((char *)W_RxBuffer, "+MQTTCONNECTED:0,1,\"mqtts.heclouds.com\",\"1883\",\"\",1") != NULL)
    return W_OK;
  return 0xFF;
}

/**
  * @简要  判断wifi连接情况
  * @参数  	无
  * @注意	连接或者断开wifi时，无线模块的串口会收到下面数据
  * @数据示例  	1.wifi恢复连接：WIFI GOT IP
        2.wifi断开连接：WIFI DISCONNECT
  * @返回值 wifi连接情况
  */
uint8_t wireless_wifi_connect_status(void)
{
  if (strstr((char *)W_RxBuffer, "WIFI DISCONNECT") != NULL)
    return !W_OK;
  if (strstr((char *)W_RxBuffer, "WIFI GOT IP") != NULL)
    return W_OK;
  return 0xFF;
}

/**
 * @简要  获取无线模块数据接收标志位
 * @参数  	无
 * @注意	无线模块串口接收到具有换行回车的数据后，会设置该标志位
 * @返回值 1-接收 	0-未接收
 */
uint8_t wireless_get_receive_flag(void)
{
  uint8_t flag = WirelessStatus.receiveDataFlag;
  WirelessStatus.receiveDataFlag = 0;
  return flag;
}

/**
  * @简要  	无线模块串口发送AT指令
  * @参数  	char *cmd：要发送的指令字符串
      char *res：期望的返回值字符串结果
      uint8_t sendCount：最大发送次数
      uint8_t clear_buffer 是否清除接收缓存
      uint16_t delay_xms：每次发送AT指令后x毫秒后判断结果
      uint8_t printf_enable:选择是否打印本次指令
  * @注意	根据每个AT指令的响应时间或者具有多个回复的AT指令，去填写适当的delay_xms延时，例如连接wifi的AT指令，需要大约2000ms以及会回复3个AT数据，可以填写2000ms延时等待
  * @返回值 0：不符合预期接收数据
      1：符合预期接收数据
  */
uint8_t wireless_send_command(char *cmd, char *res, uint8_t sendCount, uint8_t clear_buffer, uint16_t delay_xms, uint8_t printf_enable)
{
  if (printf_enable == W_ENABLE)
    wireless_log_print("%s", cmd); // 这里可以打印每次发送的指令
  while (sendCount--)
  {

    wireless_send_data(cmd);                 // AT指令发送
    wireless_delay_ms(delay_xms);            // 适当增加点延迟，等待串口接收完成
    if (wireless_get_receive_flag() == W_OK) // 如果串口接收到换行回车为结尾的数据
    {
#if ONENET_MQTT_SET_ENABLE
      if (wireless_get_onenet_command_flag() == W_OK)
      {
        WirelessStatus.receiveDataFlag = 1;
        return 0;
      } // 这里特殊判断是否有属性设置信息，优先级最高，直接中断当前AT指令
#endif
      if (strstr((const char *)W_RxBuffer, res) != NULL) // 若找到关键字
      {
        if (clear_buffer == W_TRUE)
          wireless_clear_buffer(); // 清除数组
        return 1;                  // 退出，返回0-成功
      }
      wireless_log_print("error recovery data:%s", W_RxBuffer); // 当接收错误时，打印出接收的数据，方便调试
      if (clear_buffer == W_TRUE)
        wireless_clear_buffer();
    }
    wireless_delay_ms(500);
  }
  return 0; // 返回1-失败
}

/**
 * @简要  无线模块初始化
 * @参数  	无
 * @注意	如果初始化效果不理想，或者响应错误，请检查AT指令是否正确，并可以适当加大AT指令的响应时间或者发送次数
 * @返回值 无
 */
void wireless_init(void)
{
  wireless_serial_init(115200); // 无线模块串口初始化
  wireless_log_print("Start MQTT service\r\n");
  // 复位模块，清除所有状态
  if (wireless_send_command("AT+RST\r\n", "", 3, W_TRUE, 500, W_ENABLE) != W_OK)
    WirelessStatus.error_code |= (1 << 0);
  // 关闭AT指令回显，减少进入串口中断的“\r\n”判断
  if (wireless_send_command("ATE0\r\n", "OK", 3, W_TRUE, 50, W_ENABLE) != W_OK)
    WirelessStatus.error_code |= (1 << 1);
  // 设置模式一，配置它作为Wi-Fi客户端，让其可以连接到无线路由器
  if (wireless_send_command("AT+CWMODE=1\r\n", "OK", 3, W_TRUE, 50, W_ENABLE) != W_OK)
    WirelessStatus.error_code |= (1 << 2);
  // 开启DHCP,在连接到Wi-Fi网络时能够自动获得IP地址
  if (wireless_send_command("AT+CWDHCP=1,1\r\n", "OK", 3, W_TRUE, 50, W_ENABLE) != W_OK)
    WirelessStatus.error_code |= (1 << 3);
  // 连接2.4GHz wifi
  if (wireless_send_command(WIRELESS_WIFI_INFO, "GOT IP", 3, W_TRUE, 2000, W_ENABLE) != W_OK)
    WirelessStatus.error_code |= (1 << 4) | ERROR_WiFi_CONNECT;
  // 用于配置MQTT客户端的用户参数
  if (wireless_send_command(ONENET_MQTT_USERCFG_INFO, "OK", 3, W_TRUE, 50, W_ENABLE) != W_OK)
    WirelessStatus.error_code |= (1 << 5);
  // 连接onenet的MQTT服务器
  if (wireless_send_command(ONENET_MQTT_SERVER_INFO, "OK", 3, W_TRUE, 500, W_ENABLE) != W_OK)
    WirelessStatus.error_code |= (1 << 6) | ERROR_MQTT_CONNECT;
  // 订阅设备属性上报响应主题
  if (wireless_send_command(ONENET_MQTT_REPLY_TOPIC, "OK", 3, W_TRUE, 50, W_ENABLE) != W_OK)
    WirelessStatus.error_code |= (1 << 7);
#if ONENET_MQTT_SET_ENABLE
  // 订阅设备属性设置请求
  if (wireless_send_command(ONENET_MQTT_SET_TOPIC, "OK", 3, W_TRUE, 50, W_ENABLE) != W_OK)
    WirelessStatus.error_code |= (1 << 8);
#endif
  if (WirelessStatus.error_code == 0) // 如果所有AT指令都没有错误
  {
    wireless_log_print("\r\n	!!!!!	MQTT service started successfully	!!!!!	\r\n\r\n");
  }
  else // 如果有错误
  {
    wireless_log_print("\r\n	*****	MQTT service failed to start	******	\r\n\r\n");
    wireless_error_handler(WirelessStatus.error_code); // 错误处理函数
  }
}
char esp_Init(void)
{
  // memset(RECS,0,sizeof(RECS));
  wireless_init();
  return 0;
}
/**
 * @简要  无线模块初始化错误处理函数
 * @参数  ErrorCode: 错误码，对应的错误状态
 * @返回值 无
 */
void wireless_error_handler(uint16_t error_code)
{
  wireless_log_print("\r\n	Here are the AT instructions that did not meet expectations		\r\n\r\n");
  if (error_code & (0x01 << 0))
    wireless_log_print("1.AT+RST\r\n\r\n");
  if (error_code & (0x01 << 1))
    wireless_log_print("2.ATE0\r\n\r\n");
  if (error_code & (0x01 << 2))
    wireless_log_print("3.AT+CWMODE=1\r\n\r\n");
  if (error_code & (0x01 << 3))
    wireless_log_print("4.AT+CWDHCP=1,1\r\n\r\n");
  if (error_code & (0x01 << 4))
    wireless_log_print("5.%s\r\n", WIRELESS_WIFI_INFO);
  if (error_code & (0x01 << 5))
    wireless_log_print("6.%s\r\n", ONENET_MQTT_USERCFG_INFO);
  if (error_code & (0x01 << 6))
    wireless_log_print("7.%s\r\n", ONENET_MQTT_SERVER_INFO);
  if (error_code & (0x01 << 7))
    wireless_log_print("8.%s\r\n", ONENET_MQTT_REPLY_TOPIC);
#if ONENET_MQTT_SET_ENABLE
  if (error_code & (0x01 << 8))
    wireless_log_print("9.%s\r\n", ONENET_MQTT_SET_TOPIC);
#endif
  wireless_log_print("\r\n	End		\r\n");
}

/**
 * @简要  无线模块系统事件处理函数
 * @参数  	无
 * @注意	无
 * @返回值 无
 */
void wireless_system_handler(void)
{
  if (WirelessStatus.error_code & ERROR_WiFi_CONNECT)
    wireless_log_print("ERROR_WiFi_CONNECT:	WIFI DISCONNECT\r\n");
  if (WirelessStatus.error_code & ERROR_MQTT_CONNECT)
    wireless_log_print("ERROR_MQTT_CONNECT:	MQTT DISCONNECT\r\n");
  if (WirelessStatus.error_code & ERROR_PUBLISH)
    wireless_log_print("ERROR_PUBLISH\r\n");
}

/**
 * @简要  无线模块onenet云平台数据处理
 * @参数  	无
 * @注意	这里统一修改有关onenet平台的属性值，然后统一上报数据
 * @返回值 无
 */
void wireless_onenet_data_handler(void)
{
	func1.value.float_value = wen1; 
	func2.value.float_value = shi1;
	func5.value.int_value = MQ2_Value;
	func4.value.int_value = feng;  
	func16.value.int_value = fire;
	func3.value.int_value = yan;
	func12.value.int_value = ran;
	func17.value.int_value = tem;
	func9.value.int_value = shui;
	func11.value.int_value = window;
	func18.value.int_value = MQ7_Value;
}

/**
  * @简要  数据上报至onenet云平台
  * @参数  	无
  * @注意	根据OneNET_MQTT_Data创建的属性结构体，并且使用指针数组封装，然后统一处理后上报到onenet云平台
      这里会发送两个AT指令数据，第一个是mqtt长指令上报AT指令，发送后进入长数据模式
      第二个是发送json格式的数据上报至onenet云平台
      需要注意第二个AT指令组合后的长度是否超出数组缓存大小
  * @示例：	第一个AT指令:AT+MQTTPUBRAW=0,"$sys/HbJo2787Fn/device-001/thing/property/post",182,0,0
      第二个AT指令：{"id":"123","params":{"ChuangLian_Control":{"value":false},"JiaShiQi_Control":{"value":false},"sensor_status":{"value":0},"temperature":{"value":13.5},"humidity":{"value":39.0}}}
  * @返回值 无
  */
void wireless_publish_data(void)
{
#define BUFFER_SIZE 512
#define AT_COMMAND_SIZE 256
  uint16_t bufferPos = 0;
  uint16_t atCommandPos = 0;
  char globalBuffer[BUFFER_SIZE];
  char atCommand[AT_COMMAND_SIZE];
  static uint8_t error_send_count = 0;
	//onenet 连接不上
if((WirelessStatus.error_code & ERROR_MQTT_CONNECT) != 0) 
	return;
  bufferPos += snprintf(globalBuffer + bufferPos, BUFFER_SIZE - bufferPos, "{\"id\":\"123\",\"params\":{"); // 拼接JSON数据
  // 遍历指针数组，根据类型拼接数据
  for (unsigned char i = 0; i < DATA_ARRAY_SIZE; ++i)
  {
    switch (onenet_data_array[i]->type)
    {
    case TYPE_BOOL:
      bufferPos += snprintf(globalBuffer + bufferPos, BUFFER_SIZE - bufferPos,
                            "\"%s\":{\"value\":%s}%s",
                            onenet_data_array[i]->name, onenet_data_array[i]->value.bool_value ? "true" : "false",
                            (i < DATA_ARRAY_SIZE - 1) ? "," : "");
      break;
    case TYPE_INT:
      bufferPos += snprintf(globalBuffer + bufferPos, BUFFER_SIZE - bufferPos,
                            "\"%s\":{\"value\":%d}%s",
                            onenet_data_array[i]->name, onenet_data_array[i]->value.int_value,
                            (i < DATA_ARRAY_SIZE - 1) ? "," : "");
      break;
    case TYPE_FLOAT:
      bufferPos += snprintf(globalBuffer + bufferPos, BUFFER_SIZE - bufferPos,
                            "\"%s\":{\"value\":%.1f}%s",
                            onenet_data_array[i]->name, onenet_data_array[i]->value.float_value,
                            (i < DATA_ARRAY_SIZE - 1) ? "," : "");
      break;
    case TYPE_STRING:
      bufferPos += snprintf(globalBuffer + bufferPos, BUFFER_SIZE - bufferPos,
                            "\"%s\":{\"value\":\"%s\"}%s",
                            onenet_data_array[i]->name, onenet_data_array[i]->value.string_value,
                            (i < DATA_ARRAY_SIZE - 1) ? "," : "");
      break;
    }
  }

  // 拼接JSON结尾
  bufferPos += snprintf(globalBuffer + bufferPos, BUFFER_SIZE - bufferPos, "}}\r\n\r\n");

  // 如果上报数据的字符数组大于缓存，打印报错
  if (bufferPos >= BUFFER_SIZE)
  {
    // 处理错误，例如通过日志记录
    printf("\r\nERROR:publish_data buffer overflow\r\n");
    return;
  }

  // 拼接 AT 命令
  atCommandPos += snprintf(atCommand + atCommandPos, AT_COMMAND_SIZE - atCommandPos,
                           "%s,%d,0,0\r\n", ONENET_MQTT_PUBTOPIC, (int)bufferPos);

  // 检查 AT 命令是否溢出
  if (atCommandPos >= AT_COMMAND_SIZE)
  {
    printf("\r\nERROR:atCommand buffer overflow\r\n");
    return;
  }
  wireless_send_command(atCommand, "OK", 1, W_TRUE, 25, W_ENABLE); // 发送 AT 命令进入上报长数据模式
  // 发送数据
  if (wireless_send_command(globalBuffer, "{\"id\":\"123\",\"code\":200,\"msg\":\"success\"}", 1, W_TRUE, 300, W_ENABLE) != W_OK)
    error_send_count++;
  else
    error_send_count = 0, WirelessStatus.error_code &= ~ERROR_PUBLISH;

  if (error_send_count >= 5)
    WirelessStatus.error_code |= ERROR_PUBLISH; // 如果连续失败次数达到 5 次，设置错误代码
}

char Esp_PUB(void)
{
  // 如果错误次数太多，重新初始化

  wireless_system_handler();		//执行无线模块相关事件
  wireless_onenet_data_handler();	//处理有关onenet的数据
  if(WirelessStatus.error_code != 0)
    return 1;
  if((WirelessStatus.error_code & ERROR_MQTT_CONNECT) == 0)  
    {
      wireless_publish_data(); 	//发布数据测试	
      return 2;
    }
  return 0;
}
#if ONENET_MQTT_SET_ENABLE

/**
 * @简要  判断接收的数据是否是属性设置消息
 * @参数  	无
 * @注意	只需要判断订阅消息内容：$sys/HbJo2787Fn/device-001/thing/property/set
 * @数据示例： +MQTTSUBRECV:0,"$sys/HbJo2787Fn/device-001/thing/property/set",63,{"id":"4","version":"1.0","params":{"ChuangLian_Control":true}}
 * @返回值 是否是属性设置消息
 */
uint8_t wireless_get_onenet_command_flag(void)
{
  // 判断如果是属性设置数据包
  if (strstr((char *)W_RxBuffer, ONENET_MQTT_REVEIVE_SET_TOPIC) != NULL)
    return W_OK;

  return !W_OK;
}

/**
 * @简要  属性设置接收信息-提取id号
 * @参数  	char *jsonData：json数据
 * @注意	传入json格式的数据，需要将原始接收的数据分割‘{’前面的内容
 * @参数示例：{"id":"4","version":"1.0","params":{"ChuangLian_Control":true}}
 * @返回值 当前属性设置信息的id号
 */
uint32_t wireless_get_command_id(char *jsonData)
{
  uint32_t cmd_id = 0; // 定义一个id整形变量

  cJSON *json_obj = cJSON_Parse(jsonData); // 判断JSON字符串是否合法
  if (json_obj == NULL)
  {
    const char *error_ptr = cJSON_GetErrorPtr();
    if (error_ptr != NULL)
    {
      wireless_log_print("Error before: %s\n", error_ptr);
    }
    return 0xFFFFFFFF;
  }

  cJSON *id_item = cJSON_GetObjectItem(json_obj, "id"); // 提取id字段
  if (cJSON_IsString(id_item) && (id_item->valuestring != NULL))
  {
    cmd_id = atoi(id_item->valuestring); // 将id字符串转为整形
  }

  cJSON_Delete(json_obj); // 清理JSON对象

  return cmd_id;
}

/**
 * @简要  根据属性设置信息-提取命令内容
 * @参数  	char *jsonData：json数据
 * @注意	需要根据对应的数据类型执行对应的解析函数，cJSON_IsBool(cmd_item)或者cJSON_IsNumbmer(cmd_item)
 * @示例  {"id":"4","version":"1.0","params":{"ChuangLian_Control":true}}
 * @返回值 无
 */
void wireless_receiver_command_extract(char *jsonData)
{
  // 检测字符串是否符合json格式
  cJSON *json_obj = cJSON_Parse(jsonData);
  if (json_obj == NULL)
  {
    const char *error_ptr = cJSON_GetErrorPtr();
    if (error_ptr != NULL)
    {
      wireless_log_print("Error before: %s\r\n", error_ptr);
    }
    return;
  }

  // 提取params字段
  cJSON *params_item = cJSON_GetObjectItem(json_obj, "params");
  if (params_item == NULL)
  {
    wireless_log_print("Params not found\r\n");
    cJSON_Delete(json_obj);
    return;
  }

  cJSON *cmd_item = NULL;

  cmd_item = cJSON_GetObjectItem(params_item, func4.name);
  if (cmd_item != NULL && cJSON_IsNumber(cmd_item))
  {
    func4.value.int_value = (uint8_t)cmd_item->valueint;
    wireless_log_print("func4.value:%u\r\n", func4.value.int_value);
  }

    cmd_item = cJSON_GetObjectItem(params_item, func9.name);
  if (cmd_item != NULL && cJSON_IsNumber(cmd_item))
  {
    func9.value.int_value = (uint8_t)cmd_item->valueint;
    wireless_log_print("func9.value:%u\r\n", func9.value.int_value);
  }


    cmd_item = cJSON_GetObjectItem(params_item, func11.name);
  if (cmd_item != NULL && cJSON_IsNumber(cmd_item))
  {
    func11.value.int_value = (uint8_t)cmd_item->valueint;
    wireless_log_print("func11.value:%u\r\n", func11.value.int_value);
  }
  
      cmd_item = cJSON_GetObjectItem(params_item, func3.name);
  if (cmd_item != NULL && cJSON_IsNumber(cmd_item))
  {
    func3.value.int_value = (uint8_t)cmd_item->valueint;
    wireless_log_print("func3.value:%u\r\n", func3.value.int_value);
  }
  
      cmd_item = cJSON_GetObjectItem(params_item, func17.name);
  if (cmd_item != NULL && cJSON_IsNumber(cmd_item))
  {
    func17.value.int_value = (uint8_t)cmd_item->valueint;
    wireless_log_print("func17.value:%u\r\n", func17.value.int_value);
  }
  
      cmd_item = cJSON_GetObjectItem(params_item, func12.name);
  if (cmd_item != NULL && cJSON_IsNumber(cmd_item))
  {
    func12.value.int_value = (uint8_t)cmd_item->valueint;
    wireless_log_print("func12.value:%u\r\n", func12.value.int_value);
  }
  cJSON_Delete(json_obj); // 释放内存
}

uint8_t Check_Device_Online(void)
{
    // 检查MQTT连接
    wireless_send_data("AT+MQTTCONN?\r\n");
    Delay_ms(100);
    if (WirelessStatus.error_code & ERROR_MQTT_CONNECT)
    {
        return 0;
    }
    
    return 1;
}

/**
  * @简要  接收到属性设置数据后回应
  * @参数  	uint32_t respond_id：回应的id
      uint16_t code：回应的代码，默认：200
      char *msg：回应的字符，默认：success
  * @注意	主要用于APP或者微信小程序中调用API执行HTTP操作时，需要返回执行状态的场景
  * @返回值 无
  */
void wireless_receive_command_respond(uint32_t respond_id, uint16_t code, char *msg)
{
  char jsonData[256] = {0};
  snprintf(jsonData, sizeof(jsonData),
           "%s,\"{\\\"id\\\":\\\"%d\\\"\\,\\\"code\\\":%d\\,\\\"msg\\\":\\\"%s\\\"}\",0,0\r\n",
           ONENET_MQTT_PUB_SET, respond_id, code, msg);
  wireless_send_command(jsonData, "OK", 1, W_TRUE, 50, W_ENABLE);
}

/**
 * @简要  无线模块命令控制
 * @参数  	无
 * @注意	这里会在接收到属性设置信息后执行函数，根据提取后的指令，执行不同的操作，最后需要修改对应的数据值
 * @返回值 无
 */
void wireless_command_control(void)
{
  // 风扇控制 - 查找格式 feng":1 或 feng":0
  if (func4.value.int_value == 1)
	{
		feng = 1;
		fengkai();
		wireless_log_print("fengkai11\r\n");
	}
  else if (func4.value.int_value == 0)
  {
	
	  fengguan();
	  feng = 0;
  }
   

  // 水泵控制 - 查找格式 shui":1 或 shui":0
  if (func9.value.int_value == 1)
	{
		shui = 1;
		shuikai();
		wireless_log_print("shuikai22\r\n");
	}
  else if (func9.value.int_value == 0)
  {
	  shui = 0;
	  shuiguan();
  }
  // 窗户控制 - 查找格式 window":1 或 window":0
  if (func11.value.int_value == 1)
	{
		window = 1;
		Servo_SetAngle(180);
		wireless_log_print("chuanghu kai33\r\n");
	}
  else if (func11.value.int_value == 0)
  {
	   window = 0;
	  Servo_SetAngle(0);
  }
    
  yan = func3.value.int_value;
  ran = func12.value.int_value;
  tem = func17.value.int_value;
  
}

/**
  * @简要  无线模块处理接收到的属性设置数据
  * @参数  	无
  * @注意	将数据转为json格式，然后提取id号和指令，最后响应属性设置
      主要用于APP或者微信小程序中调用API执行HTTP操作时，需要返回执行状态的场景
  * @示例：	原始数据： +MQTTSUBRECV:0,"$sys/HbJo2787Fn/device-001/thing/property/set",63,{"id":"4","version":"1.0","params":{"ChuangLian_Control":true}}
      转为json格式：{"id":"4","version":"1.0","params":{"ChuangLian_Control":true}}
      提取id号转为整形：4
      提取参数params： "ChuangLian_Control":true
  * @返回值 无
  */
void wireless_processing_command(void)
{
  char *jsonData = NULL;
  uint32_t cmd_id;
  jsonData = strstr((const char *)W_RxBuffer, ONENET_MQTT_REVEIVE_SET_TOPIC); // 先截取出属性设置部分的字符串
  if (jsonData == NULL)
    strchr((const char *)W_RxBuffer, '{'); // 提取json数据
  else
    jsonData = strchr((const char *)jsonData, '{');         // 提取json数据
  wireless_log_print("jsonData:\r\n%s\r\n", jsonData);      // 显示接收到的json数据
  cmd_id = wireless_get_command_id(jsonData);               // 获得命令的id号
  wireless_receiver_command_extract(jsonData);              // 提取指令
  wireless_receive_command_respond(cmd_id, 200, "success"); // 回应onenet
}

#endif

/**
 * @简要  无线模块串口接收数据处理函数
 * @参数  	无
 * @注意	在接收到数据后，判断是什么内容，根据对应内容执行对应的操作，最后清除接收数据缓存
 * @返回值 无
 */
void wireless_receive_data_handler(void)
{
  if (wireless_wifi_connect_status() == !W_OK)
    WirelessStatus.error_code |= ERROR_WiFi_CONNECT, wireless_log_print("ERROR_WiFi_CONNECT:	WIFI DISCONNECT\r\n");
  else if (wireless_wifi_connect_status() == W_OK)
    WirelessStatus.error_code &= ~ERROR_WiFi_CONNECT, wireless_log_print("WIFI GOT IP\r\n");

  if (wireless_mqtt_connect_status() == !W_OK)
    WirelessStatus.error_code |= ERROR_MQTT_CONNECT, wireless_log_print("ERROR_MQTT_CONNECT:	MQTT DISCONNECT\r\n");
  else if (wireless_mqtt_connect_status() == W_OK)
    WirelessStatus.error_code &= ~ERROR_MQTT_CONNECT, wireless_log_print("+MQTTCONNECTED:0,1,\"mqtts.heclouds.com\",\"1883\",\"\",1\r\n");
#if ONENET_MQTT_SET_ENABLE
  if (wireless_get_onenet_command_flag() == W_OK) // 判断是命令数据
  {
    wireless_processing_command(); // 处理数据
    wireless_command_control();    // 执行命令
  }
#endif
  wireless_clear_buffer(); // 清除无线模块接收数据缓存
}

/**
 * @简要  无线模块接收函数
 * @参数  	uint8_t byte：接收的字节
 * @注意	放在无线模块串口中断中，参数传入串口接收的字节
 * @返回值 无
 */
void wireless_receive_callback(uint8_t byte)
{
  W_RxBuffer[W_RxDataCnt++] = byte; // 接收数据转存
  if (W_RxDataCnt > RX_BUFFER_SIZE) // 判断接收数据是否溢出
  {
    wireless_clear_buffer();
  }

  if (W_RxDataCnt >= 2 && (W_RxBuffer[W_RxDataCnt - 2] == '\r' && W_RxBuffer[W_RxDataCnt - 1] == '\n')) // 判断结束位
  {
    WirelessStatus.receiveDataFlag = 1;
  }
}

///**
//  * @简要  串口2接收中断服务函数
//  * @参数  	无
//  * @注意	将无线模块接收回调函数放在此中断的接收中
//  * @返回值 无
//  */
// void USART2_IRQHandler(void)
//{
//	uint8_t data = 0;
//	if (USART_GetITStatus(USART2, USART_IT_RXNE) == SET)
//	{
//		data = USART_ReceiveData(USART2);
//		/* 将无线模块接收数据函数放在串口接收中断中，接收数据 */
//		wireless_receive_callback(data);
//
//		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
//	}
//}
