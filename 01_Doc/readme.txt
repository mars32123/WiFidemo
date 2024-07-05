【20230809】
修改：
1）修改wifi.h和wifi.c新增初始化WIFI基本信息结构体类型、WIFI基本信息函数
2）修改includes.h新增BIOS使用到的端口号变量
3）修改init.c新增调用初始化WIFI基本信息函数并写入倒数第三个扇区供BIOS使用
说明：BIOS程序除端口号和User不同，其它WIFI相关信息（IP地址、用户名、密码）与User保持一致。

【20210602】CRX
修改uecom构件里面的获取时间函数uecom_getTime，之前的函数里加上东八区的时间以后计算有误
【20210517】CRX
将main函数执行的功能划分为五个线程执行
【20210427】CRX
梳理main程序，去除程序中的goto语句


【20210331】
04_GEC
   void **  component_fun;声明移到gec.c
   gec.h修改为extern void **  component_fun;
增加NumToStr构件
原OsFunc.h文件删除，05文件夹增加Os_Self_API.h和Os_United_API.h文件
去除多余的警告
删除gec.h中多余的包含关系
注意02文件夹已经清理，直接复制这边02_CPU替换

【20200225】V6.1 jjl
   03_GEC
       gec.c：增加flash_init初始化步骤，增强flash写入稳定性。
【20210216】
 （1）02_CPU文件夹除cpu.h之外，其他文件BIOS工程与User工程相同，故参照BIOS修改
            移除了mpu_armv7.h、cmsis.h、cmsis_nvic.h文件
（2）user.h文件中对使用的各构件进行包含，故移除gec.h中多余的包含关系
    #include "printf.h"
    #include "flash.h"
    #include "emuart.h"


【20210203】V6.0
一、操作系统启动说明：
 ① 修改前
（1）首先要在gec.c文件中通过Vectors_Init函数继承BIOS中PendSV_Handler与SysTick_Handler
     中断向量，使得在User这边能正常使用操作系统进行调度
（2）在Vectors_Init函数中给component_fun赋值，指向BIOS的接口函数列表，以便User能
     调用BIOS中存在的接口函数
（3）在06文件夹下OsFunc.h文件中对BIOS中操作系统接口函数进行重映射
（4）main中调用OsFunc.h文件中的OS_start函数启动操作系统
 
 ② 修改后
 启动过程不变，只是添加了RTThread_Start宏条件编译，方便切换程序是否带操作系统内核源码，修改说明见下面二。
 
 对于RTThread_Start宏条件，若User程序中无RT-Thread内核源码，从BIOS中启动RT-Thread需要继承
 BIOS中PendSV_Handler与SysTick_Handler中断向量函数，RT-Thread才能正常运行；
 若User程序中有RT-Thread内核源码，此时再继承BIOS中对应中断向量，会替换掉本该需要的User自身的PendSV_Handler
 与SysTick_Handler中断向量函数，使得程序无法正常运转。
 因此需要作出区分，故在OsFunc.h文件中添加RTThread_Start宏条件：
     0=从BIOS启动RT-Thread,User程序中无RT-Thread内核源码；
              非0=从User启动RT-Thread,User程序中包含RT-Thread内核源码。
       
二、程序修改说明：       
 ① 不包含RT-Thread内核源码程序修改说明
（1）更新OsFunc.h文件中所有RT-Thread函数接口为系统原函数接口
（2）OsFunc.h文件中添加操作系统所有结构类型定义，添加RTThread_Start宏条件定义
（3）gec文件添加RTThread_Start宏条件编译，方便切换选择从BIOS启动还是从User启动RT-Thread
    #if (RTThread_Start==0) 
        user[BIOS_SVC_IRQn]=(uint32_t)bios[BIOS_SVC_IRQn];
		user[BIOS_PendSV_Handler]=(uint32_t)bios[BIOS_PendSV_Handler];
		user[BIOS_SysTick_Handler]=(uint32_t)bios[BIOS_SysTick_Handler];
    #endif
（4）切换需要从包含RT-Thread内核源码的User程序中复制06文件下RT-Thread_Src文件进行替换：
     从BIOS启动RT-Thread：只需OsFunc.h
     从User启动RT-Thread：只需RT-Thread_Src文件

    
     
 ② 包含RT-Thread内核源码程序修改说明
