//===========================================================================
//文件名称：flash.c
//功能概要：Flash底层驱动构件源文件
//版权所有：SD-Arm(sumcu.suda.edu.cn)
//版本更新：20181201-20200221
//芯片类型：STM32
//===========================================================================
//包含头文件
#include "flash.h"
#include "string.h"

//=================内部调用函数声明=====================================
//======================================================================
//函数名称：flash_write_DoubleWord
//函数返回：0-成功 1-失败
//参数说明：addr：目标地址，要求为4的倍数且大于Flash首地址
//              （例如：0x08000004，Flash首地址为0x08000000）
//       data：写入的双字
//功能概要：Flash双字写入操作
//======================================================================
uint8_t flash_write_DoubleWord(uint32_t addr,uint32_t data_l,uint32_t data_h);

//======================================================================
//======================================================================

//=================外部接口函数=========================================
//======================================================================
//函数名称：flash_init
//函数返回：无
//参数说明：无
//功能概要：初始化flash模块
//======================================================================
void flash_init(void)
{
	//清除所有错误标志位【CC_改动】
	FLASH->SR &= 0xFFFFFFUL;
	//解锁Flash控制寄存器(CR)
	if((FLASH->CR & FLASH_CR_LOCK) != 0U)
    {
		FLASH->KEYR = (uint32_t)FLASH_KEY1;
		FLASH->KEYR = (uint32_t)FLASH_KEY2;
    }

    //等待之前最后一个flash操作完成
	while( (FLASH->SR & FLASH_SR_BSY) != 0U);

	//清数据缓冲区
    FLASH->ACR &= ~FLASH_ACR_DCEN_Msk;
    //清闪存即时编程位
    FLASH->CR &= ~FLASH_CR_PG_Msk;
}
//======================================================================
//函数名称：flash_erase
//函数返回：函数执行执行状态：0=正常；1=异常。
//参数说明：sect：目标扇区号（范围取决于实际芯片，例如 STM32L433:0~127,每扇区2KB;
//功能概要：擦除flash存储器的sect扇区
//======================================================================
uint8_t flash_erase(uint16_t sect)
{
	//等待之前最后一个flash操作完成
	while( (FLASH->SR & FLASH_SR_BSY) != 0U);
	//清闪存即时编程位
    FLASH->CR &= ~FLASH_CR_PG;

    //使能扇区擦除
	FLASH->CR |= FLASH_CR_PER;
	//设置擦除的扇区
	FLASH->CR &= ~FLASH_CR_PNB;
	FLASH->CR |= (uint32_t)(sect << 3u);
	//开始扇区擦除
	FLASH->CR |= FLASH_CR_STRT;
	//等待擦除操作完成
	while( (FLASH->SR & FLASH_SR_BSY) != 0U);
	//禁止扇区擦除
	FLASH->CR &= ~FLASH_CR_PER;

    return 0;  //成功返回
}
//======================================================================
//函数名称：flash_write
//函数返回：函数执行状态：0=正常；1=异常。
//参数说明：sect：扇区号（范围取决于实际芯片，例如 STM32L433:0~127,每扇区2KB）
//        offset:写入扇区内部偏移地址（0~2044，要求为0,4,8,12，......）
//        N：写入字节数目（4~2048,要求为4,8,12,......）
//        buf：源数据缓冲区首地址
//功能概要：将buf开始的N字节写入到flash存储器的sect扇区的 offset处
//=======================================================================
uint8_t flash_write(uint16_t sect,uint16_t offset,uint16_t N,uint8_t *buf)
{
	uint32_t addr;    //双字写入绝对地址
	uint32_t i,j;    //计数
	uint32_t temp_l,temp_h;    //temp_l双字中低位字，temp_h双字中高位字
    uint32_t wCount;  //双字(64位)写入次数
    uint8_t PadNum,PadCount;  //补齐字节，补齐数组计数
    uint8_t PadArray[8];  //补齐数组，用来存放补齐数据
    uint16_t front_offset;  //地址偏移，双字(64位)对齐
	front_offset = offset%8;//计算地址偏移
	if(front_offset == 0)  //如果地址双字对齐，则正常写入
    {
        wCount = (N-1)/8+1;  //计算双字写入次数
        PadNum = wCount*8-N;  //计算需补齐字节
        addr = (uint32_t)(FLASH_BASE+sect*FLASH_PAGE_SIZE+offset+N);  //计算读取地址
	    flash_read_physical((PadArray+(8-PadNum)),addr,PadNum);  //读取需补齐地址值存入补齐数组
		//等待之前最后一个flash操作完成
		while( (FLASH->SR & FLASH_SR_BSY) != 0U);
		//计算双字写入绝对地址
		addr = (uint32_t)(FLASH_BASE+sect*FLASH_PAGE_SIZE+offset);
	
		if(N<=8) //待写入数据不满双字，则补齐值双字长度
	    {
			PadCount = 0;
			for(i = 0;i < N; i++)
			{
				PadArray[PadCount] = buf[i];
				PadCount++;
			}
			temp_l =(uint32_t)((PadArray[0])|(PadArray[1]<<8)|(PadArray[2]<<16)|(PadArray[3]<<24));
			temp_h =(uint32_t)((PadArray[4])|(PadArray[5]<<8)|(PadArray[6]<<16)|(PadArray[7]<<24));
			flash_write_DoubleWord((addr),temp_l,temp_h);
	    }
	    else
		{
			//循环写入双字，每8个数写一次，不足8个自动补齐
			for(i = 0; i < (N-8); i+=8)
			{
				//低位字赋值
    			temp_l = (uint32_t)((buf[i])|(buf[i+1]<<8)|(buf[i+2]<<16)|(buf[i+3]<<24));
    			//高位字赋值
				temp_h = (uint32_t)((buf[i+4])|(buf[i+5]<<8)|(buf[i+6]<<16)|(buf[i+7]<<24));
				//在绝对地址(addr+i)，实现双字写入
				flash_write_DoubleWord((addr+i),temp_l,temp_h);
			}
		    PadCount = 0;  //补齐数组计数清零
		    //将还未写入的字节与补齐数组组合成双字
			for(j = i;j<N;j++)
			{
				PadArray[PadCount] = buf[j];
				PadCount++;
			}
			//低位字赋值
			temp_l =(uint32_t)((PadArray[0])|(PadArray[1]<<8)|(PadArray[2]<<16)|(PadArray[3]<<24));
			//高位字赋值
			temp_h =(uint32_t)((PadArray[4])|(PadArray[5]<<8)|(PadArray[6]<<16)|(PadArray[7]<<24));
			//在绝对地址(addr+i)，实现双字写入
			flash_write_DoubleWord((addr+i),temp_l,temp_h);	
		}
	}
	else    //如果地址非双字对齐，则对齐后写入
	{
		addr = (uint32_t)(FLASH_BASE+sect*FLASH_PAGE_SIZE+offset);  //计算写入地址
		flash_read_physical(PadArray,(addr-front_offset),front_offset);//读取地址前面需补齐地址值存入补齐数组
		PadCount = 0; //补齐数组计数清零
		//将需要写入的字节与补齐数组组合成双字
		for(i = front_offset; i < 8; i++)
		{
			PadArray[i] = buf[PadCount];
			PadCount++;
		}
		//低位字赋值
		temp_l =(uint32_t)((PadArray[0])|(PadArray[1]<<8)|(PadArray[2]<<16)|(PadArray[3]<<24));
		//高位字赋值
		temp_h =(uint32_t)((PadArray[4])|(PadArray[5]<<8)|(PadArray[6]<<16)|(PadArray[7]<<24));
		//在绝对地址(addr+i)，实现双字写入
		flash_write_DoubleWord((addr-front_offset),temp_l,temp_h);
		//剩下的数据正常写入
		flash_write_physical(addr+(8-front_offset),N-(8-front_offset),(buf+(8-front_offset)));
	}
	
	//等待写入操作完成
	while( (FLASH->SR & FLASH_SR_BSY) != 0U);
	return 0;  //成功执行
}

