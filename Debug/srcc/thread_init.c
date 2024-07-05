//======================================================================
//�ļ����ƣ�main.c��Ӧ�ù�����������
//����ṩ��SD-Arm��sumcu.suda.edu.cn��
//�汾���£�20191108-20200419
//�����������������̵�..\01_Doc\Readme.txt
//��ֲ���򣺡��̶���
//======================================================================
#define GLOBLE_VAR
#include "includes.h"      //������ͷ�ļ�
//----------------------------------------------------------------------
//����ʹ�õ����ڲ�����
void userData_init(UserData *data);   
void ArrayCpy(uint8_t * dest,uint8_t*source,uint16_t len);
void main_Dly_ms(uint32_t ms);
void main_TimeStamp();
void comm_init();
void consrv_init();
//----------------------------------------------------------------------
//��������һ������¿�����Ϊ����Ӵ˿�ʼ���У�ʵ�������������̣��μ���壩
void thread_init()
{
    printf("\r\n�����ʼ���߳�thread_init��\r\n");
    //��1���������֣���ͷ��======
    //��1.1�������߳�ʹ�õľֲ�����
   
    //��1.2�������䡿�����ж�
    DISABLE_INTERRUPTS;
    //��1.3�����ֲ���������ֵ
    ArrayCpy(sTimeString,(uint8_t *)"2023-07-18 20:19:00\0",20);
    //��1.4����ȫ�ֱ�������ֵ
    ArrayCpy(gTimeString,(uint8_t *)"2023-07-18 20:19:00\0",20);
    gCount=0;
    gUserData.touchNum=0;
    gUserData.signalPower=0;
    //��1.5���û�����ģ���ʼ��
    gpio_init(LIGHT_RED,GPIO_OUTPUT,LIGHT_ON);    //��ʼ�����
    gpio_init(POWER_PIN,GPIO_OUTPUT,0);           //��ʼ��wifi��Դ
    uart_init(UART_UE,115200);                    //UE���ڳ�ʼ��
    uart_init(UART_2,115200);
    timer_init(TIMER_USER,20);                    //LPTMR��ʱ����ʼ��Ϊ500����
    flash_init();                                 //��ʼ��flash
    adc_init(AD_MCU_TEMP,0);					   //��ʼ��ADоƬ�¶�ģ��
    printf("\r\n AHL-IoT-GEC start...  \n");
    //����ư��1��-��ʼ��
	//��1.6��ʹ��ģ���ж�
	timer_enable_int(TIMER_USER);                //ʹ��LPTMR�ж�
	uart_enable_re_int(UART_UE);
	//��1.7�������䡿�����ж�
	ENABLE_INTERRUPTS;
	//��1.8��������ʵ����Ҫ��ɾ�� ��ѭ��ǰ�ĳ�ʼ������
	printf("\r\n flash_erase and write...        \r\n");
	flash_erase(MCU_SECTOR_NUM-2);               //�������һ������
	flash_write((MCU_SECTOR_NUM-2),28,sizeof(FlashData),(uint8_t *)flashInit);  //�����һ������д����
	printf("\r\n flash_erase and write...OK        \r\n");
	//��1.8.1����ȡflash�е�������Ϣ��gFlashData,��ʼ���û�֡����gUserData
	flash_read_logic((uint8_t*)(&gFlashData),(MCU_SECTOR_NUM-2),28,sizeof(FlashData));
	userData_init(&gUserData);                  //��ʼ���û�֡�ṹ��gUserData
	//��1.8.2���жϸ�λ״̬��������λ״̬���ݴ洢��flash�У�ע�ⲻӰ��ԭ�����ݣ�
	if (IS_POWERON_RESET)                         //�临λ������
		gFlashData.resetCount = 0;
	else                                          //�ȸ�λ�����1
	{
		gFlashData.resetCount++;
		flash_read_logic((uint8_t*)gcRecvBuf,(MCU_SECTOR_NUM-2),0,MCU_SECTORSIZE);
		flash_erase(MCU_SECTOR_NUM-2);
		ArrayCpy(((uint8_t*)gcRecvBuf+166),(uint8_t*)(gFlashData.resetCount),4);
		flash_write((MCU_SECTOR_NUM-2),0,MCU_SECTORSIZE,(uint8_t*)gcRecvBuf);
	}
	printf("\r\n AHL Init                    \r\n");
	for(;;)                                         //��ʼ��ͨ��ģ��
	{
		//WIFI����
		comm_init();
		consrv_init();
		//��20230809����������������д��WIFI������Ϣ
		WiFiMessageInit();
		ArrayCpy(gWiFiData.serverPort2,BIOSPort,5);
		flash_erase(MCU_SECTOR_NUM-3);
		flash_write((MCU_SECTOR_NUM-3),0,sizeof(WiFiData),(uint8_t *)&gWiFiData);  //�����һ������д����  
		gUserData.startTime=TimeStamp(sTimeString); //��gUserData.startTime��ֵ	
		break;
	}
	
	printf("\r\n ��ʼ����������...\r\n");
}   


