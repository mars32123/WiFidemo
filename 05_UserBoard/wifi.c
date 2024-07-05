//======================================================================
//�ļ����ƣ�wifi.c��wifi����Դ�ļ���
//������λ���մ�arm��������(sumcu.suda.edu.cn)
//���¼�¼��20190513
//======================================================================
#include "wifi.h"
#include "stdio.h"
#include "string.h"
#include "printf.h"

//wifi����ָ��
#define WIFI_MODE_SET "AT+CWMODE="
#define WIFI_LINK_AP  "AT+CWJAP="         //����AP
#define WIFI_QUIT_AP  "AT+WJAPQ\r"      //�Ͽ�AP
#define WIFI_DHCP     "AT+WDHCP=ON\r"     //����DHCP

//wifi͸������
#define WIFI_CIP_MODE0 "+++" //��͸��ģʽ
#define WIFI_CIP_MODE1 "AT+CIPSENDRAW\r" //͸��ģʽ

//wifiͨ��ָ��
#define WIFI_STATUS "AT+CIPSTATUS\r\n"    //��ѯ����״̬
#define WIFI_START "AT+CIPSTART="         //��������
#define WIFI_CLOSE "AT+CIPSTOP=1\r"      //�ر�����
#define WIFI_SEND "AT+CIPSEND\r\n"        //����ָ��
#define WIFI_PING "AT+PING="              //pingָ��ip
#define WIFI_DOMAIN_RESOLVE "AT+CIPDOMAIN="  //��������

//ͨ�Ų������úͲ�ѯָ��
#define WIFI_STA_MAC_Q "AT+CIPSTAMAC?\r\n"//��ѯstationģʽ�µ�mac��ַ
#define WIFI_STA_MAC_S "AT+CIPSTAMAC="    //����stationģʽ�µ�mac��ַ
#define WIFI_STA_IP    "AT+CIPSTA="       //����stationģʽ�µ�IP��ַ
#define WIFI_SET_AP    "AT+CWSAP="        //����APģʽ�Ĳ���

//����ģʽ
#define WIFI_RECV_DATA 0
#define WIFI_RECV_CMD  1
#define WIFI_RECV_RST  2

//͸��ģʽ
#define WIFI_NONE_PASS_THROUGH 0
#define WIFI_PASS_THROUGH 1

//��Դ����
#define WIFI_ON  1
#define WIFI_OFF 0

//�ڲ�����ʹ������
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
//WIFI ͨ��ָ�������ȫ�ֱ���
static uint8_t  WIFI_CMD[400];   //��������ݽ��ջ�����
static uint16_t WIFI_CMDLENGTH;  //��ǰ����������ַ������ȣ�����\0�ַ�
static uint8_t  WIFI_CMD_FLAG;


static uint8_t  WIFI_RECVSTOP;    //����ģʽ��־

static uint8_t  WIFI_ERR_STOP[16]; //ʧ��֡β
static uint8_t  WIFI_OK_STOP[21]; //�ɹ�֡β

//��֡��ȫ�ֱ���
static uint8_t frameHead[2] = "V!"; //֡ͷ
static uint8_t frameTail[2] = "S$"; //֡β

