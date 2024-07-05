#define GLOBLE_VAR
#include "includes.h"

//主线程函数（会被变为自启动线程）-----------------------------------
void app_init(void)
{
	//printf提示区
	printf("【提示】本程序为带RT-Thread的WiFi通信程序\r\n");
	printf("【基本功能】① 在 RT-Thread启动后创建了初始化和发送两个用户线程\r\n");
	printf("	        ② 每隔30s发送一次数据给服务器\r\n");
	printf("【操作方法】连接Debug串口，选择波特率为115200，打开串口，查看输出结果...\r\n");
	printf("\r\nMCU启动\r\n");
	
	//（1）【根据实际需要增删】声明主线程函数使用的变量（声明时不准赋值）
	thread_t thd_init;
	thread_t thd_send;
	
    //（2）【不变】关总中断
	DISABLE_INTERRUPTS;

	//（3）【根据实际需要增删】初始化全局变量及局部变量
	
	
    //（4）【根据实际需要增删】 初始化外设模块
    
    //（5）【根据实际需要增删】 使能模块中断
    
    //（6）【不变】开总中断
	ENABLE_INTERRUPTS;
	
	//（7）创建用户线程（即把相关函数变为线程）    
    thd_init=thread_create("init",   //线程名称
               (void *)thread_init,   //线程入口函数
                      0,              //线程参数
                     (1024*5),        //线程栈空间
                     11,              //线程优先级
                     120);            //线程轮询调度的时间片
    thd_send=thread_create("send", (void *)thread_send, 0, (1024*2), 12, 120);
    
	 //（8）启动用户线程
	thread_startup(thd_init);
	thread_startup(thd_send);
	
	//（9）阻塞本线程
   thread_block();
}
