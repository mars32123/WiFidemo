��20230809��
�޸ģ�
1���޸�wifi.h��wifi.c������ʼ��WIFI������Ϣ�ṹ�����͡�WIFI������Ϣ����
2���޸�includes.h����BIOSʹ�õ��Ķ˿ںű���
3���޸�init.c�������ó�ʼ��WIFI������Ϣ������д�뵹��������������BIOSʹ��
˵����BIOS������˿ںź�User��ͬ������WIFI�����Ϣ��IP��ַ���û��������룩��User����һ�¡�

��20210602��CRX
�޸�uecom��������Ļ�ȡʱ�亯��uecom_getTime��֮ǰ�ĺ�������϶�������ʱ���Ժ��������
��20210517��CRX
��main����ִ�еĹ��ܻ���Ϊ����߳�ִ��
��20210427��CRX
����main����ȥ�������е�goto���


��20210331��
04_GEC
   void **  component_fun;�����Ƶ�gec.c
   gec.h�޸�Ϊextern void **  component_fun;
����NumToStr����
ԭOsFunc.h�ļ�ɾ����05�ļ�������Os_Self_API.h��Os_United_API.h�ļ�
ȥ������ľ���
ɾ��gec.h�ж���İ�����ϵ
ע��02�ļ����Ѿ�����ֱ�Ӹ������02_CPU�滻

��20200225��V6.1 jjl
   03_GEC
       gec.c������flash_init��ʼ�����裬��ǿflashд���ȶ��ԡ�
��20210216��
 ��1��02_CPU�ļ��г�cpu.h֮�⣬�����ļ�BIOS������User������ͬ���ʲ���BIOS�޸�
            �Ƴ���mpu_armv7.h��cmsis.h��cmsis_nvic.h�ļ�
��2��user.h�ļ��ж�ʹ�õĸ��������а��������Ƴ�gec.h�ж���İ�����ϵ
    #include "printf.h"
    #include "flash.h"
    #include "emuart.h"


��20210203��V6.0
һ������ϵͳ����˵����
 �� �޸�ǰ
��1������Ҫ��gec.c�ļ���ͨ��Vectors_Init�����̳�BIOS��PendSV_Handler��SysTick_Handler
     �ж�������ʹ����User���������ʹ�ò���ϵͳ���е���
��2����Vectors_Init�����и�component_fun��ֵ��ָ��BIOS�Ľӿں����б��Ա�User��
     ����BIOS�д��ڵĽӿں���
��3����06�ļ�����OsFunc.h�ļ��ж�BIOS�в���ϵͳ�ӿں���������ӳ��
��4��main�е���OsFunc.h�ļ��е�OS_start������������ϵͳ
 
 �� �޸ĺ�
 �������̲��䣬ֻ�������RTThread_Start���������룬�����л������Ƿ������ϵͳ�ں�Դ�룬�޸�˵�����������
 
 ����RTThread_Start����������User��������RT-Thread�ں�Դ�룬��BIOS������RT-Thread��Ҫ�̳�
 BIOS��PendSV_Handler��SysTick_Handler�ж�����������RT-Thread�����������У�
 ��User��������RT-Thread�ں�Դ�룬��ʱ�ټ̳�BIOS�ж�Ӧ�ж����������滻��������Ҫ��User�����PendSV_Handler
 ��SysTick_Handler�ж�����������ʹ�ó����޷�������ת��
 �����Ҫ�������֣�����OsFunc.h�ļ������RTThread_Start��������
     0=��BIOS����RT-Thread,User��������RT-Thread�ں�Դ�룻
              ��0=��User����RT-Thread,User�����а���RT-Thread�ں�Դ�롣
       
���������޸�˵����       
 �� ������RT-Thread�ں�Դ������޸�˵��
��1������OsFunc.h�ļ�������RT-Thread�����ӿ�Ϊϵͳԭ�����ӿ�
��2��OsFunc.h�ļ�����Ӳ���ϵͳ���нṹ���Ͷ��壬���RTThread_Start����������
��3��gec�ļ����RTThread_Start���������룬�����л�ѡ���BIOS�������Ǵ�User����RT-Thread
    #if (RTThread_Start==0) 
        user[BIOS_SVC_IRQn]=(uint32_t)bios[BIOS_SVC_IRQn];
		user[BIOS_PendSV_Handler]=(uint32_t)bios[BIOS_PendSV_Handler];
		user[BIOS_SysTick_Handler]=(uint32_t)bios[BIOS_SysTick_Handler];
    #endif
��4���л���Ҫ�Ӱ���RT-Thread�ں�Դ���User�����и���06�ļ���RT-Thread_Src�ļ������滻��
     ��BIOS����RT-Thread��ֻ��OsFunc.h
     ��User����RT-Thread��ֻ��RT-Thread_Src�ļ�

    
     
 �� ����RT-Thread�ں�Դ������޸�˵��
��1������RTOS��������ļ�OsFunc.c��OsFunc.h
��2��07�ļ�����main�����޸�Ϊ��
         OS_start(app_init);  //����RTOS��ִ�����߳�app_init
��3��07�ļ�����includes.h�ļ�ע��#include "rtthread.h"�����#include "OsFunc.h"
��4��03_MCU�ļ����������ļ���../xxx.s���޸ġ�bl entry��Ϊ��bl	main��
��5��..src/components.c�ļ���
           ע��ԭ��rtthread_startup������rt_application_init������entry����
 (6)OsFunc.c��OsFunc.h�ļ���02_CPU�ļ�����context_gcc.S����ԭ��RT-ThreadԴ����08_RT-Thread�ļ���ȫ��
    �Ƶ�06_SoftComponent�ļ���RT-Thread_Src��
��7���޸�gec.c�����
    #if (RTThread_Start==0)        
        user[BIOS_SVC_IRQn]=(uint32_t)bios[BIOS_SVC_IRQn];
		user[BIOS_PendSV_Handler]=(uint32_t)bios[BIOS_PendSV_Handler];
		user[BIOS_SysTick_Handler]=(uint32_t)bios[BIOS_SysTick_Handler];
    #endif 
    gec.h�ļ������
    #define BIOS_SVC_IRQn            (11)
    #define BIOS_PendSV_Handler      (14)
    #define BIOS_SysTick_Handler     (15)
��9��09�ļ��������Ƶ�07�ļ��У��޸�07�ļ���Ϊ07_AppPrg 


������User�����Ϊֱ�Ӵ�����������
   ��1���Ӱ���RT-Thread�ں�Դ���User�����и���06�ļ���RT-Thread_Src�ļ��滻��OsFunc.h�ļ�
   ��2��03_MCU�ļ����е�.ld�У�
        GEC_USER_SECTOR_START  =0;    /*USER����ʼ������*/
   ��3��04_GEC�ļ�����gec.h�ļ�
         #define GEC_USER_SECTOR_START    (0)  
   ��4����ʹ�õ�printf����������Ӷ�printf��Ҫ�Ĵ��ڵĳ�ʼ��  
            uart_init(UART_Debug,115200);
    �����ĳ���ɾ��Debug,���±����ʹ��SWDд�������أ�ֱ�����У�����ҪBIOS
    
    

---������˵����---
��1������ժҪ������BIOS��STM32L431����
��2������оƬ�������̻���BIOS����,����0����
��3�����������AHL-GEC-IDE������STM32cubeIDE
��4��Ӳ��������AHL-STM32L431��ʵ����ֻҪ��STM32L431���ɣ�
              ��AHL-GEC��չ�壬����ɫ��ָʾ
