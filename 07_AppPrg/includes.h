///=====================================================================
//文件名称：includes.h文件
//制作单位：苏州大学嵌入式实验室(sumcu.suda.edu.cn)
//更新记录：201812-202308
//=====================================================================

#ifndef  INCLUDES_H     //防止重复定义（INCLUDES_H 开头）
#define  INCLUDES_H

//----------------------------------------------------------------------
//（1）包含用到的构件
#include "user.h"
#include "common.h"
#include "NumToStr.h"
#include "valueType.h"
//----------------------------------------------------------------------
//（2）全局使用的宏常数。
//----------------------------------------------------------------------
//（3）自定义数据类型
#pragma  pack(push,1)        //取消4字节对齐
//【不动】记录金葫芦硬件的相关配置信息，保存在flash中的信息
typedef struct FlashData
{
	//①产品信息
	uint8_t equipName[30];      //产品名称
	uint8_t equipID[20];        //产品序列号
	uint8_t equipType[20];      //产品型号
	uint8_t vendor[30];         //生产厂家  100
	uint8_t softVer[4];         //软件版本
	uint64_t productTime;       //生产时间（64位时间戳）
	uint8_t userName[20];       //用户名
	uint8_t phone[11];          //手机号  43
	//②服务器信息
	uint8_t serverIP[15];       //服务器IP
	uint8_t serverPort[5];      //服务器端口号
    //③用户存入flash的信息
	uint32_t sendFrequencySec;  //发送频率
	uint32_t resetCount;        //复位次数
	uint8_t  frameCmd[2];       //命令   30
	//④Wi-Fi接入点信息
	uint8_t UE_SSID[32];        //接入点SSID
	uint8_t UE_PSSWD[32];       //接入点密码
}FlashData;
//【根据实际需要增删】存储用户采集的数据
typedef struct UserData
{
	//【不动】这些变量为必须要有的变量
    uint8_t cmd[2];           //用户命令
	uint16_t sn;              //帧号
	uint8_t IMSI[15];         //IMSI号
	uint8_t serverIP[15];     //服务器IP
	uint8_t serverPort[5];    //服务器端口
	uint64_t currentTime;     //发送时间（64位时间戳）
	uint32_t resetCount;      //复位的次数
	uint32_t sendFrequencySec;//发送时间间隔（秒）
	//【根据实际需要增删】此处可按需要增删变量，注意与服务器端帧结构保持一致
	uint8_t userName[20];     //用户名
	uint8_t softVer[4];       //软件版本
	uint8_t equipName[30];    //产品名称
	uint8_t equipID[20];      //产品序列号
	uint8_t equipType[20];    //产品类型
	uint8_t vendor[30];       //生产厂家
	int32_t mcuTemp;          //芯片温度
	uint8_t surpBaseInfo[20]; //备注1
	uint8_t phone[11];        //手机号
	uint8_t IMEI[15];         //IMEI号
	uint8_t signalPower;      //信号强度
	uint16_t bright;          //光线亮度（光敏传感器A/D值）
	uint16_t touchNum;        //触摸按键次数（TSI）
	uint8_t surplusInfo[20];  //备注2
	uint8_t lbs_location[25];
	//【画瓢处1】-用户自定义添加数据
    uint64_t startTime;     //开始时间（64位时间戳）
	//【画瓢处2】-用户自定义添加数据
}UserData;

#pragma  pack(pop)           //恢复4字节对齐

//----------------------------------------------------------------------

//（4）声明全局变量。命名规范见注2。
//【不动】宏定义全局变量前缀G_VAR_PREFIX。实现原理见注3。
#ifdef GLOBLE_VAR              //GLOBLE_VAR在main.c文件中有宏定义
  #define G_VAR_PREFIX         //前缀G_VAR_PREFIX定义为空
#else                          //GLOBLE_VAR在非main.c文件中无定义
  #define G_VAR_PREFIX extern  //前缀G_VAR_PREFIX定义为"extern"
#endif
//（在此增加全局变量）
//（系统保留）
G_VAR_PREFIX uint8_t  gcRecvBuf[2048];
G_VAR_PREFIX uint16_t gcRecvLen;

G_VAR_PREFIX uint16_t gcUserRecvLen;    //用户/配置数据帧长度
G_VAR_PREFIX uint8_t  gCount;

G_VAR_PREFIX uint16_t gcRecvDCLen;
G_VAR_PREFIX vuint8_t gcReccrc32[4];

G_VAR_PREFIX UserData  gUserData;       //用户信息帧结构体
G_VAR_PREFIX FlashData gFlashData;      //需要写入flash中的数据

G_VAR_PREFIX uint64_t  gTimeSec;        //时间戳
G_VAR_PREFIX uint8_t   gTimeString[20]; //时间"2019-01-01 00:00:21/0"
G_VAR_PREFIX uint8_t   sTimeString[20];

//（用户增加）
G_VAR_PREFIX  uint8_t  gTime[3];        //时间数组
G_VAR_PREFIX struct MAC gMac;
G_VAR_PREFIX struct WiFiData gWiFiData;
//线程函数声明
void app_init(void);
void thread_init();
void thread_send();
//----------------------------------------------------------------------
//【20230809】若要修改BIOS端口，在此处修改
static uint8_t BIOSPort[6] = "23335";
const static FlashData flashInit[]=
{
		{
		//012345678901234567890123456789  产品名称equipName[30]
		"[金葫芦]WiFi终端UE模板   ",
	   //01234567890123456789  产品序列号equipID[20]
		"SD-23-05      ",
	   //01234567890123456789  产品型号equipType[20]
		"AHL-STM32-WiFi-DX-0001",
	   //012345678901234567890123456789    生产厂家vendor[30]
		"SD-ARM 嵌入式人工智能与物联网 ",
	   //0123  软件版本softVer[4]
		"0.40",
		1683614384,                 //生产时间的时间戳2019-11-10
	   //01234567890123456789  用户名userName[20]
		"WiFi",
	   //01234567890  手机号phone[11]
		"13000000000",
	   //②服务器信息
	   //012345678901234  服务器IPserverIP[15]
	   "192.168.137.1",           //CS-Monitor使用
	  	//01234 服务器端口号serverPort[5]
		"32225",                   //CS-Monitor监听的端口号
		//③用户存入flash的信息
		30,                          //发送频率
		0,                          //复位次数
		"U0",                       //命令
		"WiFiTest",                 // WiFi名称（SSID）
		"12345678"                  // WiFi密码
		}
};
#endif                  //防止重复定义（INCLUDES_H 结尾）


/*
 知识要素：
 注1：全局使用的宏常数，使用全大写
 注2：全局变量命名一律以g开头，尽可能保证全工程内唯一性，并且一定要注明
      其含义，（声明时不准赋值）
 注3：全局变量一处声明多处使用处理方法代码段是为了解决全局变量重复声明
 的问题，宏GLOBLE_VAR仅在main.c中声明，所以只有在main.c中包含
 本头文件时才会声明全局变量，在其他文件中包含本文件时，编译时，就会自动加
 上前缀extern，表明是main函数中声明的全局变量，该文件中直接使用。
 */


