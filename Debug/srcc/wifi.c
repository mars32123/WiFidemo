//======================================================================
//文件名称：wifi.c（wifi构件源文件）
//制作单位：苏大arm技术中心(sumcu.suda.edu.cn)
//更新记录：20190513
//======================================================================
#include "wifi.h"
#include "stdio.h"
#include "string.h"
#include "printf.h"

//wifi连接指令
#define WIFI_MODE_SET "AT+CWMODE="
#define WIFI_LINK_AP  "AT+CWJAP="         //连接AP
#define WIFI_QUIT_AP  "AT+WJAPQ\r"      //断开AP
#define WIFI_DHCP     "AT+WDHCP=ON\r"     //开启DHCP

//wifi透传设置
#define WIFI_CIP_MODE0 "+++" //非透传模式
#define WIFI_CIP_MODE1 "AT+CIPSENDRAW\r" //透传模式

//wifi通信指令
#define WIFI_STATUS "AT+CIPSTATUS\r\n"    //查询连接状态
#define WIFI_START "AT+CIPSTART="         //建立连接
#define WIFI_CLOSE "AT+CIPSTOP=1\r"      //关闭连接
#define WIFI_SEND "AT+CIPSEND\r\n"        //发送指令
#define WIFI_PING "AT+PING="              //ping指定ip
#define WIFI_DOMAIN_RESOLVE "AT+CIPDOMAIN="  //域名解析

//通信参数设置和查询指令
#define WIFI_STA_MAC_Q "AT+CIPSTAMAC?\r\n"//查询station模式下的mac地址
#define WIFI_STA_MAC_S "AT+CIPSTAMAC="    //设置station模式下的mac地址
#define WIFI_STA_IP    "AT+CIPSTA="       //设置station模式下的IP地址
#define WIFI_SET_AP    "AT+CWSAP="        //设置AP模式的参数

//接收模式
#define WIFI_RECV_DATA 0
#define WIFI_RECV_CMD  1
#define WIFI_RECV_RST  2

//透传模式
#define WIFI_NONE_PASS_THROUGH 0
#define WIFI_PASS_THROUGH 1

//电源控制
#define WIFI_ON  1
#define WIFI_OFF 0

//内部函数使用声明
uint8_t wifi_send_cmd(uint8_t* cmdstring);
uint8_t wifi_check_mac(uint8_t* mac);
uint8_t wifi_check_ip(uint8_t* ip);
uint8_t wifi_check_port(uint8_t* port);
uint8_t wifi_disconnect();
uint8_t wifi_mode_set(uint8_t modetype);
uint8_t wifi_set_send_mode(uint8_t mode);
uint8_t wifi_set_ap_para(uint8_t* ssid,uint8_t* pssword,uint8_t chn);
void wifi_power_ctr(uint8_t state);
void wifi_frameEncode(uint8_t *imsi,uint8_t *data,uint16_t dlen,
                 uint8_t *frame,uint16_t *framelen);
uint8_t wifi_rebootcheck(uint8_t ch);
uint8_t uecom_recv(uint8_t ch,uint16_t *dataLen, uint8_t *data);
uint8_t wifi_returncheck(uint8_t ch);
uint16_t crc16(uint8_t *ptr,uint16_t len);
extern void ArrayCpy(uint8_t * dest,uint8_t*source,uint16_t len);
//WIFI 通信指令接收用全局变量
static uint8_t  WIFI_CMD[400];   //命令返回数据接收缓冲区
static uint16_t WIFI_CMDLENGTH;  //当前接收命令返回字符串长度，包括\0字符
static uint8_t  WIFI_CMD_FLAG;


static uint8_t  WIFI_RECVSTOP;    //接收模式标志

static uint8_t  WIFI_ERR_STOP[16]; //失败帧尾
static uint8_t  WIFI_OK_STOP[21]; //成功帧尾

//组帧用全局变量
static uint8_t frameHead[2] = "V!"; //帧头
static uint8_t frameTail[2] = "S$"; //帧尾

