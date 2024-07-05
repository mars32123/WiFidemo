//=====================================================================
//文件名称：timer.c
//功能概要：timer底层驱动构件源文件
//制作单位：苏州大学嵌入式系统与物联网研究所(sumcu.suda.edu.cn)
//版    本：  2019-11-15  V2.0
//适用芯片：STM32
//=====================================================================
#include"includes.h"
#include "timer.h"

//============================================================================
//函数名称：timer_init
//函数返回：无
//参数说明： timer_No:时钟模块号  具体时钟模块号见gec.h文件定义
//                 time_ms 定时器中断的时间间隔，单位为毫秒。
//功能概要：时钟模块初始化，其中TIM1为高级定时器，TIM2、TIM15、TIM16为通用定时器，
//         TIM6、TIM7为基本定时器
//============================================================================
void timer_init(uint8_t timer_No,uint32_t time_ms)
{
    switch(timer_No)
    {
    case 1:
	{
		RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;//使能定时器时钟
		TIM1->CR1 &= ~(TIM_CR1_DIR | TIM_CR1_CMS);//设置向上计数模式
		TIM1->CR1 &= ~TIM_CR1_CKD;//设置时钟划分
		TIM1->ARR = (uint16_t)(time_ms-1) ;//设置周期
		TIM1->PSC = MCU_SYSTEM_CLK_KHZ-1;//设置预分频
		//TIM2->RCR = 0;
		TIM1->EGR = TIM_EGR_UG;//初始化计数器
		TIM1->CR1 |= 0x00000001U;//计数器使能
		break;
	}
	case 2:
	{
		RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;//使能定时器时钟
		TIM2->CR1 &= ~(TIM_CR1_DIR | TIM_CR1_CMS);//设置向上计数模式
		TIM2->CR1 &= ~TIM_CR1_CKD;//设置时钟划分
		TIM2->ARR = (uint32_t)(time_ms-1) ;//设置周期   //time_ms*48000/PSC
		TIM2->PSC = MCU_SYSTEM_CLK_KHZ-1;//设置预分频  //分频之后的频率=选定的时钟源频率/(PSC+1),   (ARR+1)*(PSC+1)/fclk
		//TIM2->RCR = 0;
		TIM2->EGR = TIM_EGR_UG;//初始化计数器
		TIM2->CR1 |= 0x00000001U;//计数器使能
		break;
	}
	case 6: //TIM6只有向上计数
	{
		RCC->APB1ENR1 |= RCC_APB1ENR1_TIM6EN;
		TIM6->ARR = (uint16_t)(time_ms-1) ;
		TIM6->PSC = MCU_SYSTEM_CLK_KHZ-1;
		TIM6->EGR = TIM_EGR_UG;
		TIM6->CR1 |= 0x00000001U;
		break;
	}
	case 7: //TIM7只有向上计数
	{
		RCC->APB1ENR1 |= RCC_APB1ENR1_TIM7EN;
		TIM7->ARR = (uint16_t)(time_ms-1) ;
		TIM7->PSC = MCU_SYSTEM_CLK_KHZ-1;
		TIM7->EGR = TIM_EGR_UG;
		TIM7->CR1 |= 0x00000001U;
		break;
	}
	case 15: //TIM15只有向上计数
	{
		RCC->APB2ENR |= RCC_APB2ENR_TIM15EN;
		TIM15->ARR = (uint16_t)(time_ms-1) ;
		TIM15->PSC = MCU_SYSTEM_CLK_KHZ-1;
		TIM15->EGR = TIM_EGR_UG;
		TIM15->CR1 |= 0x00000001U;
		break;
	}
	case 16: //TIM16只有向上计数
	{
	    RCC->APB2ENR |= RCC_APB2ENR_TIM16EN;
		TIM16->ARR = (uint16_t)(time_ms-1) ;
		TIM16->PSC = MCU_SYSTEM_CLK_KHZ-1;
		TIM16->EGR = TIM_EGR_UG;
		TIM16->CR1 |= 0x00000001U;
		break;
	}
	}

}


