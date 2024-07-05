#include "includes.h"
void userData_get(UserData *data);    //给用户帧结构体data赋值
extern void ArrayCpy(uint8_t * dest,uint8_t*source,uint16_t len);
extern void main_TimeStamp();
void thread_send()
{
	printf("\r\n 进入线程thread_send!\r\n");
	 //（1）声明线程使用的局部变量
	uint8_t  mSendFlag;                 //发送数据标志位;1:发送;0:不发送
    uint16_t mUserLen;                  //需要发送的字节长度
    uint16_t mFlashLen;                 //需要存储flash的字节长度
    uint64_t mRecvCount;                //收到的次数
    uint8_t mCmd[2];                    //存储命令
    uint8_t mflag;                      //主循环使用的标志判断变量
    uint8_t mWriteFlashFlag;            //表明是否需要将数据写入flash
    uint8_t  mSendData[1000];           //待发送数据缓冲区
    uint16_t mSendLen;                  //待发送数据的长度
    uint16_t  mSendFreq;
    uint16_t  mSendFreqOld;
    uint8_t  mSec;                      //存放运行时长，单位：秒
    uint8_t  mSecOld;                   //存放运行时长，单位：秒
     //（2）给局部变量赋初值
    mSendFlag = 1;                     //默认终端发送数据
    mWriteFlashFlag = 0;                    //默认不写入flash
    mUserLen = sizeof(UserData);      //获得需要发送的字节长度
    mFlashLen = sizeof(FlashData);    //获得存入flash的字节长度
    mRecvCount = 0;                    //清空接收次数
    mSendLen=0;
    mSendFreq=0;
    mSendFreqOld=0;
    mSec=0;
    mSecOld=0;
    while(1)
    {	
		if (gcRecvLen<=0)   goto Send_Procedure_Branch;
		else goto PostBack_Procedure_Branch;
		Send_Procedure_Branch:	
			wdog_feed();	
			mSec=gTimeString[18];                 //取秒ASCII
			if (mSec==mSecOld)	continue;
			mSecOld=mSec;
			printf("\r\n 当前时间为：%s\r\n",gTimeString);
			mSendFreq++;
			gpio_reverse(LIGHT_RED);
			if (mSendFreq-mSendFreqOld>=gUserData.sendFrequencySec)
			{
				mSendFreqOld=mSendFreq;
				printf("\r\n%d 秒时间到，发送一帧数据！\r\n",(int)gUserData.sendFrequencySec);
				mSendFlag = 1;                    //发送标志置1
			}

			if(mSendFlag == 1)
			{
				userData_get(&gUserData);
				//根据命令，获得需要发送的数据
				if(gFlashData.frameCmd[0]=='U'&&gFlashData.frameCmd[1]=='0')
				{
					mSendLen = mUserLen;
					ArrayCpy(gUserData.cmd,gFlashData.frameCmd,2);
					ArrayCpy(mSendData,(uint8_t *)&gUserData,mSendLen);
				}

				mflag = wifi_sendN(mSendData,mSendLen,gUserData.IMSI);
				if(!mflag)
				{
					printf("\r\n WiFi Send Successfully\r\n");
				}
			}
			wdog_feed();	
			goto Send_ClearMark_Procedure_Branch;
		PostBack_Procedure_Branch:
			mRecvCount++;
			mflag = 0xff;
			mSendLen = 0;
			mWriteFlashFlag = 0;
			printf("\r\n收到一帧\n");
			ArrayCpy(mCmd,gcRecvBuf,2);
			ArrayCpy(mSendData,gcRecvBuf,2);

			if(mCmd[0]=='U')
			{
				if(mCmd[1]=='0')		//获取“U0”命令要发送的数据
				{
					ArrayCpy(gFlashData.frameCmd,mCmd,2);
					if(gcRecvLen == mUserLen)       //若为整帧数据
					{
						mSendLen = gcRecvLen;
						mWriteFlashFlag  =1;
						ArrayCpy((uint8_t*)(&gUserData),gcRecvBuf,mUserLen);
						ArrayCpy(gFlashData.equipName,gUserData.equipName,30);
						ArrayCpy(gFlashData.equipID,gUserData.equipID,20);
						ArrayCpy(gFlashData.equipType,gUserData.equipType,20);
						ArrayCpy(gFlashData.vendor,gUserData.vendor,30);
						ArrayCpy(gFlashData.userName,gUserData.userName,20);
						ArrayCpy(gFlashData.phone,gUserData.phone,11);
						ArrayCpy(gFlashData.serverIP,gUserData.serverIP,15);
						ArrayCpy(gFlashData.serverPort,gUserData.serverPort,5);
						gFlashData.sendFrequencySec = gUserData.sendFrequencySec;
						gFlashData.resetCount = gUserData.resetCount;
						ArrayCpy(gFlashData.frameCmd,gUserData.cmd,2);
						//【画瓢处2】-执行操作
					}
				}

			}
			wdog_feed();	
			if (mSendLen>0)                      //若有需要发送的数据
			{
				mflag = wifi_sendN(mSendData,mSendLen,gUserData.IMSI);
			}
			gcRecvLen = 0;                    //接收数据长度清零，表明已经读取
			if(mWriteFlashFlag == 1)         //判断是否需要写flash
			 {
				   flash_read_logic((uint8_t*)gcRecvBuf,(MCU_SECTOR_NUM-2),0,MCU_SECTORSIZE);
				   flash_erase(MCU_SECTOR_NUM-2);
				   ArrayCpy(((uint8_t*)gcRecvBuf+28),(uint8_t*)(&gFlashData),mFlashLen);
				   flash_write((MCU_SECTOR_NUM-2),0,MCU_SECTORSIZE,(uint8_t*)gcRecvBuf);

				mWriteFlashFlag = 0;
			 }
			wdog_feed();	
		    Send_ClearMark_Procedure_Branch:
		    wdog_feed();	
			mSendFlag = 0;                    //修改发送标记
			mSendLen=0;
    }
}

//=====================================================================
//函数名称：userData_get
//函数返回：无
//参数说明：data：需要赋值的结构体数据
//功能概要：给用户帧结构体data赋值
//=====================================================================
void userData_get(UserData *data)             //给用户帧结构体data赋值
{
	wdog_feed();	
    uint16_t mcu_temp_AD;
	float mcu_temp;
    static uint32_t sn = 0;
    data->sn = sn++;
    //获取mcu温度
    mcu_temp_AD = adc_read(AD_MCU_TEMP);       //读取mcu温度通道AD值
    mcu_temp=TempTrans(mcu_temp_AD);           //温度回归
    data->mcuTemp =(int32_t)(TEMP) ;
    main_TimeStamp();                          //给gUserData.currentTime赋值
    //【画瓢处1】-数据获取
    wifi_get_mac(&gMac);
    ArrayCpy(data->IMSI,(uint8_t*)gMac.stamac,15);
}