static uint8_t MODE_TYPE;
static uint8_t WIFI_CONNECT_STATE;  //服务器连接状态
static uint8_t WIFI_AP_STATE;       //WIFI连接状态
static uint8_t WIFI_SEND_STATE;     //WIFI发送模式
static uint8_t WIFI_BOOTFLAG;
MAC WIFI_MAC;
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
void wifi_init(uint8_t mode_type,uint8_t* mode_name,uint8_t* pssword,uint8_t chn,uint8_t* ip)
{
	//定义局部变量
	//1.初始化全局变量
	WIFI_RECVSTOP = 0;                     //接收模式标志
	WIFI_CMDLENGTH = 0;                    //命令返回长度置零
	WIFI_CONNECT_STATE=0;                  //TCP连接状态
	WIFI_AP_STATE=0;                       //Wi-Fi连接状态
	WIFI_SEND_STATE=0;                     //透传模式下发送模式状态标志
	WIFI_BOOTFLAG = 0;                     //启动完成标志
	WIFI_CMD_FLAG = 0;                     //命令状态标志
	//2.初始化通信用串口并打开WIFI电源
	uart_init(UART_WIFI,115200);           //初始化通信串口，波特率115200
	uart_enable_re_int(UART_WIFI);         //打开串口接收中断
	gpio_init(POWER_PIN,GPIO_OUTPUT,0);
	WIFI_RECVSTOP = 2;                     //设置当前接收重启模式
	wifi_power_ctr(WIFI_ON);               //打开Wi-Fi模块电源
	//for(;!WIFI_BOOTFLAG;);                 //等待启动完成
	//3.设置模块工作参数
	strcpy((char*)WIFI_OK_STOP,"OK");              //设置工作方式成功帧尾OK
	strcpy((char*)WIFI_ERR_STOP,"ERROR");          //设置工作方式失败帧尾OK
	switch(mode_type)
	{
	    //3.1 设置成STA模式,只能连接AP
	    case WIFI_STATION:
	    	while(wifi_mode_set(WIFI_STATION));     //设置Wi-Fi模块为STA模式
	    	MODE_TYPE = 0;                          //标记当前模式为STA模式
	    	wifi_send_cmd((uint8_t*)"AT+CWAUTOCONN=1\r\n");   //开启wifi模块自动连接
	    	//设置STA模式的IP
	    	if(wifi_check_ip(ip))
	    	{
	    		//输入的ip值为"0.0.0.0"
	    		if(ip[0] == '0' && ip[1] == '.' &&
	    		ip[2] == '0' && ip[3] == '.' &&
				ip[4] == '0' && ip[5] == '.' && ip[6] == '0')
	    		{
	    			wifi_send_cmd((uint8_t*)WIFI_DHCP);       //开启DHCP
	    		}
	    		else
	    			wifi_set_ip(ip);                //设为固定IP
	    	}
	    	else
	    		wifi_send_cmd((uint8_t*)WIFI_DHCP);
	    	break;
	    //3.2 设置成AP模式,只能作为AP供其他设备连接
	    case WIFI_AP:
	    	while(wifi_mode_set(WIFI_AP));            //设置Wi-Fi模块为AP模式
	    	wifi_set_ap_para(mode_name,pssword,chn);  //设置AP模式的SSID、密码和工作信道
	    	MODE_TYPE = 1;                            //标记当前模式为AP模式
	    	break;
	    //3.3 设置成混合模式,既可以连接AP也可以作为AP
	    case WIFI_ST_AP:
		    while(wifi_mode_set(WIFI_ST_AP));         //设置Wi-Fi模块为混合模式
		    wifi_set_ap_para(mode_name,pssword,chn);  //设置混合模式下AP的SSID、密码和工作信道
		    MODE_TYPE = 2;                            //标记当前模式为混合模式
	    	if(wifi_check_ip(ip))
	    	{
	    		//输入的ip值为"0.0.0.0"
	    		if(ip[0] == '0' && ip[1] == '.' &&
	    		ip[2] == '0' && ip[3] == '.' &&
				ip[4] == '0' && ip[5] == '.' && ip[6] == '0')
	    		{
	    			wifi_send_cmd((uint8_t*)WIFI_DHCP);
	    		}
	    		else
	    			wifi_set_ip(ip);
	    	}
	    	else
	    		wifi_send_cmd((uint8_t*)WIFI_DHCP);
		    break;
	    default:
	    	return;
	}
	//4.重启模块
	wifi_reset();
}

//======================================================================
//函数名称：wifi_reset
//功能概要：重启Wi-Fi模块
//参数说明：无
//函数返回：无
//======================================================================
void wifi_reset()
{
	wdog_feed();	
	//1.初始化全局变量
	WIFI_RECVSTOP = 0;                     //接收模式标志
	WIFI_CMDLENGTH = 0;                    //命令返回长度置零
	WIFI_CONNECT_STATE=0;                  //TCP连接状态
	WIFI_AP_STATE=0;                       //Wi-Fi连接状态
	WIFI_SEND_STATE=0;                     //透传模式下发送模式状态标志
	WIFI_BOOTFLAG = 0;                     //启动完成标志
	WIFI_CMD_FLAG = 0;                     //命令状态标志
	uart_init(UART_WIFI,115200);           //初始化通信串口，波特率115200
	uart_enable_re_int(UART_WIFI);         //打开串口接收中断
	//2.其他设置
	//2.1 设置命令成功帧尾和命令失败帧尾
	strcpy((char*)WIFI_OK_STOP,"OK");
	strcpy((char*)WIFI_ERR_STOP,"ERROR");
	//2.2 关闭命令回显
	wdog_feed();	
	wifi_send_cmd((uint8_t*)"AT+UARTE=OFF\r");
	//2.3 关闭回发事件
	wdog_feed();	
	wifi_send_cmd((uint8_t*)"AT+CIPEVENT=OFF\r");
}