//==========================================================================
//函数名称：flash_write_physical
//函数返回：函数执行状态：0=正常；非0=异常。
//参数说明： addr：目标地址，要求为4的倍数且大于Flash首地址
//              （例如：0x08000004，Flash首地址为0x08000000）
//       cnt：写入字节数目（8~512）
//       buf：源数据缓冲区首地址
//功能概要：flash写入操作
//==========================================================================
uint8_t flash_write_physical(uint32_t addr,uint16_t N,uint8_t buf[])
{
	uint32_t Raddr;    //补齐数据读取地址
	uint32_t i,j;    //计数
	uint32_t temp_l,temp_h;    //temp_l双字中低位字，temp_h双字中高位字
    uint32_t wCount;  //双字(64位)写入次数
    uint8_t PadNum,PadCount;  //补齐字节，补齐数组计数
    uint8_t PadArray[8];  //补齐数组，用来存放补齐数据
    uint16_t front_offset;  //地址偏移，双字(64位)对齐
	front_offset = addr%8;//计算地址偏移
	if(front_offset == 0)  //如果地址双字对齐，则正常写入
    {
        wCount = (N-1)/8+1;  //计算双字写入次数
        PadNum = wCount*8-N;  //计算需补齐字节
        Raddr = (uint32_t)(addr+N);  //计算读取地址
	    flash_read_physical((PadArray+(8-PadNum)),Raddr,PadNum);  //读取需补齐地址值存入补齐数组
		//等待之前最后一个flash操作完成
		while( (FLASH->SR & FLASH_SR_BSY) != 0U);
	
		if(N<=8) //待写入数据不满双字，则补齐值双字长度
	    {
			PadCount = 0;//补齐数组计数清零
			//将需要写入的字节与补齐数组组合成双字
			for(i = 0;i < N; i++)
			{
				PadArray[PadCount] = buf[i];
				PadCount++;
			}
			//低位字赋值
			temp_l =(uint32_t)((PadArray[0])|(PadArray[1]<<8)|(PadArray[2]<<16)|(PadArray[3]<<24));
			//高位字赋值
			temp_h =(uint32_t)((PadArray[4])|(PadArray[5]<<8)|(PadArray[6]<<16)|(PadArray[7]<<24));
			//在绝对地址(addr+i)，实现双字写入
			flash_write_DoubleWord(addr,temp_l,temp_h);
	    }
	    else
		{
			//循环写入双字，每8个数写一次，不足8个自动补齐
			for(i = 0; i < (N-8); i+=8)
			{
				//低位字赋值
    			temp_l = (uint32_t)((buf[i])|(buf[i+1]<<8)|(buf[i+2]<<16)|(buf[i+3]<<24));
    			//高位字赋值
				temp_h = (uint32_t)((buf[i+4])|(buf[i+5]<<8)|(buf[i+6]<<16)|(buf[i+7]<<24));
				//在绝对地址(addr+i)，实现双字写入
				flash_write_DoubleWord((addr+i),temp_l,temp_h);
			}
		    PadCount = 0;  //补齐数组计数清零
		    //将还未写入的字节与补齐数组组合成双字
			for(j = i;j<N;j++)
			{
				PadArray[PadCount] = buf[j];
				PadCount++;
			}
			//低位字赋值
			temp_l =(uint32_t)((PadArray[0])|(PadArray[1]<<8)|(PadArray[2]<<16)|(PadArray[3]<<24));
			//高位字赋值
			temp_h =(uint32_t)((PadArray[4])|(PadArray[5]<<8)|(PadArray[6]<<16)|(PadArray[7]<<24));
			//在绝对地址(addr+i)，实现双字写入
			flash_write_DoubleWord((addr+i),temp_l,temp_h);	
		}
	}
	else    //如果地址非双字对齐，则对齐后写入
	{
		flash_read_physical(PadArray,(addr-front_offset),front_offset);//读取地址前面需补齐地址值存入补齐数组
		PadCount = 0; //补齐数组计数清零
		//将需要写入的字节与补齐数组组合成双字
		for(i = front_offset; i < 8; i++)
		{
			PadArray[i] = buf[PadCount];
			PadCount++;
		}
		//低位字赋值
		temp_l =(uint32_t)((PadArray[0])|(PadArray[1]<<8)|(PadArray[2]<<16)|(PadArray[3]<<24));
		//高位字赋值
		temp_h =(uint32_t)((PadArray[4])|(PadArray[5]<<8)|(PadArray[6]<<16)|(PadArray[7]<<24));
		//在绝对地址(addr+i)，实现双字写入
		flash_write_DoubleWord((addr-front_offset),temp_l,temp_h);
		//剩下的数据正常写入
		flash_write_physical(addr+(8-front_offset),N-(8-front_offset),(buf+(8-front_offset)));
	}
	
	//等待写入操作完成
	while( (FLASH->SR & FLASH_SR_BSY) != 0U);
	return 0;  //成功执行
}

