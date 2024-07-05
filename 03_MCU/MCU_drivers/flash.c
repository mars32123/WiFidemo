//===========================================================================
//�ļ����ƣ�flash.c
//���ܸ�Ҫ��Flash�ײ���������Դ�ļ�
//��Ȩ���У�SD-Arm(sumcu.suda.edu.cn)
//�汾���£�20181201-20200221
//оƬ���ͣ�STM32
//===========================================================================
//����ͷ�ļ�
#include "flash.h"
#include "string.h"

//=================�ڲ����ú�������=====================================
//======================================================================
//�������ƣ�flash_write_DoubleWord
//�������أ�0-�ɹ� 1-ʧ��
//����˵����addr��Ŀ���ַ��Ҫ��Ϊ4�ı����Ҵ���Flash�׵�ַ
//              �����磺0x08000004��Flash�׵�ַΪ0x08000000��
//       data��д���˫��
//���ܸ�Ҫ��Flash˫��д�����
//======================================================================
uint8_t flash_write_DoubleWord(uint32_t addr,uint32_t data_l,uint32_t data_h);

//======================================================================
//======================================================================

//=================�ⲿ�ӿں���=========================================
//======================================================================
//�������ƣ�flash_init
//�������أ���
//����˵������
//���ܸ�Ҫ����ʼ��flashģ��
//======================================================================
void flash_init(void)
{
	//������д����־λ��CC_�Ķ���
	FLASH->SR &= 0xFFFFFFUL;
	//����Flash���ƼĴ���(CR)
	if((FLASH->CR & FLASH_CR_LOCK) != 0U)
    {
		FLASH->KEYR = (uint32_t)FLASH_KEY1;
		FLASH->KEYR = (uint32_t)FLASH_KEY2;
    }

    //�ȴ�֮ǰ���һ��flash�������
	while( (FLASH->SR & FLASH_SR_BSY) != 0U);

	//�����ݻ�����
    FLASH->ACR &= ~FLASH_ACR_DCEN_Msk;
    //�����漴ʱ���λ
    FLASH->CR &= ~FLASH_CR_PG_Msk;
}
//======================================================================
//�������ƣ�flash_erase
//�������أ�����ִ��ִ��״̬��0=������1=�쳣��
//����˵����sect��Ŀ�������ţ���Χȡ����ʵ��оƬ������ STM32L433:0~127,ÿ����2KB;
//���ܸ�Ҫ������flash�洢����sect����
//======================================================================
uint8_t flash_erase(uint16_t sect)
{
	//�ȴ�֮ǰ���һ��flash�������
	while( (FLASH->SR & FLASH_SR_BSY) != 0U);
	//�����漴ʱ���λ
    FLASH->CR &= ~FLASH_CR_PG;

    //ʹ����������
	FLASH->CR |= FLASH_CR_PER;
	//���ò���������
	FLASH->CR &= ~FLASH_CR_PNB;
	FLASH->CR |= (uint32_t)(sect << 3u);
	//��ʼ��������
	FLASH->CR |= FLASH_CR_STRT;
	//�ȴ������������
	while( (FLASH->SR & FLASH_SR_BSY) != 0U);
	//��ֹ��������
	FLASH->CR &= ~FLASH_CR_PER;

    return 0;  //�ɹ�����
}
//======================================================================
//�������ƣ�flash_write
//�������أ�����ִ��״̬��0=������1=�쳣��
//����˵����sect�������ţ���Χȡ����ʵ��оƬ������ STM32L433:0~127,ÿ����2KB��
//        offset:д�������ڲ�ƫ�Ƶ�ַ��0~2044��Ҫ��Ϊ0,4,8,12��......��
//        N��д���ֽ���Ŀ��4~2048,Ҫ��Ϊ4,8,12,......��
//        buf��Դ���ݻ������׵�ַ
//���ܸ�Ҫ����buf��ʼ��N�ֽ�д�뵽flash�洢����sect������ offset��
//=======================================================================
uint8_t flash_write(uint16_t sect,uint16_t offset,uint16_t N,uint8_t *buf)
{
	uint32_t addr;    //˫��д����Ե�ַ
	uint32_t i,j;    //����
	uint32_t temp_l,temp_h;    //temp_l˫���е�λ�֣�temp_h˫���и�λ��
    uint32_t wCount;  //˫��(64λ)д�����
    uint8_t PadNum,PadCount;  //�����ֽڣ������������
    uint8_t PadArray[8];  //�������飬������Ų�������
    uint16_t front_offset;  //��ַƫ�ƣ�˫��(64λ)����
	front_offset = offset%8;//�����ַƫ��
	if(front_offset == 0)  //�����ַ˫�ֶ��룬������д��
    {
        wCount = (N-1)/8+1;  //����˫��д�����
        PadNum = wCount*8-N;  //�����貹���ֽ�
        addr = (uint32_t)(FLASH_BASE+sect*FLASH_PAGE_SIZE+offset+N);  //�����ȡ��ַ
	    flash_read_physical((PadArray+(8-PadNum)),addr,PadNum);  //��ȡ�貹���ֵַ���벹������
		//�ȴ�֮ǰ���һ��flash�������
		while( (FLASH->SR & FLASH_SR_BSY) != 0U);
		//����˫��д����Ե�ַ
		addr = (uint32_t)(FLASH_BASE+sect*FLASH_PAGE_SIZE+offset);
	
		if(N<=8) //��д�����ݲ���˫�֣�����ֵ˫�ֳ���
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
			//ѭ��д��˫�֣�ÿ8����дһ�Σ�����8���Զ�����
			for(i = 0; i < (N-8); i+=8)
			{
				//��λ�ָ�ֵ
    			temp_l = (uint32_t)((buf[i])|(buf[i+1]<<8)|(buf[i+2]<<16)|(buf[i+3]<<24));
    			//��λ�ָ�ֵ
				temp_h = (uint32_t)((buf[i+4])|(buf[i+5]<<8)|(buf[i+6]<<16)|(buf[i+7]<<24));
				//�ھ��Ե�ַ(addr+i)��ʵ��˫��д��
				flash_write_DoubleWord((addr+i),temp_l,temp_h);
			}
		    PadCount = 0;  //���������������
		    //����δд����ֽ��벹��������ϳ�˫��
			for(j = i;j<N;j++)
			{
				PadArray[PadCount] = buf[j];
				PadCount++;
			}
			//��λ�ָ�ֵ
			temp_l =(uint32_t)((PadArray[0])|(PadArray[1]<<8)|(PadArray[2]<<16)|(PadArray[3]<<24));
			//��λ�ָ�ֵ
			temp_h =(uint32_t)((PadArray[4])|(PadArray[5]<<8)|(PadArray[6]<<16)|(PadArray[7]<<24));
			//�ھ��Ե�ַ(addr+i)��ʵ��˫��д��
			flash_write_DoubleWord((addr+i),temp_l,temp_h);	
		}
	}
	else    //�����ַ��˫�ֶ��룬������д��
	{
		addr = (uint32_t)(FLASH_BASE+sect*FLASH_PAGE_SIZE+offset);  //����д���ַ
		flash_read_physical(PadArray,(addr-front_offset),front_offset);//��ȡ��ַǰ���貹���ֵַ���벹������
		PadCount = 0; //���������������
		//����Ҫд����ֽ��벹��������ϳ�˫��
		for(i = front_offset; i < 8; i++)
		{
			PadArray[i] = buf[PadCount];
			PadCount++;
		}
		//��λ�ָ�ֵ
		temp_l =(uint32_t)((PadArray[0])|(PadArray[1]<<8)|(PadArray[2]<<16)|(PadArray[3]<<24));
		//��λ�ָ�ֵ
		temp_h =(uint32_t)((PadArray[4])|(PadArray[5]<<8)|(PadArray[6]<<16)|(PadArray[7]<<24));
		//�ھ��Ե�ַ(addr+i)��ʵ��˫��д��
		flash_write_DoubleWord((addr-front_offset),temp_l,temp_h);
		//ʣ�µ���������д��
		flash_write_physical(addr+(8-front_offset),N-(8-front_offset),(buf+(8-front_offset)));
	}
	
	//�ȴ�д��������
	while( (FLASH->SR & FLASH_SR_BSY) != 0U);
	return 0;  //�ɹ�ִ��
}

