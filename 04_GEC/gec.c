//=====================================================================
//文件名称：gec.c文件
//制作单位：SD-Arm(sumcu.suda.edu.cn)
//更新记录：20181201-20200627
//移植规则：【固定】
//=====================================================================
#include "gec.h"
void **  component_fun;
//======================================================================
//函数名称：Vectors_Init
//函数返回：无
//参数说明：无
//功能概要：User对BIOS中断向量表的部分继承,构件库函数指针初始化
//修改信息：WYH，20200805，规范
//======================================================================
void  Vectors_Init()
{
    //根据是否有BIOS决定编译本段程序，若有BIOS，即GEC_USER_SECTOR_START!=0，
    //编译本函数代码
    #if (GEC_USER_SECTOR_START!=0)
    //【20210225】添加flash初始化，增强系统写入稳定性
	flash_init();
    //（1）若Flash倒数1扇区的前24字节为空，则写入设备序列号及软件版本号初值
    if(flash_isempty(MCU_SECTOR_NUM-1,24))
    {
        flash_write_physical((MCU_SECTOR_NUM-1)*MCU_SECTORSIZE+
        MCU_FLASH_ADDR_START,24,(uint8_t *)"0123456789ABCDEF20200716");
    }
    //（2）继承BIOS的串口中断
    void ** bios=(void *)MCU_FLASH_ADDR_START;   //向量表起始地址
    uint32_t user[MCU_SECTORSIZE/4];     //向量表变量数组
    //读取USER中断向量表各中断处理程序地址赋给user数组
    flash_read_physical((uint8_t *)user,GEC_USER_SECTOR_START*
    MCU_SECTORSIZE+MCU_FLASH_ADDR_START,MCU_SECTORSIZE);
    //判断USER中串口更新中断是否为BIOS对应中断地址。若是，则不作操作；否则修改
    //为BIOS对应中断地址
    if(user[BIOS_UART_UPDATE_IRQn] != (uint32_t)bios[BIOS_UART_UPDATE_IRQn])
    {
        //改变User中断向量表中BIOS_UART中断向量
        flash_erase(GEC_USER_SECTOR_START);
        //改USER中断向量表的BIOS程序写入串口的中断处理程序地址
        user[BIOS_UART_UPDATE_IRQn]=(uint32_t)bios[BIOS_UART_UPDATE_IRQn];
        
        //  0=从BIOS启动RT-Thread,User程序中无RT-Thread内核源码
        #if (RTThread_Start==0) 
        user[BIOS_SVC_IRQn]=(uint32_t)bios[BIOS_SVC_IRQn];
        user[BIOS_PendSV_Handler]=(uint32_t)bios[BIOS_PendSV_Handler];
        user[BIOS_SysTick_Handler]=(uint32_t)bios[BIOS_SysTick_Handler];
        #endif
        
        flash_write_physical(GEC_USER_SECTOR_START*MCU_SECTORSIZE+
        MCU_FLASH_ADDR_START,MCU_SECTORSIZE,(uint8_t *)user);
    }
    
    //（3）中断向量表指针重定向到USER程序的中断向量表
    SCB->VTOR = (uint32_t)(GEC_USER_SECTOR_START*MCU_SECTORSIZE+
    MCU_FLASH_ADDR_START);  
    //（4）给component_fun赋值，SYSTEM_FUNCTION函数用
    component_fun=(void **)(MCU_FLASH_ADDR_START+
    GEC_COMPONENT_LST_START*MCU_SECTORSIZE); 
    //（5）printf提示
    printf("  【User提示】：将进入User的main()执行...\r\n");
    
    #endif
}


//
void SYSTEM_FUNCTION(uint8_t * data)
{
    uint8_t userVersion[9];   //user版本号
    uint8_t IP_port[48+16+9]; //上位机的发送的IP 端口号 等信息
    switch(data[0])
    {
        //用户程序更新，复位停留至BIOS，由BIOS接管中断向量表、接管用户串口中断
        case '0':
        if(strncmp((char *)(data+1),(char *)((MCU_SECTOR_NUM-1)*
        MCU_SECTORSIZE+MCU_FLASH_ADDR_START+16),8) == 0)
        {
//            emuart_sendN(4,(uint8_t *)"same");
            break;
        }
        //跳转
        memcpy(component_fun[25],"tran\0",5);
        memcpy(userVersion,USER_TYPE,9);
        flash_read_physical(IP_port,(MCU_SECTOR_NUM-1)*MCU_SECTORSIZE+MCU_FLASH_ADDR_START,24);
        for(int i=0;i<40;i++)
        {
        	IP_port[i+24] = data[i+9];
        }
        
        for(int i=0;i<9;i++)
        {
        	IP_port[i+64] = userVersion[i];
        }
        
        flash_erase(MCU_SECTOR_NUM-1);
        flash_write_physical((MCU_SECTOR_NUM-1)*MCU_SECTORSIZE+MCU_FLASH_ADDR_START,73,IP_port);
        for(uint32_t i=0;i<=0x28ffff;i++);
        NVIC_SystemReset();
        
        break;
    }
    return;
}