//======================================================================
//函数名称：wifi_resolve_domain
//功能概要：连接ap后，进行域名解析
//参数说明：domain:域名,retVal:返回域名解析存放的缓冲区
//函数返回：域名解析是否成功=0成功，=1错误，=2无返回数据
//======================================================================
uint8_t wifi_resolve_domain(uint8_t* domain,uint8_t* retVal)
{
	uint8_t flag,*cmd;
	uint16_t i;
	cmd=malloc((sizeof(domain)-1)+(sizeof(WIFI_DOMAIN_RESOLVE)-1)+5);
	if(cmd==NULL)
		return WIFI_ERROR;
	strcpy((char*)WIFI_OK_STOP,"OK");
	strcpy((char*)WIFI_ERR_STOP,"ERROR");
	sprintf((char*)cmd,"%s\"%s\"\r\n",WIFI_DOMAIN_RESOLVE,domain);
	flag=wifi_send_cmd(cmd);
	free(cmd);
	for(i=0;i<WIFI_CMDLENGTH;i++)
		retVal[i]=WIFI_CMD[i];
	return flag;
}

//======================================================================
//函数名称：wifi_ping
//功能概要：连接ap后，ping指定ip，也可以是域名
//参数说明：address:需要ping的目标地址,time存放ping通返回时间
//函数返回：是否ping通=0成功，=1错误，=2无返回数据
//======================================================================
uint8_t wifi_ping(uint8_t* address,uint8_t* time)
{
	uint8_t flag,*cmd;
	uint16_t i;
	cmd=malloc((sizeof(address)-1)+(sizeof(WIFI_PING)-1)+5);
	if(cmd==NULL)
		return WIFI_ERROR;
	sprintf((char*)cmd,"%s\"%s\"\r\n",WIFI_PING,address);
	flag=wifi_send_cmd(cmd);
	free(cmd);
	if(flag==WIFI_OK)
		for(i=0;i<WIFI_CMDLENGTH;i++)
			time[i]=WIFI_CMD[i];
	return flag;
}

//======================================================================
//函数名称：wifi_get_mac
//功能概要：获取本机mac
//参数说明：mac:返回获取的mac值
//函数返回：获取成功返回1，失败返回0
//======================================================================
uint8_t wifi_get_mac(MAC* mac)
{
	uint8_t *addr,i;
	wifi_send_cmd((uint8_t*)"AT+WMAC?\r\n");
	for(uint32_t i= 0; i < (3000); i++) wdog_feed();
	addr=(uint8_t*)strstr((char*)WIFI_CMD,(char*)":");
	if(addr != NULL)
	{
		i = 3;
		addr++;
	    WIFI_MAC.stamac[0] = '0';
		WIFI_MAC.stamac[1] = '0';
		WIFI_MAC.stamac[2] = '0';
		while(*addr != '\r')
		{
			WIFI_MAC.stamac[i++] = *addr;
			addr++;
		}
		WIFI_MAC.stamac[i] = '\0';
	}
//	printf("WIFI_MAC:%s\r\n",(char *)WIFI_MAC.stamac);
	switch(MODE_TYPE)
	{
	    case 0:strcpy((char*)mac->stamac,(char*)WIFI_MAC.stamac);break;
	    case 1:strcpy((char*)mac->apmac,(char*)WIFI_MAC.apmac);break;
	    case 2:strcpy((char*)mac->stamac,(char*)WIFI_MAC.stamac);
	           strcpy((char*)mac->apmac,(char*)WIFI_MAC.apmac);
	           break;
	    default:return 0;
	}
//	printf("mac->apmac:%s\r\n",mac->stamac);
	return 1;
}


//======================================================================
//函数名称：wifi_int_re
//功能概要：中断接收字符
//参数说明：ch:要接受的字符，*dataLen:存储接收到的数据长度，
//      *data:将接收到的数据存放到该数组中;
//函数返回：无
//======================================================================
void wifi_int_re(uint8_t ch,uint16_t *dataLen, uint8_t *data)
{
	//定义局部变量
	//1.WIFI_ACKSTOP大于零时接收指令回信
	switch(WIFI_RECVSTOP)
	{
	    //数据接收模式
	    case 0:uecom_recv(ch,dataLen,data);
	    	break;
	    //命令返回接收模式
	    case 1:if(WIFI_CMDLENGTH<400)
               {
 	               wifi_returncheck(ch);
 	               WIFI_CMD[WIFI_CMDLENGTH+1]=0;
               }
	    	   break;
	    //重启模式
	    case 2:WIFI_BOOTFLAG = wifi_rebootcheck(ch);
	           if(WIFI_BOOTFLAG)
	        	   WIFI_RECVSTOP = 0;
	           break;                        //重启等待
	    case 3:
	    break;
	}
}