//======����Ϊ���������õ��Ӻ���===========================================
//=====================================================================
//�������ƣ�userData_init
//�������أ���
//����˵����data����Ҫ��ʼ���Ľṹ������
//���ܸ�Ҫ����ʼ���û�֡�ṹ��data
//=====================================================================
void userData_init(UserData *data)            //��ʼ���û�֡�ṹ��
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
    //����ư��2��-��ʼ������

}



//=====================================================================
//�������ƣ�ArrayCpy
//�������أ���
//����˵����dest�����ƺ��ŵ����飻source�������Ƶ����飻len:���Ƶĳ���
//���ܸ�Ҫ����Դ���鸴��ָ�����ȵ����ݵ�Ŀ������
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
	//������ʱ���� ʮ����
	uint16_t tyear,tmonth,tday,thour,tmin,tsec;
	uint16_t i;
	uint64_t timestamp;	    //ʱ���
	uint8_t st[20];
	//ÿ�·�֮ǰ���������������
	uint16_t MonthDay[12]={31,59,90,120,151,181,212,243,273,304,334,365};
	timestamp=0;
	//����ݻ�Ϊʮ����
	for (i=0;i<=3;i++) st[i]=gTimeString[i]-0x30;
	tyear=st[0]*1000+st[1]*100+st[2]*10+st[3];
	//���·ݻ�Ϊʮ����
	for (i=5;i<=6;i++) st[i]=gTimeString[i]-0x30;
	tmonth=st[5]*10+st[6];
	//���컯Ϊʮ����
	for (i=8;i<=9;i++) st[i]=gTimeString[i]-0x30;
	tday=st[8]*10+st[9];
	//��ʱ��Ϊʮ����
	for (i=11;i<=12;i++) st[i]=gTimeString[i]-0x30;
	thour=st[11]*10+st[12];
	//���ֻ�Ϊʮ����
	for (i=14;i<=15;i++) st[i]=gTimeString[i]-0x30;
	tmin=st[14]*10+st[15];
	//���뻯Ϊʮ����
	for (i=17;i<=18;i++) st[i]=gTimeString[i]-0x30;
	tsec=st[17]*10+st[18];
	//����֮ǰ��ݵ�����
	for(i=1970;i<tyear;i++)
	{
		if(i%4)
			timestamp+=31536000;

		else
			timestamp+=31622400;
	}
	//����֮ǰ�·ݵ�����
	if(tmonth>12) return;
	if(tmonth>2 && tyear%4)
		timestamp+=MonthDay[tmonth-2]*24*3600;
	else if(tmonth>2 && (tyear%4==0))
		timestamp+=(MonthDay[tmonth-2]+1)*24*3600;
	else if(tmonth==2)
		timestamp+=31*24*3600;
	else
		timestamp+=0;
	//���㵱ǰ�������
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