//==========================================================================
//�������ƣ�flash_write_physical
//�������أ�����ִ��״̬��0=��������0=�쳣��
//����˵���� addr��Ŀ���ַ��Ҫ��Ϊ4�ı����Ҵ���Flash�׵�ַ
//              �����磺0x08000004��Flash�׵�ַΪ0x08000000��
//       cnt��д���ֽ���Ŀ��8~512��
//       buf��Դ���ݻ������׵�ַ
//���ܸ�Ҫ��flashд�����
//==========================================================================
uint8_t flash_write_physical(uint32_t addr,uint16_t N,uint8_t buf[])
{
	uint32_t Raddr;    //�������ݶ�ȡ��ַ
	uint32_t i,j;    //����
	uint32_t temp_l,temp_h;    //temp_l˫���е�λ�֣�temp_h˫���и�λ��
    uint32_t wCount;  //˫��(64λ)д�����
    uint8_t PadNum,PadCount;  //�����ֽڣ������������
    uint8_t PadArray[8];  //�������飬������Ų�������
    uint16_t front_offset;  //��ַƫ�ƣ�˫��(64λ)����
	front_offset = addr%8;//�����ַƫ��
	if(front_offset == 0)  //�����ַ˫�ֶ��룬������д��
    {
        wCount = (N-1)/8+1;  //����˫��д�����
        PadNum = wCount*8-N;  //�����貹���ֽ�
        Raddr = (uint32_t)(addr+N);  //�����ȡ��ַ
	    flash_read_physical((PadArray+(8-PadNum)),Raddr,PadNum);  //��ȡ�貹���ֵַ���벹������
		//�ȴ�֮ǰ���һ��flash�������
		while( (FLASH->SR & FLASH_SR_BSY) != 0U);
	
		if(N<=8) //��д�����ݲ���˫�֣�����ֵ˫�ֳ���
	    {
			PadCount = 0;//���������������
			//����Ҫд����ֽ��벹��������ϳ�˫��
			for(i = 0;i < N; i++)
			{
				PadArray[PadCount] = buf[i];
				PadCount++;
			}
			//��λ�ָ�ֵ
			temp_l =(uint32_t)((PadArray[0])|(PadArray[1]<<8)|(PadArray[2]<<16)|(PadArray[3]<<24));
			//��λ�ָ�ֵ
			temp_h =(uint32_t)((PadArray[4])|(PadArray[5]<<8)|(PadArray[6]<<16)|(PadArray[7]<<24));
			//�ھ��Ե�ַ(addr+i)��ʵ��˫��д��
			flash_write_DoubleWord(addr,temp_l,temp_h);
	    }
	    else
		{
			//ѭ��д��˫�֣�ÿ8����дһ�Σ�����8���Զ�����
			for(i = 0; i < (N-8); i+=8)
			{
				//��λ�ָ�ֵ
    			temp_l = (uint32_t)((buf[i])|(buf[i+1]<<8)|(buf[i+2]<<16)|(buf[i+3]<<24));
    			//��λ�ָ�ֵ
				temp_h = (uint32_t)((buf[i+4])|(buf[i+5]<<8)|(buf[i+6]<<16)|(buf[i+7]<<24));
				//�ھ��Ե�ַ(addr+i)��ʵ��˫��д��
				flash_write_DoubleWord((addr+i),temp_l,temp_h);
			}
		    PadCount = 0;  //���������������
		    //����δд����ֽ��벹��������ϳ�˫��
			for(j = i;j<N;j++)
			{
				PadArray[PadCount] = buf[j];
				PadCount++;
			}
			//��λ�ָ�ֵ
			temp_l =(uint32_t)((PadArray[0])|(PadArray[1]<<8)|(PadArray[2]<<16)|(PadArray[3]<<24));
			//��λ�ָ�ֵ
			temp_h =(uint32_t)((PadArray[4])|(PadArray[5]<<8)|(PadArray[6]<<16)|(PadArray[7]<<24));
			//�ھ��Ե�ַ(addr+i)��ʵ��˫��д��
			flash_write_DoubleWord((addr+i),temp_l,temp_h);	
		}
	}
	else    //�����ַ��˫�ֶ��룬������д��
	{
		flash_read_physical(PadArray,(addr-front_offset),front_offset);//��ȡ��ַǰ���貹���ֵַ���벹������
		PadCount = 0; //���������������
		//����Ҫд����ֽ��벹��������ϳ�˫��
		for(i = front_offset; i < 8; i++)
		{
			PadArray[i] = buf[PadCount];
			PadCount++;
		}
		//��λ�ָ�ֵ
		temp_l =(uint32_t)((PadArray[0])|(PadArray[1]<<8)|(PadArray[2]<<16)|(PadArray[3]<<24));
		//��λ�ָ�ֵ
		temp_h =(uint32_t)((PadArray[4])|(PadArray[5]<<8)|(PadArray[6]<<16)|(PadArray[7]<<24));
		//�ھ��Ե�ַ(addr+i)��ʵ��˫��д��
		flash_write_DoubleWord((addr-front_offset),temp_l,temp_h);
		//ʣ�µ���������д��
		flash_write_physical(addr+(8-front_offset),N-(8-front_offset),(buf+(8-front_offset)));
	}
	
	//�ȴ�д��������
	while( (FLASH->SR & FLASH_SR_BSY) != 0U);
	return 0;  //�ɹ�ִ��
}

