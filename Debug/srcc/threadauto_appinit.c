#define GLOBLE_VAR
#include "includes.h"

//���̺߳������ᱻ��Ϊ�������̣߳�-----------------------------------
void app_init(void)
{
	//printf��ʾ��
	printf("����ʾ��������Ϊ��RT-Thread��WiFiͨ�ų���\r\n");
	printf("���������ܡ��� �� RT-Thread�����󴴽��˳�ʼ���ͷ��������û��߳�\r\n");
	printf("	        �� ÿ��30s����һ�����ݸ�������\r\n");
	printf("����������������Debug���ڣ�ѡ������Ϊ115200���򿪴��ڣ��鿴������...\r\n");
	printf("\r\nMCU����\r\n");
	
	//��1��������ʵ����Ҫ��ɾ���������̺߳���ʹ�õı���������ʱ��׼��ֵ��
	thread_t thd_init;
	thread_t thd_send;
	
    //��2�������䡿�����ж�
	DISABLE_INTERRUPTS;

	//��3��������ʵ����Ҫ��ɾ����ʼ��ȫ�ֱ������ֲ�����
	
	
    //��4��������ʵ����Ҫ��ɾ�� ��ʼ������ģ��
    
    //��5��������ʵ����Ҫ��ɾ�� ʹ��ģ���ж�
    
    //��6�������䡿�����ж�
	ENABLE_INTERRUPTS;
	
	//��7�������û��̣߳�������غ�����Ϊ�̣߳�    
    thd_init=thread_create("init",   //�߳�����
               (void *)thread_init,   //�߳���ں���
                      0,              //�̲߳���
                     (1024*5),        //�߳�ջ�ռ�
                     11,              //�߳����ȼ�
                     120);            //�߳���ѯ���ȵ�ʱ��Ƭ
    thd_send=thread_create("send", (void *)thread_send, 0, (1024*2), 12, 120);
    
	 //��8�������û��߳�
	thread_startup(thd_init);
	thread_startup(thd_send);
	
	//��9���������߳�
   thread_block();
}
