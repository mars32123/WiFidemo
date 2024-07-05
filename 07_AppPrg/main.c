//======================================================================
//文件名称：main.c（应用工程主函数）
//框架提供：SD-Arm（sumcu.suda.edu.cn）
//版本更新：2017.08:1.0, 2019.1:A.10
//功能描述：见本工程的..\01_Doc\Readme.txt
//移植规则：【固定】
//======================================================================
#define GLOBLE_VAR
#include "includes.h"      //包含总头文件
int main(void)
{
    OS_start(app_init);  //启动RTOS并执行主线程
}





/*
知识要素：
1.main.c是一个具体的实例，执行相关程序流程不会涉及任何环境，芯片问题。
该文件所有代码均不会涉及具体的硬件和环境，它通过调用相关构件来实现与硬件
系统的交互。
2.本文件共有两类代码，一类为【根据实际需要增删】，此类代码根据具体
项目需求进行更改；另一类为【不变】，此类代码与具体项目无关，是通用的，
无需根据具体项目进行更改。
3.本文件对宏GLOBLE_VAR进行了定义， 所以在包含"includes.h"头文件时
会定义全局变量，在其他文件中包含"includes.h"头文件时，仅声明该头文件中
的全局变量。
*/