//======================================================================
//函数名称：wifi_get_state
//功能概要：获取模块连接状态
//参数说明：无
//函数返回：=0未连接WIFI和服务器,=1已连接WIFI但未连接服务器,=3已连接WIFI和服务器
//======================================================================
uint8_t wifi_get_state()
{
	return (WIFI_CONNECT_STATE<<1)|(WIFI_AP_STATE);
}

//======================================================================
//函数名称：wifi_linktossid
//功能概要：接入指定wifi接入点
//参数说明：ssid:接入点名称,password:密码
//函数返回：接入是否成功=0成功，=1错误，=2无返回数据
//======================================================================
uint8_t wifi_linktossid(uint8_t* ssid,uint8_t* password)
{
	//定义局部变量
	uint8_t cmd[40],flag;
	//2.设置帧尾
	strcpy((char*)WIFI_OK_STOP,"OK");
	strcpy((char*)WIFI_ERR_STOP,"ERROR");
	strcpy((char*)cmd,"AT+WJAP=");
	strcat((char*)cmd,(char*)ssid);
	strcat((char*)cmd,",");
	strcat((char*)cmd,(char*)password);
	strcat((char*)cmd,"\r");
	//3.发送指令
	flag=wifi_send_cmd(cmd);
	if(flag==WIFI_NORESPONSE)
		wifi_send_cmd((uint8_t*)"AT+WJAP=?\r");
	if(strstr((char*)WIFI_CMD,(char*)WIFI_OK_STOP)!=NULL)
		flag=WIFI_OK;
	if(flag==WIFI_OK)
		WIFI_AP_STATE=1;
	return (flag);
}

//======================================================================
//函数名称：wifi_quitssid
//功能概要：断开wifi接入点的连接
//参数说明：无
//函数返回：断开是否成功=0成功，=1错误，=2无返回数据
//======================================================================
uint8_t wifi_quitssid()
{
	strcpy((char*)WIFI_OK_STOP,"OK");
	strcpy((char*)WIFI_ERR_STOP,"ERROR");
	return (wifi_send_cmd((uint8_t*)WIFI_QUIT_AP));
}

//======================================================================
//函数名称：wifi_con_srv
//功能概要：与服务器建立连接
//参数说明：ip:服务器IP地址,port:端口号（均为字符串格式）,
//       格式"xxx.xxx.xxx.xxx",xxx取值范围0-255
//函数返回：=0连接成功,=1连接失败,=2处于发送模式
//======================================================================
uint8_t wifi_con_srv(uint8_t* ip,uint8_t* port)
{
	//定义局部变量
	uint8_t flag,cmd[100];
	//1.检测wifi模块当前状态是否处于发送模式
	if(WIFI_SEND_STATE==1)
		return 2;
	//2.检测输入的ip地址是否合法
	if(!wifi_check_ip(ip))
		return WIFI_ERROR;
	//3.检测输入的port值是否合法
	if(!wifi_check_port(port))
	    return WIFI_ERROR;
	//4.设置帧尾
	strcpy((char*)WIFI_OK_STOP,"OK");
	strcpy((char*)WIFI_ERR_STOP,"ERROR");
	//5.断开已有连接
	wifi_disconnect();
	//6.拼接指令

	port[5]=0;
	strcpy((char*)cmd,(char*)WIFI_START);
	strcat((char*)cmd,"1,tcp_client,");
	strcat((char*)cmd,(char*)(ip));
	strcat((char*)cmd,",");
	strcat((char*)cmd,(char*)(port));
	strcat(((char*)cmd),"\r");

	//7.发送指令
	flag=wifi_send_cmd(cmd);
	return flag;
}

//======================================================================
//函数名称：wifi_sendN
//功能概要：连接建立后，向服务器发送N个字符,如果是第一次发送,会开启发送模式,开启
//发送模式后部分函数会不可用,具体请看各函数说明
//参数说明：data:需要发送的字符,length:数据长度,IMSI:本机的IMSI号
//函数返回：发送是否成功=0成功,=1开启发送模式失败
//======================================================================
uint8_t wifi_sendN(uint8_t* data,uint16_t length,uint8_t* IMSI)
{
	uint8_t buf[400], cmd[20], strlength[5];
	if(WIFI_SEND_STATE==0)
	{
		//3.拼接发送请求帧尾
		strcpy((char*)WIFI_OK_STOP,"OK");
		strcpy((char*)WIFI_ERR_STOP,"ERROR");
		WIFI_SEND_STATE=1;
	}

	wifi_frameEncode(IMSI,data,length,buf,&length);
	//5.发送内容
	itoa(length,(char*)strlength,10);
	strcpy((char*)cmd,"AT+CIPSEND=1,");
	strcat((char*)cmd,(char*)strlength);
	strcat((char*)cmd,"\r");
	uart_send_string(UART_WIFI,(uint8_t*)cmd);
	uart_sendN(UART_WIFI,length,buf);
	return WIFI_OK;
}