//======================================================================
//函数名称：flash_read_logic
//函数返回：无
//参数说明：dest：读出数据存放处（传地址，目的是带出所读数据，RAM区）
//       sect：扇区号（范围取决于实际芯片，例如 STM32L433:0~127,每扇区2KB）
//       offset:扇区内部偏移地址（0~2024，要求为0,4,8,12，......）
//       N：读字节数目（4~2048,要求为4,8,12,......）//
//功能概要：读取flash存储器的sect扇区的 offset处开始的N字节，到RAM区dest处
//=======================================================================
void flash_read_logic(uint8_t *dest,uint16_t sect,uint16_t offset,uint16_t N)
{
	uint8_t *src;
	src=(uint8_t *)(FLASH_BASE+sect*FLASH_PAGE_SIZE+offset);
	memcpy(dest,src,N);
}

//======================================================================
//函数名称：flash_read_physical
//函数返回：无
//参数说明：dest：读出数据存放处（传地址，目的是带出所读数据，RAM区）
//       addr：目标地址，要求为4的倍数（例如：0x00000004）
//       N：读字节数目（0~1020,要求为4，8,12,......）
//功能概要：读取flash指定地址的内容
//======================================================================
void flash_read_physical(uint8_t *dest,uint32_t addr,uint16_t N)
{
	uint8_t *src;
	src=(uint8_t *)addr;
	memcpy(dest,src,N);
}