（1）增加RTOS启动入口文件OsFunc.c与OsFunc.h
（2）07文件夹中main函数修改为：
         OS_start(app_init);  //启动RTOS并执行主线程app_init
（3）07文件夹中includes.h文件注释#include "rtthread.h"，添加#include "OsFunc.h"
（4）03_MCU文件夹中启动文件“../xxx.s”修改“bl entry”为“bl	main”
（5）..src/components.c文件中
           注释原有rtthread_startup函数、rt_application_init函数、entry函数
 (6)OsFunc.c与OsFunc.h文件、02_CPU文件夹下context_gcc.S、与原有RT-Thread源代码08_RT-Thread文件夹全部
    移到06_SoftComponent文件夹RT-Thread_Src下
（7）修改gec.c，添加
    #if (RTThread_Start==0)        
        user[BIOS_SVC_IRQn]=(uint32_t)bios[BIOS_SVC_IRQn];
		user[BIOS_PendSV_Handler]=(uint32_t)bios[BIOS_PendSV_Handler];
		user[BIOS_SysTick_Handler]=(uint32_t)bios[BIOS_SysTick_Handler];
    #endif 
    gec.h文件中添加
    #define BIOS_SVC_IRQn            (11)
    #define BIOS_PendSV_Handler      (14)
    #define BIOS_SysTick_Handler     (15)
（9）09文件夹内容移到07文件夹，修改07文件夹为07_AppPrg 


三、把User程序改为直接从零启动程序
   （1）从包含RT-Thread内核源码的User程序中复制06文件下RT-Thread_Src文件替换掉OsFunc.h文件
   （2）03_MCU文件夹中的.ld中，
        GEC_USER_SECTOR_START  =0;    /*USER程序开始扇区号*/
   （3）04_GEC文件夹中gec.h文件
         #define GEC_USER_SECTOR_START    (0)  
   （4）若使用到printf函数，则添加对printf需要的串口的初始化  
            uart_init(UART_Debug,115200);
    这样的程序，删除Debug,重新编译后，使用SWD写入器下载，直接运行，不需要BIOS
    
    

---【程序说明】---
（1）程序摘要：基于BIOS的STM32L431工程
（2）运行芯片：本工程基于BIOS运行,不从0启动
（3）软件环境：AHL-GEC-IDE，兼容STM32cubeIDE
（4）硬件环境：AHL-STM32L431，实质是只要是STM32L431即可，
              接AHL-GEC扩展板，有三色灯指示
（5）硬件连接：见"..\05_UserBoard\user"文件
（6）功能简述：
              ① 红灯每5秒闪烁一次，绿灯每10秒闪烁一次，蓝灯每20秒闪烁一次
              ② 连接Debug串口，可以查看红绿蓝三色灯亮暗提示
              



备份原函数接口
//（2）RT-Thread相关函数声明===================================================
//#define OS_start  ((void (*)(void (*func)(void)))(component_fun[34]))
//#define thread_create  ((uint8_t (*)(const char *name,void (*entry)(void *parameter),\
//                                          void *parameter,uint32_t stack_size,uint8_t priority,\
//                                                 uint32_t tick))(component_fun[35]))
//#define thread_start  ((void (*)(uint8_t threadIndex))(component_fun[36]))
//#define thread_wait  ((void (*)(uint32_t millisec))(component_fun[37]))
//#define heap_init  ((void (*)(uint32_t userHeapBase))(component_fun[33]))
//#define get_thread_self  ((uint32_t (*)(void))(component_fun[38]))
////事件
//#define event_create  ((uint8_t (*)(const char *name, uint8_t flag))(component_fun[41]))
//#define event_recv  ((void (*)(uint8_t EventIndex, uint32_t set, uint8_t option,\
//                                     uint32_t timeout,uint32_t *recved))(component_fun[42]))
//#define event_send  ((void (*)(uint8_t EventIndex,uint32_t set))(component_fun[43]))
////信号量
//#define rt_sem_create  ((uint8_t (*)(const char *name, uint32_t value, uint8_t flag))(component_fun[46]))
//#define rt_sem_take  ((void (*)(uint8_t SpIndex, uint32_t time))(component_fun[47]))
//#define rt_sem_release  ((void (*)(uint8_t SpIndex))(component_fun[48]))
//#define sem_getNum  ((uint8_t (*)(uint8_t SpIndex))(component_fun[49]))
////互斥量
//#define rt_mutex_create  ((uint8_t (*)(const char *name,uint8_t flag))(component_fun[52]))
//#define rt_mutex_take  ((void (*)(uint8_t MutexIndex, uint32_t time))(component_fun[53]))
//#define rt_mutex_release  ((void (*)(uint8_t MutexIndex))(component_fun[54]))
////消息队列
//#define rt_mq_create  ((uint8_t (*)(const char *name,uint32_t msg_size,\
//                                            uint32_t max_msgs,uint8_t flag))(component_fun[57]))
//#define rt_mq_send  ((uint32_t (*)(uint8_t MqIndex, void *buffer, uint32_t size))(component_fun[58]))
//#define rt_mq_recv  ((uint8_t (*)(uint8_t MqIndex, void *buffer,\
//                                            uint32_t size,uint32_t timeout))(component_fun[59]))
//#define rt_malloc  ((char* (*)(uint32_t size))(component_fun[60]))
//#define rt_sprintf  ((void (*)(char *buf, const char *format, ...))(component_fun[61]))
//#define mq_getIndex  ((uint8_t (*)(uint8_t MqIndex))(component_fun[62]))
////系统时间函数
//#define rt_tick_get ((uint32_t (*)(void))(component_fun[63]))