//======================================================================
//函数名称：wifi_set_ip
//功能概要：设置sta模式下WiFi模块的IP地址
//参数说明：ip:要设置的IP地址
//函数返回：=0设置成功，=1设置失败，=2WiFi模块无应答
//======================================================================
uint8_t wifi_set_ip(uint8_t* ip)
{
	uint8_t cmd[30];
	if(!wifi_check_ip(ip))
	{
		return WIFI_ERROR;
	}
	sprintf((char*)cmd,"%s\"%s\"\r\n",WIFI_STA_IP,ip);
	return wifi_send_cmd(cmd);
}



//===========================内部函数====================================
//======================================================================
//函数名称：wifi_check_port
//功能概要：检查端口号是否合法
//参数说明：port:需要检查的端口号字符串,端口号范围应为0-65535
//函数返回：=1合法，=0不合法
//======================================================================
uint8_t wifi_check_port(uint8_t* port)
{
	//定义局部变量
	uint8_t length,maxlength;
	uint32_t num;
	maxlength=5;    //端口字符串内容最长5字
	for(length=0,num=0;*port!='\0'&&length<maxlength;length++)
	{
		//是数字则计算新的num值，否则返回0
		if(*port>='0'&&*port<='9')
			num=num*10+(*port-'0');
		else return 0;
		port++;
	}
	//超长返回0
	if(length>maxlength)
		return 0;
	//端口值超出范围
	if(num>65535)
		return 0;
	return 1;
}

//======================================================================
//函数名称：wifi_check_ip
//功能概要：检查ip地址是否合法
//参数说明：ip:需要检查的ip地址字符串,格式"xxx.xxx.xxx.xxx",xxx取值范围0-255
//函数返回：=1合法，=0不合法
//======================================================================
uint8_t wifi_check_ip(uint8_t* ip)
{
	//定义局部变量
	uint8_t i,j,maxlength;           //i用作标记子块,j用作记录子块长度
	uint16_t length,num[4];
	//初始化局部变量
	for(i=0;i<4;i++)
		num[i]=0;
	maxlength=15;
	//1.检查长度并将整个ip字符串切分成四块
	for(length=0,i=0,j=0;*ip!='\0'&&length<=maxlength;length++)
	{
		if(*ip=='.')
		{
			if(j==0||j>3)
				return 0;
			i++;                     //子块标识加一
			j=0;                     //子块长度清零开始计算下一个子块
		}
		else if(*ip>='0'&&*ip<='9')
		{
			num[i]=num[i]*10+(*ip-'0');
			j++;                     //当前子块长度+1
		}
		else return 0;               //检测到非数字返回0
		ip++;                        //转到下个字符
	}
	if(length>maxlength)             //超长,返回0
		return 0;
	//2.检测子块是否小于等于255
	for(i=0;i<4;i++)
	{
		if(!(num[i]<256))            //检测到超过255的子块,返回0
			return 0;
	}
	return 1;                        //所有条件均满足,返回1
}

//======================================================================
//函数名称：wifi_check_mac
//功能概要：检查mac值是否合法
//参数说明：mac:mac值
//函数返回：=1合法，=0不合法
//======================================================================
uint8_t wifi_check_mac(uint8_t* mac)
{
	uint8_t flag,i;
	flag=1;
	for(i=0;i<17;i++)
	{
		if((i+1)%3==0)
		{
			if(mac[i]!=':')
			{
				flag=0;
				break;
			}
		}
		else
			if(!((mac[i]>='0'&&mac[i]<='9')||
			(mac[i]>='a'&&mac[i]<='f')))
			{
				flag=0;
				break;
			}
	}
	return flag;
}


//======================================================================
//函数名称：wifi_disconnect
//功能概要：断开tcp或udp连接
//参数说明：无
//函数返回：断开连接是否成功=0成功，=1错误，=2无返回数据
//======================================================================
uint8_t wifi_disconnect()
{
	//1.拼接帧尾
	strcpy((char*)WIFI_OK_STOP,"OK");
	strcpy((char*)WIFI_ERR_STOP,"ERROR");
	//2.断开连接
    return wifi_send_cmd((uint8_t*)WIFI_CLOSE);
}