//======================================================================
//函数名称：flash_protect
//函数返回：无
//参数说明：M：待保护区域的扇区号入口值，实际保护所有扇区
//功能概要：flash保护操作
//======================================================================
void flash_protect(uint8_t M)
{
	uint32_t STRT,END;    //待保护区域的绝对地址，STRT起始地址，END结束地址

	//计算起始地址
    STRT = (uint32_t)(FLASH_PAGE_SIZE*M/0x8u);
    //计算结束地址
    END = (uint32_t)(FLASH_PAGE_SIZE*(M+1)/0x8u);
    //配置相关寄存器
	FLASH->PCROP1SR |= STRT;
	FLASH->PCROP1ER |= END;
}

//======================================================================
//函数名称：flash_unprotect
//函数返回：无
//参数说明：
//功能概要：flash解保护操作
//说        明：
//======================================================================
void flash_unprotect(uint8_t M)
{
	FLASH->PCROP1SR = 0xffffffff;
	FLASH->PCROP1ER = 0x08000000;
}



//========================================================================
//函数名称：flash_isempty
//函数返回：1=目标区域为空；0=目标区域非空。
//参数说明：所要探测的flash区域扇区号及字节数
//功能概要：flash判空操作
//========================================================================
uint8_t flash_isempty(uint16_t sect,uint16_t N)
{
   uint16_t i,flag;
   uint8_t* buff;
   i = 0;
   flag = 1;
   for(i = 0; i<N; i++)   //遍历区域内字节
   {
	  buff=(uint8_t*)(sect*2048);
      if(buff[i]!=0xff)   //非空
      {
      	  flag=0;
          break;
       }
   }
   return flag;
}

//========================================================================
//函数名称：flashCtl_isSectorProtected
//函数返回：1=扇区被保护；0=扇区未被保护
//参数说明：所要检测的扇区
//功能概要：判断flash扇区是否被保护
//=========================================================================
uint8_t flash_isSectorProtected(uint16_t sect)
{
return 1;
}

//----------------------以下为内部函数存放处----------------------------
//======================================================================
//函数名称：flash_write_DoubleWord
//函数返回：0-成功 1-失败
//参数说明：addr：目标地址，要求为4的倍数且大于Flash首地址
//              （例如：0x08000004，Flash首地址为0x08000000）
//       data：写入的双字
//功能概要：Flash双字写入操作（STM32L433每次只能实现双字写入，先写低位字，再写高位字）
//======================================================================
uint8_t flash_write_DoubleWord(uint32_t addr,uint32_t data_l,uint32_t data_h)
{
	//清数据缓冲区
	if((FLASH->ACR & FLASH_ACR_DCEN) != 0U)
	{
		FLASH->ACR &= ~FLASH_ACR_DCEN;
	}

	//使能Flash即时编程
    FLASH->CR |= FLASH_CR_PG;

    //先写入低位字
    *(__IO uint32_t*)addr = data_l;

    //再写入高位字
    *(__IO uint32_t*)(addr+4U) = data_h;

    //禁止Flash即时编程
	FLASH->CR &= ~FLASH_CR_PG;

	return 0;    //返回成功
}