//======================================================================
//�������ƣ�flash_read_logic
//�������أ���
//����˵����dest���������ݴ�Ŵ�������ַ��Ŀ���Ǵ����������ݣ�RAM����
//       sect�������ţ���Χȡ����ʵ��оƬ������ STM32L433:0~127,ÿ����2KB��
//       offset:�����ڲ�ƫ�Ƶ�ַ��0~2024��Ҫ��Ϊ0,4,8,12��......��
//       N�����ֽ���Ŀ��4~2048,Ҫ��Ϊ4,8,12,......��//
//���ܸ�Ҫ����ȡflash�洢����sect������ offset����ʼ��N�ֽڣ���RAM��dest��
//=======================================================================
void flash_read_logic(uint8_t *dest,uint16_t sect,uint16_t offset,uint16_t N)
{
	uint8_t *src;
	src=(uint8_t *)(FLASH_BASE+sect*FLASH_PAGE_SIZE+offset);
	memcpy(dest,src,N);
}

//======================================================================
//�������ƣ�flash_read_physical
//�������أ���
//����˵����dest���������ݴ�Ŵ�������ַ��Ŀ���Ǵ����������ݣ�RAM����
//       addr��Ŀ���ַ��Ҫ��Ϊ4�ı��������磺0x00000004��
//       N�����ֽ���Ŀ��0~1020,Ҫ��Ϊ4��8,12,......��
//���ܸ�Ҫ����ȡflashָ����ַ������
//======================================================================
void flash_read_physical(uint8_t *dest,uint32_t addr,uint16_t N)
{
	uint8_t *src;
	src=(uint8_t *)addr;
	memcpy(dest,src,N);
}