//======================================================================
//函数名称：wifi_mode_set
//功能概要：设置WIFI应用模式
//参数说明：modetype:应用模式,=1 Station模式
//                      =2 AP模式
//                      =3 混合模式
//函数返回：模式设置是否成功=0成功，=1错误，=2无返回数据
//======================================================================
uint8_t wifi_mode_set(uint8_t modetype)
{
	uint8_t cmd[14];
	if(modetype<1||modetype>3)
		return WIFI_ERROR;
	strcpy((char*)WIFI_OK_STOP,"OK");
	strcpy((char*)WIFI_ERR_STOP,"ERROR");
	sprintf((char*)cmd,"%s%d\r\n",WIFI_MODE_SET,modetype);
	return (wifi_send_cmd(cmd));
}

//======================================================================
//函数名称：wifi_send_cmd
//功能概要：发送wifi模块命令，在中断开启的情况下能够返回WiFi模块应答内容
//参数说明：cmdstring:命令字符串
//函数返回：发送后wifi模块应答状态，=0成功，=1错误，=2无应答
//======================================================================
uint8_t wifi_send_cmd(uint8_t* cmdstring)
{
	//定义局部变量
	vuint32_t i;
	//1.初始化全局变量
	WIFI_CMDLENGTH = 0;
	WIFI_CMD_FLAG = 0;
	WIFI_RECVSTOP = WIFI_RECV_CMD;
	//2.向串口发送指令
	uart_send_string(UART_WIFI,cmdstring);
	i=0;
	//3.监听成功状态和失败状态
	while(1)
	{
		wdog_feed();	
		//3.1 监听到成功状态
		if(WIFI_CMD_FLAG == 1)
		{
			WIFI_RECVSTOP = WIFI_RECV_DATA;    //设置中断接收模式为数据接收模式
			return WIFI_OK;                    //返回命令成功标志WIFI_OK（0）
		}
		//3.2 监听到失败状态
		else if(WIFI_CMD_FLAG == 2)
		{
			WIFI_RECVSTOP = WIFI_RECV_DATA;    //设置中断接收模式为数据接收模式
			return WIFI_ERROR;                 //返回命令失败标志WIFI_ERROR（1）
		}
		i++;                                   //计次标志加一
		//3.3 监听超时
		if(i>0xffffff)
		{
			WIFI_RECVSTOP = WIFI_RECV_DATA;    //设置中断接收模式为数据接收模式
			return WIFI_NORESPONSE;            //返回命令超时标志WIFI_NORESPONSE（2）
		}
	}
}

//======================================================================
//函数名称：wifi_set_send_mode
//功能概要：设置模块发送模式,使用tcp发送数据必须使用透传模式
//参数说明：mode:发送模式选择，=0非透传模式，=1透传模式
//         WIFI_NONE_PASS_THROUGH 0
//         WIFI_PASS_THROUGH 1
//函数返回：设置是否成功=0命令返回成功，=1命令返回失败，=2命令返回超时
//======================================================================
uint8_t wifi_set_send_mode(uint8_t mode)
{
	//定义局部变量
	uint8_t flag;            //命令返回标志
    //1.设置命令成功帧尾和命令失败帧尾
	strcpy((char*)WIFI_OK_STOP,"OK");
	strcpy((char*)WIFI_ERR_STOP,"ERROR");
	//2.判断mode的值，对应的值发送对应的语句，如果mode不是0或1则返回命令失败
    switch(mode)
	{
		case 0:flag=wifi_send_cmd((uint8_t*)WIFI_CIP_MODE0);break;    //=0关闭透传模式
		case 1:flag=wifi_send_cmd((uint8_t*)WIFI_CIP_MODE1);break;    //=1开启透传模式
		default:return WIFI_ERROR;                          //其他返回命令失败
	}
	return flag;
}

//======================================================================
//函数名称：wifi_set_ap_para
//功能概要：设置模块AP模式下的参数，包括SSID、密码和AP信道
//参数说明：ssid:要设置的ssid字符串,pssword:要设置的密码字符串，必须是64位以内
//         ASCII字符,chn:wifi信道号，1~13，只有1、6、11三个信道互不干扰
//函数返回：设置是否成功=0成功，=1错误，=2无返回数据
//======================================================================
uint8_t wifi_set_ap_para(uint8_t* ssid,uint8_t* pssword,uint8_t chn)
{
	uint8_t flag,*cmd;
    //1.设置命令成功帧尾和命令失败帧尾
	strcpy((char*)WIFI_OK_STOP,"OK");
	strcpy((char*)WIFI_ERR_STOP,"ERROR");
    //2.拼接指令
	cmd=malloc(sizeof(ssid)+sizeof(pssword)+20);
	sprintf((char*)cmd,"%s\"%s\",\"%s\",%d,4\r\n",WIFI_SET_AP,ssid,pssword,chn);
	flag=wifi_send_cmd(cmd);
	free(cmd);
	return flag;
}

//====================================================================
//函数名称：wifi_power_ctr
//功能概要：控制wifi电源
//参数说明：state:WIFI_ON(1):打开电源,WIFI_OFF(0):关闭电源
//函数返回：无
//====================================================================
void wifi_power_ctr(uint8_t state)
{
	if(state==WIFI_ON)
		gpio_set(POWER_PIN,1);
	else
		gpio_set(POWER_PIN,0);
}

