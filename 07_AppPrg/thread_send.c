#include "includes.h"
void userData_get(UserData *data);    //���û�֡�ṹ��data��ֵ
extern void ArrayCpy(uint8_t * dest,uint8_t*source,uint16_t len);
extern void main_TimeStamp();
void thread_send()
{
	printf("\r\n �����߳�thread_send!\r\n");
	 //��1�������߳�ʹ�õľֲ�����
	uint8_t  mSendFlag;                 //�������ݱ�־λ;1:����;0:������
    uint16_t mUserLen;                  //��Ҫ���͵��ֽڳ���
    uint16_t mFlashLen;                 //��Ҫ�洢flash���ֽڳ���
    uint64_t mRecvCount;                //�յ��Ĵ���
    uint8_t mCmd[2];                    //�洢����
    uint8_t mflag;                      //��ѭ��ʹ�õı�־�жϱ���
    uint8_t mWriteFlashFlag;            //�����Ƿ���Ҫ������д��flash
    uint8_t  mSendData[1000];           //���������ݻ�����
    uint16_t mSendLen;                  //���������ݵĳ���
    uint16_t  mSendFreq;
    uint16_t  mSendFreqOld;
    uint8_t  mSec;                      //�������ʱ������λ����
    uint8_t  mSecOld;                   //�������ʱ������λ����
     //��2�����ֲ���������ֵ
    mSendFlag = 1;                     //Ĭ���ն˷�������
    mWriteFlashFlag = 0;                    //Ĭ�ϲ�д��flash
    mUserLen = sizeof(UserData);      //�����Ҫ���͵��ֽڳ���
    mFlashLen = sizeof(FlashData);    //��ô���flash���ֽڳ���
    mRecvCount = 0;                    //��ս��մ���
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
			mSec=gTimeString[18];                 //ȡ��ASCII
			if (mSec==mSecOld)	continue;
			mSecOld=mSec;
			printf("\r\n ��ǰʱ��Ϊ��%s\r\n",gTimeString);
			mSendFreq++;
			gpio_reverse(LIGHT_RED);
			if (mSendFreq-mSendFreqOld>=gUserData.sendFrequencySec)
			{
				mSendFreqOld=mSendFreq;
				printf("\r\n%d ��ʱ�䵽������һ֡���ݣ�\r\n",(int)gUserData.sendFrequencySec);
				mSendFlag = 1;                    //���ͱ�־��1
			}

			if(mSendFlag == 1)
			{
				userData_get(&gUserData);
				//������������Ҫ���͵�����
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
			printf("\r\n�յ�һ֡\n");
			ArrayCpy(mCmd,gcRecvBuf,2);
			ArrayCpy(mSendData,gcRecvBuf,2);

			if(mCmd[0]=='U')
			{
				if(mCmd[1]=='0')		//��ȡ��U0������Ҫ���͵�����
				{
					ArrayCpy(gFlashData.frameCmd,mCmd,2);
					if(gcRecvLen == mUserLen)       //��Ϊ��֡����
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
						//����ư��2��-ִ�в���
					}
				}

			}
			wdog_feed();	
			if (mSendLen>0)                      //������Ҫ���͵�����
			{
				mflag = wifi_sendN(mSendData,mSendLen,gUserData.IMSI);
			}
			gcRecvLen = 0;                    //�������ݳ������㣬�����Ѿ���ȡ
			if(mWriteFlashFlag == 1)         //�ж��Ƿ���Ҫдflash
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
			mSendFlag = 0;                    //�޸ķ��ͱ��
			mSendLen=0;
    }
}

//=====================================================================
//�������ƣ�userData_get
//�������أ���
//����˵����data����Ҫ��ֵ�Ľṹ������
//���ܸ�Ҫ�����û�֡�ṹ��data��ֵ
//=====================================================================
void userData_get(UserData *data)             //���û�֡�ṹ��data��ֵ
{
	wdog_feed();	
    uint16_t mcu_temp_AD;
	float mcu_temp;
    static uint32_t sn = 0;
    data->sn = sn++;
    //��ȡmcu�¶�
    mcu_temp_AD = adc_read(AD_MCU_TEMP);       //��ȡmcu�¶�ͨ��ADֵ
    mcu_temp=TempTrans(mcu_temp_AD);           //�¶Ȼع�
    data->mcuTemp =(int32_t)(TEMP) ;
    main_TimeStamp();                          //��gUserData.currentTime��ֵ
    //����ư��1��-���ݻ�ȡ
    wifi_get_mac(&gMac);
    ArrayCpy(data->IMSI,(uint8_t*)gMac.stamac,15);
}


