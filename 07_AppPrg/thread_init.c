//======================================================================
//文件名称：main.c（应用工程主函数）
//框架提供：SD-Arm（sumcu.suda.edu.cn）
//版本更新：20191108-20200419
//功能描述：见本工程的..\01_Doc\Readme.txt
//移植规则：【固定】
//======================================================================
#define GLOBLE_VAR
#include "includes.h"      //包含总头文件
//----------------------------------------------------------------------
//声明使用到的内部函数
void userData_init(UserData *data);   
void ArrayCpy(uint8_t * dest,uint8_t*source,uint16_t len);
void main_Dly_ms(uint32_t ms);
void main_TimeStamp();
void comm_init();
void consrv_init();
//----------------------------------------------------------------------
//主函数，一般情况下可以认为程序从此开始运行（实际上有启动过程，参见书稿）
void thread_init()
{
    printf("\r\n进入初始化线程thread_init！\r\n");
    //（1）启动部分（开头）======
    //（1.1）声明线程使用的局部变量
   
    //（1.2）【不变】关总中断
    DISABLE_INTERRUPTS;
    //（1.3）给局部变量赋初值
    ArrayCpy(sTimeString,(uint8_t *)"2023-07-18 20:19:00\0",20);
    //（1.4）给全局变量赋初值
    ArrayCpy(gTimeString,(uint8_t *)"2023-07-18 20:19:00\0",20);
    gCount=0;
    gUserData.touchNum=0;
    gUserData.signalPower=0;
    //（1.5）用户外设模块初始化
    gpio_init(LIGHT_RED,GPIO_OUTPUT,LIGHT_ON);    //初始化红灯
    gpio_init(POWER_PIN,GPIO_OUTPUT,0);           //初始化wifi电源
    uart_init(UART_UE,115200);                    //UE串口初始化
    uart_init(UART_2,115200);
    timer_init(TIMER_USER,20);                    //LPTMR计时器初始化为500毫秒
    flash_init();                                 //初始化flash
    adc_init(AD_MCU_TEMP,0);					   //初始化AD芯片温度模块
    printf("\r\n AHL-IoT-GEC start...  \n");
    //【画瓢处1】-初始化
	//（1.6）使能模块中断
	timer_enable_int(TIMER_USER);                //使能LPTMR中断
	uart_enable_re_int(UART_UE);
	//（1.7）【不变】开总中断
	ENABLE_INTERRUPTS;
	//（1.8）【根据实际需要增删】 主循环前的初始化操作
	printf("\r\n flash_erase and write...        \r\n");
	flash_erase(MCU_SECTOR_NUM-2);               //擦除最后一个扇区
	flash_write((MCU_SECTOR_NUM-2),28,sizeof(FlashData),(uint8_t *)flashInit);  //向最后一个扇区写数据
	printf("\r\n flash_erase and write...OK        \r\n");
	//（1.8.1）读取flash中的配置信息至gFlashData,初始化用户帧数据gUserData
	flash_read_logic((uint8_t*)(&gFlashData),(MCU_SECTOR_NUM-2),28,sizeof(FlashData));
	userData_init(&gUserData);                  //初始化用户帧结构体gUserData
	//（1.8.2）判断复位状态，并将复位状态数据存储到flash中（注意不影响原有数据）
	if (IS_POWERON_RESET)                         //冷复位，置零
		gFlashData.resetCount = 0;
	else                                          //热复位，则加1
	{
		gFlashData.resetCount++;
		flash_read_logic((uint8_t*)gcRecvBuf,(MCU_SECTOR_NUM-2),0,MCU_SECTORSIZE);
		flash_erase(MCU_SECTOR_NUM-2);
		ArrayCpy(((uint8_t*)gcRecvBuf+166),(uint8_t*)(gFlashData.resetCount),4);
		flash_write((MCU_SECTOR_NUM-2),0,MCU_SECTORSIZE,(uint8_t*)gcRecvBuf);
	}
	printf("\r\n AHL Init                    \r\n");
	for(;;)                                         //初始化通信模组
	{
		//WIFI连接
		comm_init();
		consrv_init();
		//【20230809】向倒数第三个扇区写入WIFI基本信息
		WiFiMessageInit();
		ArrayCpy(gWiFiData.serverPort2,BIOSPort,5);
		flash_erase(MCU_SECTOR_NUM-3);
		flash_write((MCU_SECTOR_NUM-3),0,sizeof(WiFiData),(uint8_t *)&gWiFiData);  //向最后一个扇区写数据  
		gUserData.startTime=TimeStamp(sTimeString); //给gUserData.startTime赋值	
		break;
	}
	
	printf("\r\n 初始化工作结束...\r\n");
}   