【20201115】V3.8 lq
1.flash构件更新。

【20201027】V3.7 lq cc
1.flash构件更新。
2.构件函数列表宏定义相应BIOS做更新。
3.版本号MCU型号更新。

---【程序说明】---
（1）程序摘要：基于BIOS的STM32L431工程
（2）运行芯片：本工程基于BIOS运行,不从0启动
（3）软件环境：AHL-GEC-IDE，兼容STM32cubeIDE
（4）硬件环境：AHL-STM32L431，实质是只要是STM32L431即可，
              接AHL-GEC扩展板，有三色灯指示
（5）硬件连接：见"..\05_UserBoard\user"文件
（6）功能简述：
              ① 红灯每5秒闪烁一次，绿灯每10秒闪烁一次，蓝灯每20秒闪烁一次
              ② 连接Debug串口，可以查看红绿蓝三色灯亮暗提示


【20200915】叶柯阳改
本工程与未驻留的User工程相比
1.做了以下修改
（1）STM32L431RCTX_FLASH.ld文件
更改USER程序开始扇区号为“GEC_USER_SECTOR_START =31;”
（2）startup_stm32l431rctx.s文件
更改语句“bl entry”为“bl main”
（2）gec.h文件
①修改
更改USER程序开始扇区号为“#define GEC_USER_SECTOR_START    (31)”
更改函数列表开始扇区号为“#define GEC_COMPONENT_LST_START  (24)”
更改动态命令开始扇区号为“#define GEC_DYNAMIC_START        (22)”
更改动态命令结束扇区号为“#define GEC_DYNAMIC_END	      (23)”
②新增
在“GEC基本信息”下方新增语句：
#define BIOS_SVC_IRQn            (11)
#define BIOS_PendSV_Handler      (14)
#define BIOS_SysTick_Handler     (15)
在文件最下方新增RT-Thread相关映射函数映射地址声明：
//【变动】RT-Thread相关函数声明===================================================
#define OS_start  ((void (*)(void (*func)(void)))(component_fun[34]))
#define thread_create  ((uint8_t (*)(const char *name,void (*entry)(void *parameter),void *parameter,uint32_t stack_size,uint8_t priority,uint32_t tick))(component_fun[35]))
#define thread_start  ((void (*)(uint8_t threadIndex))(component_fun[36]))
#define thread_wait  ((void (*)(uint32_t millisec))(component_fun[37]))
#define heap_init  ((void (*)(uint32_t userHeapBase))(component_fun[33]))
（3）gec.c文件
在“//改USER中断向量表的BIOS程序写入串口的中断处理程序地址”下方新增语句：
user[BIOS_SVC_IRQn]=(uint32_t)bios[BIOS_SVC_IRQn];
user[BIOS_PendSV_Handler]=(uint32_t)bios[BIOS_PendSV_Handler];
user[BIOS_SysTick_Handler]=(uint32_t)bios[BIOS_SysTick_Handler];
（4）includes.h文件
在语句“//（在此增加全局变量）”下方增加变量
G_VAR_PREFIX uint8_t gthd_redlight;
G_VAR_PREFIX uint8_t gthd_greenlight;
G_VAR_PREFIX uint8_t gthd_bluelight;
（5）main.c文件
完全修改，具体内容包括RT-Thread启动，红灯绿灯蓝灯线程创建和运行
（6）删除08_RT-Thread文件夹及其内部包含的所有文件
（7）删除09_RT-ThreadPrg文件夹及其内部包含的所有文件