static uint8_t MODE_TYPE;
static uint8_t WIFI_CONNECT_STATE;  //����������״̬
static uint8_t WIFI_AP_STATE;       //WIFI����״̬
static uint8_t WIFI_SEND_STATE;     //WIFI����ģʽ
static uint8_t WIFI_BOOTFLAG;
MAC WIFI_MAC;
//======================================================================
//�������ƣ�wifi_init
//���ܸ�Ҫ����ʼ��wifiģ��
//����˵����mode_type:����ģ����Ϊ�ͻ��˻���AP�����;
//         mode_name:����ģ��APģʽ�µ�SSID;
//         pssword:Ҫ���õ������ַ�����������64λ����ASCII�ַ�;
//         chn:wifi�ŵ��ţ�1~13��ֻ��1��6��11�����ŵ���������;
//         ip:����Wi-Fiģ��STAģʽ�µ�IP��ַ,����Ϊ0.0.0.0��ʾ����DHCP
//�������أ���
//======================================================================
void wifi_init(uint8_t mode_type,uint8_t* mode_name,uint8_t* pssword,uint8_t chn,uint8_t* ip)
{
	//����ֲ�����
	//1.��ʼ��ȫ�ֱ���
	WIFI_RECVSTOP = 0;                     //����ģʽ��־
	WIFI_CMDLENGTH = 0;                    //����س�������
	WIFI_CONNECT_STATE=0;                  //TCP����״̬
	WIFI_AP_STATE=0;                       //Wi-Fi����״̬
	WIFI_SEND_STATE=0;                     //͸��ģʽ�·���ģʽ״̬��־
	WIFI_BOOTFLAG = 0;                     //������ɱ�־
	WIFI_CMD_FLAG = 0;                     //����״̬��־
	//2.��ʼ��ͨ���ô��ڲ���WIFI��Դ
	uart_init(UART_WIFI,115200);           //��ʼ��ͨ�Ŵ��ڣ�������115200
	uart_enable_re_int(UART_WIFI);         //�򿪴��ڽ����ж�
	gpio_init(POWER_PIN,GPIO_OUTPUT,0);
	WIFI_RECVSTOP = 2;                     //���õ�ǰ��������ģʽ
	wifi_power_ctr(WIFI_ON);               //��Wi-Fiģ���Դ
	//for(;!WIFI_BOOTFLAG;);                 //�ȴ��������
	//3.����ģ�鹤������
	strcpy((char*)WIFI_OK_STOP,"OK");              //���ù�����ʽ�ɹ�֡βOK
	strcpy((char*)WIFI_ERR_STOP,"ERROR");          //���ù�����ʽʧ��֡βOK
	switch(mode_type)
	{
	    //3.1 ���ó�STAģʽ,ֻ������AP
	    case WIFI_STATION:
	    	while(wifi_mode_set(WIFI_STATION));     //����Wi-Fiģ��ΪSTAģʽ
	    	MODE_TYPE = 0;                          //��ǵ�ǰģʽΪSTAģʽ
	    	wifi_send_cmd((uint8_t*)"AT+CWAUTOCONN=1\r\n");   //����wifiģ���Զ�����
	    	//����STAģʽ��IP
	    	if(wifi_check_ip(ip))
	    	{
	    		//�����ipֵΪ"0.0.0.0"
	    		if(ip[0] == '0' && ip[1] == '.' &&
	    		ip[2] == '0' && ip[3] == '.' &&
				ip[4] == '0' && ip[5] == '.' && ip[6] == '0')
	    		{
	    			wifi_send_cmd((uint8_t*)WIFI_DHCP);       //����DHCP
	    		}
	    		else
	    			wifi_set_ip(ip);                //��Ϊ�̶�IP
	    	}
	    	else
	    		wifi_send_cmd((uint8_t*)WIFI_DHCP);
	    	break;
	    //3.2 ���ó�APģʽ,ֻ����ΪAP�������豸����
	    case WIFI_AP:
	    	while(wifi_mode_set(WIFI_AP));            //����Wi-Fiģ��ΪAPģʽ
	    	wifi_set_ap_para(mode_name,pssword,chn);  //����APģʽ��SSID������͹����ŵ�
	    	MODE_TYPE = 1;                            //��ǵ�ǰģʽΪAPģʽ
	    	break;
	    //3.3 ���óɻ��ģʽ,�ȿ�������APҲ������ΪAP
	    case WIFI_ST_AP:
		    while(wifi_mode_set(WIFI_ST_AP));         //����Wi-Fiģ��Ϊ���ģʽ
		    wifi_set_ap_para(mode_name,pssword,chn);  //���û��ģʽ��AP��SSID������͹����ŵ�
		    MODE_TYPE = 2;                            //��ǵ�ǰģʽΪ���ģʽ
	    	if(wifi_check_ip(ip))
	    	{
	    		//�����ipֵΪ"0.0.0.0"
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
	//4.����ģ��
	wifi_reset();
}

//======================================================================
//�������ƣ�wifi_reset
//���ܸ�Ҫ������Wi-Fiģ��
//����˵������
//�������أ���
//======================================================================
void wifi_reset()
{
	wdog_feed();	
	//1.��ʼ��ȫ�ֱ���
	WIFI_RECVSTOP = 0;                     //����ģʽ��־
	WIFI_CMDLENGTH = 0;                    //����س�������
	WIFI_CONNECT_STATE=0;                  //TCP����״̬
	WIFI_AP_STATE=0;                       //Wi-Fi����״̬
	WIFI_SEND_STATE=0;                     //͸��ģʽ�·���ģʽ״̬��־
	WIFI_BOOTFLAG = 0;                     //������ɱ�־
	WIFI_CMD_FLAG = 0;                     //����״̬��־
	uart_init(UART_WIFI,115200);           //��ʼ��ͨ�Ŵ��ڣ�������115200
	uart_enable_re_int(UART_WIFI);         //�򿪴��ڽ����ж�
	//2.��������
	//2.1 ��������ɹ�֡β������ʧ��֡β
	strcpy((char*)WIFI_OK_STOP,"OK");
	strcpy((char*)WIFI_ERR_STOP,"ERROR");
	//2.2 �ر��������
	wdog_feed();	
	wifi_send_cmd((uint8_t*)"AT+UARTE=OFF\r");
	//2.3 �رջط��¼�
	wdog_feed();	
	wifi_send_cmd((uint8_t*)"AT+CIPEVENT=OFF\r");
}

//======================================================================
//�������ƣ�wifi_resolve_domain
//���ܸ�Ҫ������ap�󣬽�����������
//����˵����domain:����,retVal:��������������ŵĻ�����
//�������أ����������Ƿ�ɹ�=0�ɹ���=1����=2�޷�������
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
//�������ƣ�wifi_ping
//���ܸ�Ҫ������ap��pingָ��ip��Ҳ����������
//����˵����address:��Ҫping��Ŀ���ַ,time���pingͨ����ʱ��
//�������أ��Ƿ�pingͨ=0�ɹ���=1����=2�޷�������
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
//�������ƣ�wifi_get_mac
//���ܸ�Ҫ����ȡ����mac
//����˵����mac:���ػ�ȡ��macֵ
//�������أ���ȡ�ɹ�����1��ʧ�ܷ���0
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
//�������ƣ�wifi_int_re
//���ܸ�Ҫ���жϽ����ַ�
//����˵����ch:Ҫ���ܵ��ַ���*dataLen:�洢���յ������ݳ��ȣ�
//      *data:�����յ������ݴ�ŵ���������;
//�������أ���
//======================================================================
void wifi_int_re(uint8_t ch,uint16_t *dataLen, uint8_t *data)
{
	//����ֲ�����
	//1.WIFI_ACKSTOP������ʱ����ָ�����
	switch(WIFI_RECVSTOP)
	{
	    //���ݽ���ģʽ
	    case 0:uecom_recv(ch,dataLen,data);
	    	break;
	    //����ؽ���ģʽ
	    case 1:if(WIFI_CMDLENGTH<400)
               {
 	               wifi_returncheck(ch);
 	               WIFI_CMD[WIFI_CMDLENGTH+1]=0;
               }
	    	   break;
	    //����ģʽ
	    case 2:WIFI_BOOTFLAG = wifi_rebootcheck(ch);
	           if(WIFI_BOOTFLAG)
	        	   WIFI_RECVSTOP = 0;
	           break;                        //�����ȴ�
	    case 3:
	    break;
	}
}

//======================================================================
//�������ƣ�wifi_get_state
//���ܸ�Ҫ����ȡģ������״̬
//����˵������
//�������أ�=0δ����WIFI�ͷ�����,=1������WIFI��δ���ӷ�����,=3������WIFI�ͷ�����
//======================================================================
uint8_t wifi_get_state()
{
	return (WIFI_CONNECT_STATE<<1)|(WIFI_AP_STATE);
}

//======================================================================
//�������ƣ�wifi_linktossid
//���ܸ�Ҫ������ָ��wifi�����
//����˵����ssid:���������,password:����
//�������أ������Ƿ�ɹ�=0�ɹ���=1����=2�޷�������
//======================================================================
uint8_t wifi_linktossid(uint8_t* ssid,uint8_t* password)
{
	//����ֲ�����
	uint8_t cmd[40],flag;
	//2.����֡β
	strcpy((char*)WIFI_OK_STOP,"OK");
	strcpy((char*)WIFI_ERR_STOP,"ERROR");
	strcpy((char*)cmd,"AT+WJAP=");
	strcat((char*)cmd,(char*)ssid);
	strcat((char*)cmd,",");
	strcat((char*)cmd,(char*)password);
	strcat((char*)cmd,"\r");
	//3.����ָ��
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
//�������ƣ�wifi_quitssid
//���ܸ�Ҫ���Ͽ�wifi����������
//����˵������
//�������أ��Ͽ��Ƿ�ɹ�=0�ɹ���=1����=2�޷�������
//======================================================================
uint8_t wifi_quitssid()
{
	strcpy((char*)WIFI_OK_STOP,"OK");
	strcpy((char*)WIFI_ERR_STOP,"ERROR");
	return (wifi_send_cmd((uint8_t*)WIFI_QUIT_AP));
}

//======================================================================
//�������ƣ�wifi_con_srv
//���ܸ�Ҫ�����������������
//����˵����ip:������IP��ַ,port:�˿ںţ���Ϊ�ַ�����ʽ��,
//       ��ʽ"xxx.xxx.xxx.xxx",xxxȡֵ��Χ0-255
//�������أ�=0���ӳɹ�,=1����ʧ��,=2���ڷ���ģʽ
//======================================================================
uint8_t wifi_con_srv(uint8_t* ip,uint8_t* port)
{
	//����ֲ�����
	uint8_t flag,cmd[100];
	//1.���wifiģ�鵱ǰ״̬�Ƿ��ڷ���ģʽ
	if(WIFI_SEND_STATE==1)
		return 2;
	//2.��������ip��ַ�Ƿ�Ϸ�
	if(!wifi_check_ip(ip))
		return WIFI_ERROR;
	//3.��������portֵ�Ƿ�Ϸ�
	if(!wifi_check_port(port))
	    return WIFI_ERROR;
	//4.����֡β
	strcpy((char*)WIFI_OK_STOP,"OK");
	strcpy((char*)WIFI_ERR_STOP,"ERROR");
	//5.�Ͽ���������
	wifi_disconnect();
	//6.ƴ��ָ��

	port[5]=0;
	strcpy((char*)cmd,(char*)WIFI_START);
	strcat((char*)cmd,"1,tcp_client,");
	strcat((char*)cmd,(char*)(ip));
	strcat((char*)cmd,",");
	strcat((char*)cmd,(char*)(port));
	strcat(((char*)cmd),"\r");

	//7.����ָ��
	flag=wifi_send_cmd(cmd);
	return flag;
}

//======================================================================
//�������ƣ�wifi_sendN
//���ܸ�Ҫ�����ӽ����������������N���ַ�,����ǵ�һ�η���,�Ὺ������ģʽ,����
//����ģʽ�󲿷ֺ����᲻����,�����뿴������˵��
//����˵����data:��Ҫ���͵��ַ�,length:���ݳ���,IMSI:������IMSI��
//�������أ������Ƿ�ɹ�=0�ɹ�,=1��������ģʽʧ��
//======================================================================
uint8_t wifi_sendN(uint8_t* data,uint16_t length,uint8_t* IMSI)
{
	uint8_t buf[400], cmd[20], strlength[5];
	if(WIFI_SEND_STATE==0)
	{
		//3.ƴ�ӷ�������֡β
		strcpy((char*)WIFI_OK_STOP,"OK");
		strcpy((char*)WIFI_ERR_STOP,"ERROR");
		WIFI_SEND_STATE=1;
	}

	wifi_frameEncode(IMSI,data,length,buf,&length);
	//5.��������
	itoa(length,(char*)strlength,10);
	strcpy((char*)cmd,"AT+CIPSEND=1,");
	strcat((char*)cmd,(char*)strlength);
	strcat((char*)cmd,"\r");
	uart_send_string(UART_WIFI,(uint8_t*)cmd);
	uart_sendN(UART_WIFI,length,buf);
	return WIFI_OK;
}

//======================================================================
//�������ƣ�wifi_set_ip
//���ܸ�Ҫ������staģʽ��WiFiģ���IP��ַ
//����˵����ip:Ҫ���õ�IP��ַ
//�������أ�=0���óɹ���=1����ʧ�ܣ�=2WiFiģ����Ӧ��
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



//===========================�ڲ�����====================================
//======================================================================
//�������ƣ�wifi_check_port
//���ܸ�Ҫ�����˿ں��Ƿ�Ϸ�
//����˵����port:��Ҫ���Ķ˿ں��ַ���,�˿ںŷ�ΧӦΪ0-65535
//�������أ�=1�Ϸ���=0���Ϸ�
//======================================================================
uint8_t wifi_check_port(uint8_t* port)
{
	//����ֲ�����
	uint8_t length,maxlength;
	uint32_t num;
	maxlength=5;    //�˿��ַ��������5��
	for(length=0,num=0;*port!='\0'&&length<maxlength;length++)
	{
		//������������µ�numֵ�����򷵻�0
		if(*port>='0'&&*port<='9')
			num=num*10+(*port-'0');
		else return 0;
		port++;
	}
	//��������0
	if(length>maxlength)
		return 0;
	//�˿�ֵ������Χ
	if(num>65535)
		return 0;
	return 1;
}

//======================================================================
//�������ƣ�wifi_check_ip
//���ܸ�Ҫ�����ip��ַ�Ƿ�Ϸ�
//����˵����ip:��Ҫ����ip��ַ�ַ���,��ʽ"xxx.xxx.xxx.xxx",xxxȡֵ��Χ0-255
//�������أ�=1�Ϸ���=0���Ϸ�
//======================================================================
uint8_t wifi_check_ip(uint8_t* ip)
{
	//����ֲ�����
	uint8_t i,j,maxlength;           //i��������ӿ�,j������¼�ӿ鳤��
	uint16_t length,num[4];
	//��ʼ���ֲ�����
	for(i=0;i<4;i++)
		num[i]=0;
	maxlength=15;
	//1.��鳤�Ȳ�������ip�ַ����зֳ��Ŀ�
	for(length=0,i=0,j=0;*ip!='\0'&&length<=maxlength;length++)
	{
		if(*ip=='.')
		{
			if(j==0||j>3)
				return 0;
			i++;                     //�ӿ��ʶ��һ
			j=0;                     //�ӿ鳤�����㿪ʼ������һ���ӿ�
		}
		else if(*ip>='0'&&*ip<='9')
		{
			num[i]=num[i]*10+(*ip-'0');
			j++;                     //��ǰ�ӿ鳤��+1
		}
		else return 0;               //��⵽�����ַ���0
		ip++;                        //ת���¸��ַ�
	}
	if(length>maxlength)             //����,����0
		return 0;
	//2.����ӿ��Ƿ�С�ڵ���255
	for(i=0;i<4;i++)
	{
		if(!(num[i]<256))            //��⵽����255���ӿ�,����0
			return 0;
	}
	return 1;                        //��������������,����1
}

//======================================================================
//�������ƣ�wifi_check_mac
//���ܸ�Ҫ�����macֵ�Ƿ�Ϸ�
//����˵����mac:macֵ
//�������أ�=1�Ϸ���=0���Ϸ�
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
//�������ƣ�wifi_disconnect
//���ܸ�Ҫ���Ͽ�tcp��udp����
//����˵������
//�������أ��Ͽ������Ƿ�ɹ�=0�ɹ���=1����=2�޷�������
//======================================================================
uint8_t wifi_disconnect()
{
	//1.ƴ��֡β
	strcpy((char*)WIFI_OK_STOP,"OK");
	strcpy((char*)WIFI_ERR_STOP,"ERROR");
	//2.�Ͽ�����
    return wifi_send_cmd((uint8_t*)WIFI_CLOSE);
}

//======================================================================
//�������ƣ�wifi_mode_set
//���ܸ�Ҫ������WIFIӦ��ģʽ
//����˵����modetype:Ӧ��ģʽ,=1 Stationģʽ
//                      =2 APģʽ
//                      =3 ���ģʽ
//�������أ�ģʽ�����Ƿ�ɹ�=0�ɹ���=1����=2�޷�������
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
//�������ƣ�wifi_send_cmd
//���ܸ�Ҫ������wifiģ��������жϿ�����������ܹ�����WiFiģ��Ӧ������
//����˵����cmdstring:�����ַ���
//�������أ����ͺ�wifiģ��Ӧ��״̬��=0�ɹ���=1����=2��Ӧ��
//======================================================================
uint8_t wifi_send_cmd(uint8_t* cmdstring)
{
	//����ֲ�����
	vuint32_t i;
	//1.��ʼ��ȫ�ֱ���
	WIFI_CMDLENGTH = 0;
	WIFI_CMD_FLAG = 0;
	WIFI_RECVSTOP = WIFI_RECV_CMD;
	//2.�򴮿ڷ���ָ��
	uart_send_string(UART_WIFI,cmdstring);
	i=0;
	//3.�����ɹ�״̬��ʧ��״̬
	while(1)
	{
		wdog_feed();	
		//3.1 �������ɹ�״̬
		if(WIFI_CMD_FLAG == 1)
		{
			WIFI_RECVSTOP = WIFI_RECV_DATA;    //�����жϽ���ģʽΪ���ݽ���ģʽ
			return WIFI_OK;                    //��������ɹ���־WIFI_OK��0��
		}
		//3.2 ������ʧ��״̬
		else if(WIFI_CMD_FLAG == 2)
		{
			WIFI_RECVSTOP = WIFI_RECV_DATA;    //�����жϽ���ģʽΪ���ݽ���ģʽ
			return WIFI_ERROR;                 //��������ʧ�ܱ�־WIFI_ERROR��1��
		}
		i++;                                   //�ƴα�־��һ
		//3.3 ������ʱ
		if(i>0xffffff)
		{
			WIFI_RECVSTOP = WIFI_RECV_DATA;    //�����жϽ���ģʽΪ���ݽ���ģʽ
			return WIFI_NORESPONSE;            //�������ʱ��־WIFI_NORESPONSE��2��
		}
	}
}

//======================================================================
//�������ƣ�wifi_set_send_mode
//���ܸ�Ҫ������ģ�鷢��ģʽ,ʹ��tcp�������ݱ���ʹ��͸��ģʽ
//����˵����mode:����ģʽѡ��=0��͸��ģʽ��=1͸��ģʽ
//         WIFI_NONE_PASS_THROUGH 0
//         WIFI_PASS_THROUGH 1
//�������أ������Ƿ�ɹ�=0����سɹ���=1�����ʧ�ܣ�=2����س�ʱ
//======================================================================
uint8_t wifi_set_send_mode(uint8_t mode)
{
	//����ֲ�����
	uint8_t flag;            //����ر�־
    //1.��������ɹ�֡β������ʧ��֡β
	strcpy((char*)WIFI_OK_STOP,"OK");
	strcpy((char*)WIFI_ERR_STOP,"ERROR");
	//2.�ж�mode��ֵ����Ӧ��ֵ���Ͷ�Ӧ����䣬���mode����0��1�򷵻�����ʧ��
    switch(mode)
	{
		case 0:flag=wifi_send_cmd((uint8_t*)WIFI_CIP_MODE0);break;    //=0�ر�͸��ģʽ
		case 1:flag=wifi_send_cmd((uint8_t*)WIFI_CIP_MODE1);break;    //=1����͸��ģʽ
		default:return WIFI_ERROR;                          //������������ʧ��
	}
	return flag;
}

//======================================================================
//�������ƣ�wifi_set_ap_para
//���ܸ�Ҫ������ģ��APģʽ�µĲ���������SSID�������AP�ŵ�
//����˵����ssid:Ҫ���õ�ssid�ַ���,pssword:Ҫ���õ������ַ�����������64λ����
//         ASCII�ַ�,chn:wifi�ŵ��ţ�1~13��ֻ��1��6��11�����ŵ���������
//�������أ������Ƿ�ɹ�=0�ɹ���=1����=2�޷�������
//======================================================================
uint8_t wifi_set_ap_para(uint8_t* ssid,uint8_t* pssword,uint8_t chn)
{
	uint8_t flag,*cmd;
    //1.��������ɹ�֡β������ʧ��֡β
	strcpy((char*)WIFI_OK_STOP,"OK");
	strcpy((char*)WIFI_ERR_STOP,"ERROR");
    //2.ƴ��ָ��
	cmd=malloc(sizeof(ssid)+sizeof(pssword)+20);
	sprintf((char*)cmd,"%s\"%s\",\"%s\",%d,4\r\n",WIFI_SET_AP,ssid,pssword,chn);
	flag=wifi_send_cmd(cmd);
	free(cmd);
	return flag;
}

//====================================================================
//�������ƣ�wifi_power_ctr
//���ܸ�Ҫ������wifi��Դ
//����˵����state:WIFI_ON(1):�򿪵�Դ,WIFI_OFF(0):�رյ�Դ
//�������أ���
//====================================================================
void wifi_power_ctr(uint8_t state)
{
	if(state==WIFI_ON)
		gpio_set(POWER_PIN,1);
	else
		gpio_set(POWER_PIN,0);
}

//====================================================================
//�������ƣ�wifi_frameEncode
//���ܸ�Ҫ��ʵ�ִ��������ݵ���֡,�����������ݼ���֡ͷ��֡����֡β�Լ�У����Ϣ
//       frame=֡ͷ+IMSI+data����+data+CRCУ����+֡β
//����˵����imsi���������ݵ��豸��IMSI��
//       data:����֡������ͷָ��
//       dlen:����֡�����ݳ���
//       frame:��֮֡��Ĵ���������֡
//       framelen:��֡������ݳ���
//�������أ���
//====================================================================
void wifi_frameEncode(uint8_t *imsi,uint8_t *data,uint16_t dlen,
                 uint8_t *frame,uint16_t *framelen)
{
    uint16_t crc,len;

    //��֡����
    //֡ͷ
    frame[0] = frameHead[0];
    frame[1] = frameHead[1];
    len = 2;
    //IMSI��
    strncpy((char *)(frame+2),(const char *)imsi,15);
    len += 15;
    //data����
    frame[len++] = (uint8_t)(dlen>>8);
    frame[len++] = (uint8_t)dlen;
    //data
    memcpy(frame+19,data,dlen);
    len += dlen;
    //CRCУ����
    crc = crc16(frame+2,dlen+17);//��IMSI�š�data���Ⱥ�data����CRCУ��
    frame[len++] = (uint8_t)(crc>>8);
    frame[len++] = (uint8_t)crc;
    //֡β
    frame[len++] = (uint8_t)frameTail[0];
    frame[len++] = (uint8_t)frameTail[1];
    frame[len] = 0;
    *framelen = len;
}

//====================================================================
//�������ƣ�wifi_returncheck
//�������أ�1����⵽����أ�0:δ��⵽�����
//����˵����ch�����ڽ��յ�������(1�ֽ�)
//���ܸ�Ҫ�����Wi-Fiģ���Ƿ��������
//====================================================================
uint8_t wifi_returncheck(uint8_t ch)
{
    static uint8_t index1 = 0;   //�ɹ�����֡����
    static uint8_t index2 = 0;   //���󷵻�֡����
    static uint8_t len1 = 0;     //�ɹ�����֡����
    static uint8_t len2 = 0;     //���󷵻�֡����
    uint8_t ret_val;
    //�״ν�������ģʽ
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
//�������ƣ�wifi_rebootcheck
//�������أ�1����⵽������ɣ�0:δ��⵽�������
//����˵����ch�����ڽ��յ�������(1�ֽ�)
//���ܸ�Ҫ�����Wi-Fiģ���Ƿ��������
//====================================================================
uint8_t wifi_rebootcheck(uint8_t ch)
{
	//����ֲ�����
    static uint8_t index1 = 0;   //֡����
    static uint8_t bootflag[5] = "ready";   //���������Ϣ֡β
    uint8_t ret_val;
    //����Ĵ���ʹ���˴�����goto�����Ϊ�˾������ٴ����ִ����������֤�ж��ܹ���ʱ��Ӧ
    //1.û������֡β�е�����һ���ַ�
    if(index1 == 0 && ch != bootflag[0])
    {
    	ret_val = 0;
    	goto wifi_rebootcheck_exit;
    }
    //2.�������֡β�ĵ�һ���ַ�
    if(ch==bootflag[0])
    {
    	ret_val = 0;
    	index1 = 1;
    	goto wifi_rebootcheck_exit;
    }
    //3.�������֡β�ĵڶ����ַ�
    if(index1 == 1 && ch == bootflag[1])
    {
    	ret_val = 0;
    	index1 = 2;
    	goto wifi_rebootcheck_exit;
    }
    //4.�������֡β�ĵ������ַ�
    if(index1 == 2 && ch == bootflag[2])
    {
        ret_val = 0;
        index1 = 3;
        goto wifi_rebootcheck_exit;
    }
    //5.�������֡β�ĵ��ĸ��ַ�
    if(index1 == 3 && ch == bootflag[3])
    {
        ret_val = 0;
        index1 = 4;
        goto wifi_rebootcheck_exit;
    }
    //6.�������֡β�ĵ�����ַ�
    if(index1 == 4 && ch == bootflag[4])
    {
        ret_val = 1;                     //֡β�е��ַ�ȫ����⵽
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
//�������ƣ�uecom_recv
//�������أ�0�����յ���ͨ��TCP/UDP���������ݣ�1�����ڽ�֡��2�������ݴ���
//����˵����ch�����ڽ��յ�������(1�ֽ�)��*dataLen:�洢���յ������ݳ��ȣ�
//      *data:�����յ������ݴ�ŵ���������;
//���ܸ�Ҫ���Խ��յ����Է����������ݽ��н�֡��������Ч���ݴ���data����֮�У��ɴ��ڵ�uart�жϵ��á�
//�޸����ڣ���2018-06-30��,LXD
//====================================================================
uint8_t uecom_recv(uint8_t ch,uint16_t *dataLen, uint8_t *data)
{
    static vuint16_t index1 = 0;   //֡����
    static vuint16_t length = 0;  //���ݳ���
    vuint8_t ret_val;
    vuint16_t i;
    vuint8_t WCRC[2];
    vuint16_t wcrc;
    //���δ����֡ͷ����δ�յ�֡ͷ���������(����������֡)
    if((index1 == 0 && ch != frameHead[0]) ||
      (index1 == 1 && ch != frameHead[1]))
    {
    	index1 = 0;
        length = 0;
        ret_val = 2;     //�������ݴ���
        goto uecom_recv_exit;
    }
    //����˵�������Է�����������
    data[index1++] = ch;  //�洢֡����
    if(index1 == 19)      //��ȡdata����
    {
        length = ((uint16_t)data[17]<<8) + data[18];
    }
    //���յ������ݴﵽһ֡���ȡ�length+23Ϊ����֡����
    if(length != 0 && index1 >= length+23)
    {
        //CRCУ��
    	wcrc = crc16(data+2,length+17);
        WCRC[0] = (wcrc>>8)&0xff;
        WCRC[1] = wcrc&0xff;
        if(data[index1-2]!=frameTail[0] ||
        		data[index1-1]!= frameTail[1]  //δ����֡β
            || WCRC[0] != data[length+19] ||
			WCRC[1] != data[length+20])        //CRC�������
        {
        	index1 = 0;
            length = 0;
            ret_val = 2;         //�������ݴ���
            goto uecom_recv_exit;
        }
        for(i=0;i<length;i++)
        {
            data[i] = data[i+19];//19Ϊ��Ч�ֽ�ǰ�����ݳ���
        }
        *dataLen = length;
        index1 = 0;
        length = 0;
        ret_val = 0;             //���յ���ͨ��TCP/UDP����������
        goto uecom_recv_exit;
    }
    *dataLen = 0;
    ret_val = 1;//���ڽ�֡
uecom_recv_exit:
    return ret_val;
}

//=====================================================================
//�������ƣ�crc16
//���ܸ�Ҫ�������ݽ���16λ��CRCУ�飬����У���Ľ��ֵ
//����˵����ptr:��ҪУ������ݻ�����
//      len:��Ҫ��������ݳ���
//�������أ�����õ���У��ֵ?
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