//======以下为主函数调用的子函数===========================================
//=====================================================================
//函数名称：userData_init
//函数返回：无
//参数说明：data：需要初始化的结构体数据
//功能概要：初始化用户帧结构体data
//=====================================================================
void userData_init(UserData *data)            //初始化用户帧结构体
{
    uint8_t mString[10];
    ArrayCpy(data->cmd,(uint8_t *)"U0",2);
    ArrayCpy(gFlashData.frameCmd,data->cmd,2);
    data->sn = 0;
    data->signalPower=0;
    ArrayCpy(data->serverIP,gFlashData.serverIP,15);
    ArrayCpy(data->serverPort,gFlashData.serverPort,5);
    data->currentTime = gFlashData.productTime;
    data->resetCount = gFlashData.resetCount;
    data->sendFrequencySec = gFlashData.sendFrequencySec;
    ArrayCpy(data->userName,gFlashData.userName,20);
    uecom_version(mString);
    ArrayCpy(data->softVer,mString,4);
    ArrayCpy(data->equipName,gFlashData.equipName,30);
    ArrayCpy(data->equipID,gFlashData.equipID,20);
    ArrayCpy(data->equipType,gFlashData.equipType,20);
    ArrayCpy(data->vendor,gFlashData.vendor,30);
    ArrayCpy(data->phone,gFlashData.phone,11);
    data->touchNum = 0;
    data->startTime = gFlashData.productTime;
    ArrayCpy(data->cmd,gFlashData.frameCmd,2);
    //【画瓢处2】-初始化数据

}



//=====================================================================
//函数名称：ArrayCpy
//函数返回：无
//参数说明：dest：复制后存放的数组；source：被复制的数组；len:复制的长度
//功能概要：从源数组复制指定长度的内容到目标数组
//=====================================================================
void ArrayCpy(uint8_t * dest,uint8_t*source,uint16_t len)
{
	for(uint16_t r=0;r<len;r++)  dest[r]=source[r];
}

void main_Dly_ms(uint32_t ms)
{
	for(uint32_t i= 0; i < (3000*ms); i++) wdog_feed();	
}
void  main_TimeStamp()
{
	//年月日时分秒 十进制
	uint16_t tyear,tmonth,tday,thour,tmin,tsec;
	uint16_t i;
	uint64_t timestamp;	    //时间戳
	uint8_t st[20];
	//每月份之前的天数，闰年另计
	uint16_t MonthDay[12]={31,59,90,120,151,181,212,243,273,304,334,365};
	timestamp=0;
	//将年份化为十进制
	for (i=0;i<=3;i++) st[i]=gTimeString[i]-0x30;
	tyear=st[0]*1000+st[1]*100+st[2]*10+st[3];
	//将月份化为十进制
	for (i=5;i<=6;i++) st[i]=gTimeString[i]-0x30;
	tmonth=st[5]*10+st[6];
	//将天化为十进制
	for (i=8;i<=9;i++) st[i]=gTimeString[i]-0x30;
	tday=st[8]*10+st[9];
	//将时化为十进制
	for (i=11;i<=12;i++) st[i]=gTimeString[i]-0x30;
	thour=st[11]*10+st[12];
	//将分化为十进制
	for (i=14;i<=15;i++) st[i]=gTimeString[i]-0x30;
	tmin=st[14]*10+st[15];
	//将秒化为十进制
	for (i=17;i<=18;i++) st[i]=gTimeString[i]-0x30;
	tsec=st[17]*10+st[18];
	//计算之前年份的秒数
	for(i=1970;i<tyear;i++)
	{
		if(i%4)
			timestamp+=31536000;

		else
			timestamp+=31622400;
	}
	//计算之前月份的秒数
	if(tmonth>12) return;
	if(tmonth>2 && tyear%4)
		timestamp+=MonthDay[tmonth-2]*24*3600;
	else if(tmonth>2 && (tyear%4==0))
		timestamp+=(MonthDay[tmonth-2]+1)*24*3600;
	else if(tmonth==2)
		timestamp+=31*24*3600;
	else
		timestamp+=0;
	//计算当前天的秒数
	if(tday>1) timestamp+=(tday-1)*24*3600;
	timestamp+=(thour*3600)+(tmin*60)+tsec;
    gUserData.currentTime=timestamp;

}

void comm_init()
{
	gpio_init(POWER_PIN,GPIO_OUTPUT,1);
	main_Dly_ms(4000);
    wifi_reset();

	for(;;)
	{
		gpio_init(POWER_PIN,GPIO_OUTPUT,1);
		main_Dly_ms(4000);
		if(wifi_quitssid() == WIFI_OK)
		{
			break;
		}
		printf("\r\n WiFi Quitting...\r\n");
		gpio_init(POWER_PIN,GPIO_OUTPUT,0);
    	main_Dly_ms(4000);
	}
	printf("\r\n WiFi Quitted...\r\n");
	main_Dly_ms(4000);
    for(;;)
    {
    	
        if(wifi_linktossid((uint8_t*)"WiFiTest",(uint8_t*)"12345678")==WIFI_OK)
        {
        	break;
        }

        printf("\r\n WiFi Initing...\r\n");
    }
    printf("\r\n WiFi Initted...\r\n");
    main_Dly_ms(4000);
}

void consrv_init()
{
	for(;;)
	{
		if(wifi_disconnect() == WIFI_OK)
		{
			break;
		}
		printf("\r\n WiFi Disconnecting...\r\n");
	}
	printf("\r\n WiFi Disconnectted...\r\n");
	main_Dly_ms(1000);
	for(;;)
	{
		if(wifi_con_srv((uint8_t*)gFlashData.serverIP,(uint8_t*)gFlashData.serverPort)==WIFI_OK)
		{
			break;
		}
		printf("\r\n WiFi Connecting...\r\n");
	}
	printf("\r\n WiFi Connectted...\r\n");
	main_Dly_ms(1000);
}