//====================================================================
//函数名称：wifi_frameEncode
//功能概要：实现待发送数据的组帧,将待发送数据加上帧头、帧长、帧尾以及校验信息
//       frame=帧头+IMSI+data长度+data+CRC校验码+帧尾
//参数说明：imsi：发送数据的设备的IMSI号
//       data:待组帧的数据头指针
//       dlen:待组帧的数据长度
//       frame:组帧之后的待发送数据帧
//       framelen:组帧后的数据长度
//函数返回：无
//====================================================================
void wifi_frameEncode(uint8_t *imsi,uint8_t *data,uint16_t dlen,
                 uint8_t *frame,uint16_t *framelen)
{
    uint16_t crc,len;

    //组帧操作
    //帧头
    frame[0] = frameHead[0];
    frame[1] = frameHead[1];
    len = 2;
    //IMSI号
    strncpy((char *)(frame+2),(const char *)imsi,15);
    len += 15;
    //data长度
    frame[len++] = (uint8_t)(dlen>>8);
    frame[len++] = (uint8_t)dlen;
    //data
    memcpy(frame+19,data,dlen);
    len += dlen;
    //CRC校验码
    crc = crc16(frame+2,dlen+17);//对IMSI号、data长度和data进行CRC校验
    frame[len++] = (uint8_t)(crc>>8);
    frame[len++] = (uint8_t)crc;
    //帧尾
    frame[len++] = (uint8_t)frameTail[0];
    frame[len++] = (uint8_t)frameTail[1];
    frame[len] = 0;
    *framelen = len;
}

//====================================================================
//函数名称：wifi_returncheck
//函数返回：1：检测到命令返回；0:未检测到命令返回
//参数说明：ch：串口接收到的数据(1字节)
//功能概要：检测Wi-Fi模块是否有命令返回
//====================================================================
uint8_t wifi_returncheck(uint8_t ch)
{
    static uint8_t index1 = 0;   //成功返回帧索引
    static uint8_t index2 = 0;   //错误返回帧索引
    static uint8_t len1 = 0;     //成功返回帧长度
    static uint8_t len2 = 0;     //错误返回帧长度
    uint8_t ret_val;
    //首次进入命令模式
    if(WIFI_CMDLENGTH == 0)
    {
    	len1 = strlen((char*)WIFI_OK_STOP);
    	len2 = strlen((char*)WIFI_ERR_STOP);
    	index1 = 0;
    	index2 = 0;
    }
    WIFI_CMD[WIFI_CMDLENGTH++] = ch;
    if((index1 == 0 && ch!=WIFI_OK_STOP[0])&&(index2 == 0 && ch!=WIFI_ERR_STOP[0]))
    {
    	ret_val = 0;
    	goto wifi_returncheck_exit;
    }
    ret_val = 0;
    if(ch == WIFI_OK_STOP[index1])
    	index1++;
    else if(ch == WIFI_OK_STOP[0])
    	index1 = 1;
    if(ch == WIFI_ERR_STOP[index2])
    	index2++;
    else if(ch == WIFI_ERR_STOP[0])
    	index2 = 1;
    if(index1 == len1)
    {
    	ret_val = 1;
    	WIFI_CMD_FLAG = 1;
    	goto wifi_returncheck_exit;
    }
    if(index2 == len2)
    {
    	ret_val = 1;
    	WIFI_CMD_FLAG = 2;
    }
wifi_returncheck_exit:
    return ret_val;
}


//====================================================================
//函数名称：wifi_rebootcheck
//函数返回：1：检测到启动完成；0:未检测到启动完成
//参数说明：ch：串口接收到的数据(1字节)
//功能概要：检测Wi-Fi模块是否启动完成
//====================================================================
uint8_t wifi_rebootcheck(uint8_t ch)
{
	//定义局部变量
    static uint8_t index1 = 0;   //帧索引
    static uint8_t bootflag[5] = "ready";   //启动完成信息帧尾
    uint8_t ret_val;
    //下面的代码使用了大量的goto语句是为了尽量减少代码的执行量，来保证中断能够及时响应
    //1.没有遇到帧尾中的任意一个字符
    if(index1 == 0 && ch != bootflag[0])
    {
    	ret_val = 0;
    	goto wifi_rebootcheck_exit;
    }
    //2.检测遇到帧尾的第一个字符
    if(ch==bootflag[0])
    {
    	ret_val = 0;
    	index1 = 1;
    	goto wifi_rebootcheck_exit;
    }
    //3.检测遇到帧尾的第二个字符
    if(index1 == 1 && ch == bootflag[1])
    {
    	ret_val = 0;
    	index1 = 2;
    	goto wifi_rebootcheck_exit;
    }
    //4.检测遇到帧尾的第三个字符
    if(index1 == 2 && ch == bootflag[2])
    {
        ret_val = 0;
        index1 = 3;
        goto wifi_rebootcheck_exit;
    }
    //5.检测遇到帧尾的第四个字符
    if(index1 == 3 && ch == bootflag[3])
    {
        ret_val = 0;
        index1 = 4;
        goto wifi_rebootcheck_exit;
    }
    //6.检测遇到帧尾的第五个字符
    if(index1 == 4 && ch == bootflag[4])
    {
        ret_val = 1;                     //帧尾中的字符全部检测到
        index1 = 0;
        goto wifi_rebootcheck_exit;
    }
    else
    {
    	ret_val = 0;
    	index1 = 0;
    	goto wifi_rebootcheck_exit;
    }
wifi_rebootcheck_exit:
    return ret_val;
}