//======================================================================
//�������ƣ�flash_protect
//�������أ���
//����˵����M����������������������ֵ��ʵ�ʱ�����������
//���ܸ�Ҫ��flash��������
//======================================================================
void flash_protect(uint8_t M)
{
	uint32_t STRT,END;    //����������ľ��Ե�ַ��STRT��ʼ��ַ��END������ַ

	//������ʼ��ַ
    STRT = (uint32_t)(FLASH_PAGE_SIZE*M/0x8u);
    //���������ַ
    END = (uint32_t)(FLASH_PAGE_SIZE*(M+1)/0x8u);
    //������ؼĴ���
	FLASH->PCROP1SR |= STRT;
	FLASH->PCROP1ER |= END;
}

//======================================================================
//�������ƣ�flash_unprotect
//�������أ���
//����˵����
//���ܸ�Ҫ��flash�Ᵽ������
//˵        ����
//======================================================================
void flash_unprotect(uint8_t M)
{
	FLASH->PCROP1SR = 0xffffffff;
	FLASH->PCROP1ER = 0x08000000;
}



//========================================================================
//�������ƣ�flash_isempty
//�������أ�1=Ŀ������Ϊ�գ�0=Ŀ������ǿա�
//����˵������Ҫ̽���flash���������ż��ֽ���
//���ܸ�Ҫ��flash�пղ���
//========================================================================
uint8_t flash_isempty(uint16_t sect,uint16_t N)
{
   uint16_t i,flag;
   uint8_t* buff;
   i = 0;
   flag = 1;
   for(i = 0; i<N; i++)   //�����������ֽ�
   {
	  buff=(uint8_t*)(sect*2048);
      if(buff[i]!=0xff)   //�ǿ�
      {
      	  flag=0;
          break;
       }
   }
   return flag;
}

//========================================================================
//�������ƣ�flashCtl_isSectorProtected
//�������أ�1=������������0=����δ������
//����˵������Ҫ��������
//���ܸ�Ҫ���ж�flash�����Ƿ񱻱���
//=========================================================================
uint8_t flash_isSectorProtected(uint16_t sect)
{
return 1;
}

//----------------------����Ϊ�ڲ�������Ŵ�----------------------------
//======================================================================
//�������ƣ�flash_write_DoubleWord
//�������أ�0-�ɹ� 1-ʧ��
//����˵����addr��Ŀ���ַ��Ҫ��Ϊ4�ı����Ҵ���Flash�׵�ַ
//              �����磺0x08000004��Flash�׵�ַΪ0x08000000��
//       data��д���˫��
//���ܸ�Ҫ��Flash˫��д�������STM32L433ÿ��ֻ��ʵ��˫��д�룬��д��λ�֣���д��λ�֣�
//======================================================================
uint8_t flash_write_DoubleWord(uint32_t addr,uint32_t data_l,uint32_t data_h)
{
	//�����ݻ�����
	if((FLASH->ACR & FLASH_ACR_DCEN) != 0U)
	{
		FLASH->ACR &= ~FLASH_ACR_DCEN;
	}

	//ʹ��Flash��ʱ���
    FLASH->CR |= FLASH_CR_PG;

    //��д���λ��
    *(__IO uint32_t*)addr = data_l;

    //��д���λ��
    *(__IO uint32_t*)(addr+4U) = data_h;

    //��ֹFlash��ʱ���
	FLASH->CR &= ~FLASH_CR_PG;

	return 0;    //���سɹ�
}