��5��Ӳ�����ӣ���"..\05_UserBoard\user"�ļ�
��6�����ܼ�����
              �� ���ÿ5����˸һ�Σ��̵�ÿ10����˸һ�Σ�����ÿ20����˸һ��
              �� ����Debug���ڣ����Բ鿴��������ɫ��������ʾ
              



����ԭ�����ӿ�
//��2��RT-Thread��غ�������===================================================
//#define OS_start  ((void (*)(void (*func)(void)))(component_fun[34]))
//#define thread_create  ((uint8_t (*)(const char *name,void (*entry)(void *parameter),\
//                                          void *parameter,uint32_t stack_size,uint8_t priority,\
//                                                 uint32_t tick))(component_fun[35]))
//#define thread_start  ((void (*)(uint8_t threadIndex))(component_fun[36]))
//#define thread_wait  ((void (*)(uint32_t millisec))(component_fun[37]))
//#define heap_init  ((void (*)(uint32_t userHeapBase))(component_fun[33]))
//#define get_thread_self  ((uint32_t (*)(void))(component_fun[38]))
////�¼�
//#define event_create  ((uint8_t (*)(const char *name, uint8_t flag))(component_fun[41]))
//#define event_recv  ((void (*)(uint8_t EventIndex, uint32_t set, uint8_t option,\
//                                     uint32_t timeout,uint32_t *recved))(component_fun[42]))
//#define event_send  ((void (*)(uint8_t EventIndex,uint32_t set))(component_fun[43]))
////�ź���
//#define rt_sem_create  ((uint8_t (*)(const char *name, uint32_t value, uint8_t flag))(component_fun[46]))
//#define rt_sem_take  ((void (*)(uint8_t SpIndex, uint32_t time))(component_fun[47]))
//#define rt_sem_release  ((void (*)(uint8_t SpIndex))(component_fun[48]))
//#define sem_getNum  ((uint8_t (*)(uint8_t SpIndex))(component_fun[49]))
////������
//#define rt_mutex_create  ((uint8_t (*)(const char *name,uint8_t flag))(component_fun[52]))
//#define rt_mutex_take  ((void (*)(uint8_t MutexIndex, uint32_t time))(component_fun[53]))
//#define rt_mutex_release  ((void (*)(uint8_t MutexIndex))(component_fun[54]))
////��Ϣ����
//#define rt_mq_create  ((uint8_t (*)(const char *name,uint32_t msg_size,\
//                                            uint32_t max_msgs,uint8_t flag))(component_fun[57]))
//#define rt_mq_send  ((uint32_t (*)(uint8_t MqIndex, void *buffer, uint32_t size))(component_fun[58]))
//#define rt_mq_recv  ((uint8_t (*)(uint8_t MqIndex, void *buffer,\
//                                            uint32_t size,uint32_t timeout))(component_fun[59]))
//#define rt_malloc  ((char* (*)(uint32_t size))(component_fun[60]))
//#define rt_sprintf  ((void (*)(char *buf, const char *format, ...))(component_fun[61]))
//#define mq_getIndex  ((uint8_t (*)(uint8_t MqIndex))(component_fun[62]))
////ϵͳʱ�亯��
//#define rt_tick_get ((uint32_t (*)(void))(component_fun[63]))



��20201115��V3.8 lq
1.flash�������¡�

��20201027��V3.7 lq cc
1.flash�������¡�
2.���������б�궨����ӦBIOS�����¡�
3.�汾��MCU�ͺŸ��¡�

---������˵����---
��1������ժҪ������BIOS��STM32L431����
��2������оƬ�������̻���BIOS����,����0����
��3�����������AHL-GEC-IDE������STM32cubeIDE
��4��Ӳ��������AHL-STM32L431��ʵ����ֻҪ��STM32L431���ɣ�
              ��AHL-GEC��չ�壬����ɫ��ָʾ
��5��Ӳ�����ӣ���"..\05_UserBoard\user"�ļ�
��6�����ܼ�����
              �� ���ÿ5����˸һ�Σ��̵�ÿ10����˸һ�Σ�����ÿ20����˸һ��
              �� ����Debug���ڣ����Բ鿴��������ɫ��������ʾ


