#include "common.h"
//=====================================================================
//�������ƣ�AryCpy
//�������أ���
//����˵����dest�����ƺ��ŵ����飻source�������Ƶ����飻len:���Ƶĳ���
//���ܸ�Ҫ����Դ���鸴��ָ�����ȵ����ݵ�Ŀ������
//=====================================================================
void AryCpy(uint8_t* dest,uint8_t* source,uint16_t len)
{
    for(uint16_t r=0;r<len;r++)  dest[r]=source[r];
}




//=====================================================================
//�������ƣ�TimeStamp
//�������أ���
//����˵����gTime������ʱ������
//���ܸ�Ҫ��������ʱ��ת��Ϊʱ���
//=====================================================================
uint64_t  TimeStamp(uint8_t* gTime)
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
    for (i=0;i<=3;i++) st[i]=gTime[i]-0x30;   
    tyear=st[0]*1000+st[1]*100+st[2]*10+st[3];
    //���·ݻ�Ϊʮ����
    for (i=5;i<=6;i++) st[i]=gTime[i]-0x30;  
    tmonth=st[5]*10+st[6];
    //���컯Ϊʮ����
    for (i=8;i<=9;i++) st[i]=gTime[i]-0x30;  
    tday=st[8]*10+st[9];
    //��ʱ��Ϊʮ����
    for (i=11;i<=12;i++) st[i]=gTime[i]-0x30;  
    thour=st[11]*10+st[12];
    //���ֻ�Ϊʮ����
    for (i=14;i<=15;i++) st[i]=gTime[i]-0x30;  
    tmin=st[14]*10+st[15];
    //���뻯Ϊʮ����
    for (i=17;i<=18;i++) st[i]=gTime[i]-0x30;  
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
    if(tmonth>12) return 0;
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
    return timestamp;
}
