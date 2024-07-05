#include "common.h"
//=====================================================================
//函数名称：AryCpy
//函数返回：无
//参数说明：dest：复制后存放的数组；source：被复制的数组；len:复制的长度
//功能概要：从源数组复制指定长度的内容到目标数组
//=====================================================================
void AryCpy(uint8_t* dest,uint8_t* source,uint16_t len)
{
    for(uint16_t r=0;r<len;r++)  dest[r]=source[r];
}




//=====================================================================
//函数名称：TimeStamp
//函数返回：无
//参数说明：gTime：日期时间数组
//功能概要：将日期时间转化为时间戳
//=====================================================================
uint64_t  TimeStamp(uint8_t* gTime)
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
    for (i=0;i<=3;i++) st[i]=gTime[i]-0x30;   
    tyear=st[0]*1000+st[1]*100+st[2]*10+st[3];
    //将月份化为十进制
    for (i=5;i<=6;i++) st[i]=gTime[i]-0x30;  
    tmonth=st[5]*10+st[6];
    //将天化为十进制
    for (i=8;i<=9;i++) st[i]=gTime[i]-0x30;  
    tday=st[8]*10+st[9];
    //将时化为十进制
    for (i=11;i<=12;i++) st[i]=gTime[i]-0x30;  
    thour=st[11]*10+st[12];
    //将分化为十进制
    for (i=14;i<=15;i++) st[i]=gTime[i]-0x30;  
    tmin=st[14]*10+st[15];
    //将秒化为十进制
    for (i=17;i<=18;i++) st[i]=gTime[i]-0x30;  
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
    if(tmonth>12) return 0;
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
    return timestamp;
}