//============================================================================
//函数名称：timer_enable_int
//函数返回：无
//参数说明： timer_No:时钟模块号  具体时钟模块号见gec.h文件定义
//功能概要：时钟模块使能，开启时钟模块中断及定时器中断
//============================================================================
void timer_enable_int(uint8_t timer_No)
{
    switch(timer_No)
    {
    case 1:
	{
		TIM1->DIER |= TIM_DIER_UIE;
		NVIC_EnableIRQ(TIM1_UP_TIM16_IRQn);    //开中断控制器IRQ中断
	    break;
	}
	case 2:
	{
		TIM2->DIER |= TIM_DIER_UIE;
		NVIC_EnableIRQ(TIM2_IRQn);    //开中断控制器IRQ中断
	    break;
	}
	case 6:
	{
		TIM6->DIER |= TIM_DIER_UIE;
		NVIC_EnableIRQ(TIM6_DAC_IRQn);    //开中断控制器IRQ中断
	    break;
	}
	case 7:
	{
		TIM7->DIER |= TIM_DIER_UIE;
		NVIC_EnableIRQ(TIM7_IRQn);    //开中断控制器IRQ中断
	    break;
	}
	case 15:
	{
		TIM15->DIER |= TIM_DIER_UIE;
		NVIC_EnableIRQ(TIM1_BRK_TIM15_IRQn);    //开中断控制器IRQ中断
	    break;
	}
	case 16:
	{
		TIM16->DIER |= TIM_DIER_UIE;
		NVIC_EnableIRQ(TIM1_UP_TIM16_IRQn);    //开中断控制器IRQ中断
	    break;
	}
    }
}

//============================================================================
//函数名称：timer_disable_int
//函数返回：无
//参数说明： timer_No:时钟模块号  具体时钟模块号见gec.h文件定义
//功能概要：定时器中断除能
//============================================================================
void timer_disable_int(uint8_t timer_No)
{
    switch(timer_No)
	{
	case 1:
	{
		TIM1->DIER &= ~TIM_DIER_UIE;
		NVIC_DisableIRQ(TIM1_UP_TIM16_IRQn);    //禁止中断控制器IRQ中断
	    break;
	}
	case 2:
	{
		TIM2->DIER &= ~TIM_DIER_UIE;
		NVIC_DisableIRQ(TIM2_IRQn);    //禁止中断控制器IRQ中断
	    break;
	}
	case 6:
	{
		TIM6->DIER &= ~TIM_DIER_UIE;
		NVIC_DisableIRQ(TIM6_DAC_IRQn);    //禁止中断控制器IRQ中断
	    break;
	}
	case 7:
	{
		TIM7->DIER &= ~TIM_DIER_UIE;
		NVIC_DisableIRQ(TIM7_IRQn);    //禁止中断控制器IRQ中断
	    break;
	}
	case 15:
	{
		TIM15->DIER &= ~TIM_DIER_UIE;
		NVIC_DisableIRQ(TIM1_BRK_TIM15_IRQn);    //禁止中断控制器IRQ中断
	    break;
	}
	case 16:
	{
		TIM16->DIER &= ~TIM_DIER_UIE;
		NVIC_DisableIRQ(TIM1_UP_TIM16_IRQn);    //禁止中断控制器IRQ中断
	    break;
	}
	}
}

//===================================================================
//函数名称：timer_get_int
//参数说明： timer_No:时钟模块号  具体时钟模块号见gec.h文件定义
//功能概要：获取timer模块中断标志
//函数返回：中断标志 1=有对应模块中断产生;0=无对应模块中断产生
//===================================================================
uint8_t timer_get_int(uint8_t timer_No)
{
    if(timer_No==1)
    {
		if((TIM1->SR & TIM_SR_UIF)==TIM_SR_UIF)
			return 1;
	}
	else if(timer_No==2)
	{
		if((TIM2->SR & TIM_SR_UIF)==TIM_SR_UIF)
			return 1;
	}
	else if(timer_No==6)
	{
		if((TIM6->SR & TIM_SR_UIF)==TIM_SR_UIF)
			return 1;
	}
	else if(timer_No==7)
	{
		if((TIM7->SR & TIM_SR_UIF)==TIM_SR_UIF)
			return 1;
	}
	else if(timer_No==15)
	{
		if((TIM15->SR & TIM_SR_UIF)==TIM_SR_UIF)
			return 1;
	}
	else if(timer_No==16)
	{
		if((TIM16->SR & TIM_SR_UIF)==TIM_SR_UIF)
			return 1;
	}
	return 0;
}

//============================================================================
//函数名称：timer_clear_int
//函数返回：无
//参数说明： timer_No:时钟模块号  具体时钟模块号见gec.h文件定义
//功能概要：定时器清除中断标志
//============================================================================
void timer_clear_int(uint8_t timer_No)
{
    switch(timer_No)
    {
    case 1:TIM1->SR &=  ~TIM_SR_UIF;break;
	case 2:TIM2->SR &=  ~TIM_SR_UIF;break;
	case 6:TIM6->SR &=  ~TIM_SR_UIF;break;
	case 7:TIM7->SR &=  ~TIM_SR_UIF;break;
	case 15:TIM15->SR &=  ~TIM_SR_UIF;break;
	case 16:TIM16->SR &=  ~TIM_SR_UIF;break;
    }
}
