//======================================================================
//文件名称：wifi.h（wifi构件头文件）（简化版）
//制作单位：苏大arm技术中心(sumcu.suda.edu.cn)
//更新记录：20190504
//======================================================================
#ifndef _WIFI_H_
#define _WIFI_H_

#include "gec.h"
#include <stdlib.h>
#include <string.h>

//WIFI通信使用的UART模块，根据实际使用自行修改
#define UART_WIFI UART_1

//控制WIFI电源用引脚
#define POWER_PIN PTC_NUM|13

//定义函数返回状态
#define WIFI_OK 0
#define WIFI_ERROR 1
#define WIFI_NORESPONSE 2

//工作模式
#define WIFI_STATION 1
#define WIFI_AP      2
#define WIFI_ST_AP   3


#pragma  pack(push,1)        //取消4字节对齐
typedef struct MAC{
	uint8_t stamac[18];
	uint8_t apmac[18];
}MAC;
//【20230809】WiFi信息结构体
typedef struct WiFiData
{
    uint8_t serverIP[15];       //服务器IP
    uint8_t serverPort[5];      //服务器端口号
    uint8_t serverPort2[5];      //服务器端口号
	uint8_t UE_SSID[32];        //接入点SSID
	uint8_t UE_PSSWD[32];       //接入点密码
	uint8_t IMSI[15];
}WiFiData;
#pragma  pack(pop)           //恢复4字节对齐

//======================================================================
//函数名称：wifi_init
//功能概要：初始化wifi模块
//参数说明：mode_type:决定模块作为客户端还是AP接入点;
//         mode_name:设置模块AP模式下的SSID;
//         pssword:要设置的密码字符串，必须是64位以内ASCII字符;
//         chn:wifi信道号，1~13，只有1、6、11三个信道互不干扰;
//         ip:设置Wi-Fi模块STA模式下的IP地址,设置为0.0.0.0表示开启DHCP
//函数返回：无
//======================================================================
void wifi_init(uint8_t mode_type,uint8_t* mode_name,uint8_t* pssword,uint8_t chn,uint8_t* ip);

//======================================================================
//函数名称：wifi_reset
//功能概要：重启模块
//参数说明：无
//函数返回：无
//======================================================================
void wifi_reset();

//======================================================================
//函数名称：wifi_int_re
//功能概要：中断接收字符
//参数说明：ch:要接受的字符，*dataLen:存储接收到的数据长度，
//      *data:将接收到的数据存放到该数组中;
//函数返回：无
//======================================================================
void wifi_int_re(uint8_t ch,uint16_t *dataLen, uint8_t *data);

//======================================================================
//函数名称：wifi_resolve_domain
//功能概要：连接ap后，进行域名解析
//参数说明：domain:域名,retVal:返回域名解析存放的缓冲区
//函数返回：域名解析是否成功=0成功，=1错误，=2无返回数据
//======================================================================
uint8_t wifi_resolve_domain(uint8_t* domain,uint8_t* retVal);

//======================================================================
//函数名称：wifi_ping
//功能概要：连接ap后，ping指定ip，也可以是域名
//参数说明：address:需要ping的目标地址,time存放ping通返回时间
//函数返回：是否ping通=0成功，=1错误，=2无返回数据
//======================================================================
uint8_t wifi_ping(uint8_t* address,uint8_t* time);

//======================================================================
//函数名称：wifi_get_state
//功能概要：获取模块连接状态
//参数说明：无
//函数返回：=0未连接WIFI和服务器,=1已连接WIFI但未连接服务器,=3已连接WIFI和服务器
//======================================================================
uint8_t wifi_get_state();

//======================================================================
//函数名称：wifi_get_mac
//功能概要：获取本机mac
//参数说明：mac:返回获取的mac值
//函数返回：获取成功返回1，失败返回0
//======================================================================
uint8_t wifi_get_mac(MAC* mac);

//======================================================================
//函数名称：wifi_linktossid
//功能概要：接入指定wifi接入点
//参数说明：ssid:接入点名称,password:密码
//函数返回：接入是否成功=1成功，=0失败
//======================================================================
uint8_t wifi_linktossid(uint8_t* ssid,uint8_t* password);

//======================================================================
//函数名称：wifi_quitssid
//功能概要：断开wifi接入点的连接
//参数说明：无
//函数返回：断开是否成功=0成功，=1错误，=2无返回数据
//======================================================================
uint8_t wifi_quitssid();

//======================================================================
//函数名称：wifi_con_srv
//功能概要：与服务器建立连接
//参数说明：ip:服务器IP地址,port:端口号（均为字符串格式）,
//       格式"xxx.xxx.xxx.xxx",xxx取值范围0-255
//函数返回：=0连接成功,=1连接失败,=2处于发送模式
//======================================================================
uint8_t wifi_con_srv(uint8_t* ip,uint8_t* port);

//======================================================================
//函数名称：wifi_recvN
//功能概要：接收缓冲区中N个字节的数据
//参数说明：data:返回接收数据包
//函数返回：实际接收到的字节数，若无数据返回0
//======================================================================
uint8_t wifi_recvN(uint8_t *data,uint8_t len);

//======================================================================
//函数名称：wifi_sendN
//功能概要：连接建立后，向服务器发送N个字符
//参数说明：data:需要发送的字符,字符串不得超过2048个字,length:数据长度
//函数返回：发送是否成功=0成功，=1错误，=2无返回数据
//======================================================================
uint8_t wifi_sendN(uint8_t* data,uint16_t length,uint8_t* IMSI);

//======================================================================
//函数名称：wifi_set_ip
//功能概要：设置sta模式下WiFi模块的IP地址
//参数说明：ip:要设置的IP地址
//函数返回：=0设置成功，=1设置失败，=2WiFi模块无应答
//======================================================================
uint8_t wifi_set_ip(uint8_t* ip);
//uint8_t wifi_sendN(uint8_t* data,uint16_t length,uint8_t id);

uint8_t wifi_disconnect();
//======================================================================
//函数名称：WiFiMessageInit
//功能概要：初始化WIFI信息
//参数说明：FlashData包含所有关于WIFI的基本信息
//函数返回：无
//======================================================================
void WiFiMessageInit();

#endif