==================================================================================
工程名称：User_RT-Thread_Resident_STM32
版权所有：SUDA-ARM(sumcu.suda.edu.cn)
版本更新：2020-09-15
程序介绍：本程序为基于STM32L431RC微控制器的RT-Thread驻留的User端程序。
本程序实现了：（1）在User端调用BIOS程序中的RT-Thread相关函数，实现RT-Thread的启动和线程创建
			 （2）创建了红灯、绿灯和蓝灯三个用户线程。
             （2）实现红灯每5秒闪烁一次，绿灯每10秒闪烁一次，蓝灯每20秒闪烁一次。
==================================================================================
2. 软件环境
(1)AHL-GEC-IDE 4.11
3. 硬件环境
核心板：金葫芦系列STM32L432RC核心板  
UART2接线： microUSB串口线

---【何把User程序改为直接从零启动程序（只有两处）】---
   （1）03_MCU文件夹中的.ld中，
        GEC_USER_SECTOR_START  =0;    /*USER程序开始扇区号*/
   （2）04_GEC文件夹中gec.h文件
         #define GEC_USER_SECTOR_START    (0)  
    这样的程序，删除Debug,重新编译后，使用SWD写入器下载，直接运行，不需要BIOS
         


---移植规则---【20200806】
不同芯片BIOS及User框架基本规则
02_CPU 该文件夹除cpu.h之外，其他文件BIOS工程与User工程同
       cpu.h 变动（1）～（2），根据MCU型号变动
03_MCU mcu.h 变动（1）～（3）,根据MCU型号变动，且BIOS工程与User工程同
             在该文件中修改MCU型号及BIOS版本号
       .ld文件：根据MCU型号变动,且BIOS工程与User工程不同
       .s文件：改动类似BSS处，且BIOS工程与User工程不同
04_GEC 各芯片BIOS工程一致，
       各芯片User工程一致，但同芯片BIOS与User工程的04_GEC
       内容不同
05_UserBoard BIOS中除去User.h根据MCU型号变动，其他文件相同
06、07文件夹：各芯片BIOS相同，同功能的User相同


---版本历史---

【20200830】 V3.5  规范程序，规范对于User模板，适应RTOS
【20200829】 V3.4  复位后重置动态命令
【20200812】修改.s文件，缺省中断，直接返回
【20200630】（1）获取对应芯片的官方工程样例。
            （2）参照本BIOS标准工程，整理工程文件夹结构。
                 01_Doc：存放makefile文件和其他说明文档
                 02_CPU：存放内核头文件。
                         复制本工程中的cpu.h，根据可移植性标识
                         对【变动】部分加以改动，使之适应目标芯片。
                 03_MCU：存放链接文件、启动文件和基本驱动（gpio、uart、flash）。
                         复制本工程中的mcu.h，根据可移植性标识对【变动】部分加以改动，
                         使之适应目标芯片。
                         链接文件的修改：复制本工程链接文件的第（1）部分并加以改动，
                         使之适应目标芯片。Flash字段中各分段的赋值方式直接拷贝本工
                         程链接文件的第（2）部分。Section字段将本工程的第（3）部分
                         中头部的【固定】部分复制并添加至头部。
                         参照标准工程链接文件将存储空间分为对应的分段。
                         其中BIOS分段的大小在编译后根据程序大小，取最小的值。
                         动态命令分段、构建函数列表分段均取4KB大小左右，视存储空间大小，
                         构建函数代码分段可取10KB及以上。
                         在对应的SECTIONS断中同样对各个分段给出对应的定义。
                 04_GEC、05_UserBoard、06_SoftComponent、07_NosPrg直接拷贝。
                         对gec.h和user.h中【变动】部分加以改动，使之适应目标芯片
【20200526】V3.0 （1）MCU_TYPE改为3.0；（2）删除mcu.h中无用定义；
                 （3）修改类型转换构件名及内部函数名