//====================================================================
//函数名称：uecom_recv
//函数返回：0：接收到了通过TCP/UDP发来的数据；1：正在解帧；2接收数据错误
//参数说明：ch：串口接收到的数据(1字节)，*dataLen:存储接收到的数据长度，
//      *data:将接收到的数据存放到该数组中;
//功能概要：对接收到来自服务器的数据进行解帧，并将有效数据存入data数组之中，由串口的uart中断调用。
//修改日期：【2018-06-30】,LXD
//====================================================================
uint8_t uecom_recv(uint8_t ch,uint16_t *dataLen, uint8_t *data)
{
    static vuint16_t index1 = 0;   //帧索引
    static vuint16_t length = 0;  //数据长度
    vuint8_t ret_val;
    vuint16_t i;
    vuint8_t WCRC[2];
    vuint16_t wcrc;
    //如果未遇到帧头或者未收到帧头后面的数据(即不是数据帧)
    if((index1 == 0 && ch != frameHead[0]) ||
      (index1 == 1 && ch != frameHead[1]))
    {
    	index1 = 0;
        length = 0;
        ret_val = 2;     //接收数据错误
        goto uecom_recv_exit;
    }
    //至此说明是来自服务器的数据
    data[index1++] = ch;  //存储帧数据
    if(index1 == 19)      //读取data长度
    {
        length = ((uint16_t)data[17]<<8) + data[18];
    }
    //接收到的数据达到一帧长度。length+23为整个帧长度
    if(length != 0 && index1 >= length+23)
    {
        //CRC校验
    	wcrc = crc16(data+2,length+17);
        WCRC[0] = (wcrc>>8)&0xff;
        WCRC[1] = wcrc&0xff;
        if(data[index1-2]!=frameTail[0] ||
        		data[index1-1]!= frameTail[1]  //未遇到帧尾
            || WCRC[0] != data[length+19] ||
			WCRC[1] != data[length+20])        //CRC检验错误
        {
        	index1 = 0;
            length = 0;
            ret_val = 2;         //接收数据错误
            goto uecom_recv_exit;
        }
        for(i=0;i<length;i++)
        {
            data[i] = data[i+19];//19为有效字节前的数据长度
        }
        *dataLen = length;
        index1 = 0;
        length = 0;
        ret_val = 0;             //接收到了通过TCP/UDP发来的数据
        goto uecom_recv_exit;
    }
    *dataLen = 0;
    ret_val = 1;//正在解帧
uecom_recv_exit:
    return ret_val;
}

//=====================================================================
//函数名称：crc16
//功能概要：将数据进行16位的CRC校验，返回校验后的结果值
//参数说明：ptr:需要校验的数据缓存区
//      len:需要检验的数据长度
//函数返回：计算得到的校验值?
//=====================================================================
uint16_t crc16(uint8_t *ptr,uint16_t len)
{
    uint16_t i,j,tmp,crc16;

    crc16 = 0xffff;
    for(i = (uint16_t)0;i < len;i++)
    {
        crc16 = (uint16_t)ptr[i]^crc16;
        for(j = 0;j< 8;j++)
        {
            tmp = crc16 & 0x0001;
            crc16 = crc16>>1;
            if(tmp)
                crc16 = crc16^0xa001;
        }
    }
    return crc16;
}


void WiFiMessageInit()
{
	 ArrayCpy(gWiFiData.serverIP,gFlashData.serverIP,15);
	 ArrayCpy(gWiFiData.serverPort,gFlashData.serverPort,5);
//	 ArrayCpy(gWiFiData.serverPort2,BIOSPort,5);
	 ArrayCpy(gWiFiData.UE_SSID,gFlashData.UE_SSID,32);
	 ArrayCpy(gWiFiData.UE_PSSWD,gFlashData.UE_PSSWD,32);
	 wifi_get_mac(&gMac);
	 ArrayCpy(gWiFiData.IMSI,(uint8_t*)gMac.stamac,15);
}