��20200915��Ҷ������
��������δפ����User�������
1.���������޸�
��1��STM32L431RCTX_FLASH.ld�ļ�
����USER����ʼ������Ϊ��GEC_USER_SECTOR_START =31;��
��2��startup_stm32l431rctx.s�ļ�
������䡰bl entry��Ϊ��bl main��
��2��gec.h�ļ�
���޸�
����USER����ʼ������Ϊ��#define GEC_USER_SECTOR_START    (31)��
���ĺ����б�ʼ������Ϊ��#define GEC_COMPONENT_LST_START  (24)��
���Ķ�̬���ʼ������Ϊ��#define GEC_DYNAMIC_START        (22)��
���Ķ�̬�������������Ϊ��#define GEC_DYNAMIC_END	      (23)��
������
�ڡ�GEC������Ϣ���·�������䣺
#define BIOS_SVC_IRQn            (11)
#define BIOS_PendSV_Handler      (14)
#define BIOS_SysTick_Handler     (15)
���ļ����·�����RT-Thread���ӳ�亯��ӳ���ַ������
//���䶯��RT-Thread��غ�������===================================================
#define OS_start  ((void (*)(void (*func)(void)))(component_fun[34]))
#define thread_create  ((uint8_t (*)(const char *name,void (*entry)(void *parameter),void *parameter,uint32_t stack_size,uint8_t priority,uint32_t tick))(component_fun[35]))
#define thread_start  ((void (*)(uint8_t threadIndex))(component_fun[36]))
#define thread_wait  ((void (*)(uint32_t millisec))(component_fun[37]))
#define heap_init  ((void (*)(uint32_t userHeapBase))(component_fun[33]))
��3��gec.c�ļ�
�ڡ�//��USER�ж��������BIOS����д�봮�ڵ��жϴ�������ַ���·�������䣺
user[BIOS_SVC_IRQn]=(uint32_t)bios[BIOS_SVC_IRQn];
user[BIOS_PendSV_Handler]=(uint32_t)bios[BIOS_PendSV_Handler];
user[BIOS_SysTick_Handler]=(uint32_t)bios[BIOS_SysTick_Handler];
��4��includes.h�ļ�
����䡰//���ڴ�����ȫ�ֱ��������·����ӱ���
G_VAR_PREFIX uint8_t gthd_redlight;
G_VAR_PREFIX uint8_t gthd_greenlight;
G_VAR_PREFIX uint8_t gthd_bluelight;
��5��main.c�ļ�
��ȫ�޸ģ��������ݰ���RT-Thread����������̵������̴߳���������
��6��ɾ��08_RT-Thread�ļ��м����ڲ������������ļ�
��7��ɾ��09_RT-ThreadPrg�ļ��м����ڲ������������ļ�

==================================================================================
�������ƣ�User_RT-Thread_Resident_STM32
��Ȩ���У�SUDA-ARM(sumcu.suda.edu.cn)
�汾���£�2020-09-15
������ܣ�������Ϊ����STM32L431RC΢��������RT-Threadפ����User�˳���
������ʵ���ˣ���1����User�˵���BIOS�����е�RT-Thread��غ�����ʵ��RT-Thread���������̴߳���
			 ��2�������˺�ơ��̵ƺ����������û��̡߳�
             ��2��ʵ�ֺ��ÿ5����˸һ�Σ��̵�ÿ10����˸һ�Σ�����ÿ20����˸һ�Ρ�
==================================================================================
2. �������
(1)AHL-GEC-IDE 4.11
3. Ӳ������
���İ壺���«ϵ��STM32L432RC���İ�  
UART2���ߣ� microUSB������

---���ΰ�User�����Ϊֱ�Ӵ�����������ֻ����������---
   ��1��03_MCU�ļ����е�.ld�У�
        GEC_USER_SECTOR_START  =0;    /*USER����ʼ������*/
   ��2��04_GEC�ļ�����gec.h�ļ�
         #define GEC_USER_SECTOR_START    (0)  
    �����ĳ���ɾ��Debug,���±����ʹ��SWDд�������أ�ֱ�����У�����ҪBIOS
         


---��ֲ����---��20200806��
��ͬоƬBIOS��User��ܻ�������
02_CPU ���ļ��г�cpu.h֮�⣬�����ļ�BIOS������User����ͬ
       cpu.h �䶯��1������2��������MCU�ͺű䶯
03_MCU mcu.h �䶯��1������3��,����MCU�ͺű䶯����BIOS������User����ͬ
             �ڸ��ļ����޸�MCU�ͺż�BIOS�汾��
       .ld�ļ�������MCU�ͺű䶯,��BIOS������User���̲�ͬ
       .s�ļ����Ķ�����BSS������BIOS������User���̲�ͬ
04_GEC ��оƬBIOS����һ�£�
       ��оƬUser����һ�£���ͬоƬBIOS��User���̵�04_GEC
       ���ݲ�ͬ
05_UserBoard BIOS�г�ȥUser.h����MCU�ͺű䶯�������ļ���ͬ
06��07�ļ��У���оƬBIOS��ͬ��ͬ���ܵ�User��ͬ


---�汾��ʷ---

��20200830�� V3.5  �淶���򣬹淶����Userģ�壬��ӦRTOS
��20200829�� V3.4  ��λ�����ö�̬����
��20200812���޸�.s�ļ���ȱʡ�жϣ�ֱ�ӷ���
��20200630����1����ȡ��ӦоƬ�Ĺٷ�����������
            ��2�����ձ�BIOS��׼���̣��������ļ��нṹ��
                 01_Doc�����makefile�ļ�������˵���ĵ�
                 02_CPU������ں�ͷ�ļ���
                         ���Ʊ������е�cpu.h�����ݿ���ֲ�Ա�ʶ
                         �ԡ��䶯�����ּ��ԸĶ���ʹ֮��ӦĿ��оƬ��
                 03_MCU����������ļ��������ļ��ͻ���������gpio��uart��flash����
                         ���Ʊ������е�mcu.h�����ݿ���ֲ�Ա�ʶ�ԡ��䶯�����ּ��ԸĶ���
                         ʹ֮��ӦĿ��оƬ��
                         �����ļ����޸ģ����Ʊ����������ļ��ĵڣ�1�����ֲ����ԸĶ���
                         ʹ֮��ӦĿ��оƬ��Flash�ֶ��и��ֶεĸ�ֵ��ʽֱ�ӿ�������
                         �������ļ��ĵڣ�2�����֡�Section�ֶν������̵ĵڣ�3������
                         ��ͷ���ġ��̶������ָ��Ʋ������ͷ����
                         ���ձ�׼���������ļ����洢�ռ��Ϊ��Ӧ�ķֶΡ�
                         ����BIOS�ֶεĴ�С�ڱ������ݳ����С��ȡ��С��ֵ��
                         ��̬����ֶΡ����������б�ֶξ�ȡ4KB��С���ң��Ӵ洢�ռ��С��
                         ������������ֶο�ȡ10KB�����ϡ�
                         �ڶ�Ӧ��SECTIONS����ͬ���Ը����ֶθ�����Ӧ�Ķ��塣
                 04_GEC��05_UserBoard��06_SoftComponent��07_NosPrgֱ�ӿ�����
                         ��gec.h��user.h�С��䶯�����ּ��ԸĶ���ʹ֮��ӦĿ��оƬ
��20200526��V3.0 ��1��MCU_TYPE��Ϊ3.0����2��ɾ��mcu.h�����ö��壻
                 ��3���޸�����ת�����������ڲ�������