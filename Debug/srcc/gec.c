//=====================================================================
//�ļ����ƣ�gec.c�ļ�
//������λ��SD-Arm(sumcu.suda.edu.cn)
//���¼�¼��20181201-20200627
//��ֲ���򣺡��̶���
//=====================================================================
#include "gec.h"
void **  component_fun;
//======================================================================
//�������ƣ�Vectors_Init
//�������أ���
//����˵������
//���ܸ�Ҫ��User��BIOS�ж�������Ĳ��ּ̳�,�����⺯��ָ���ʼ��
//�޸���Ϣ��WYH��20200805���淶
//======================================================================
void  Vectors_Init()
{
    //�����Ƿ���BIOS�������뱾�γ�������BIOS����GEC_USER_SECTOR_START!=0��
    //���뱾��������
    #if (GEC_USER_SECTOR_START!=0)
    //��20210225�����flash��ʼ������ǿϵͳд���ȶ���
	flash_init();
    //��1����Flash����1������ǰ24�ֽ�Ϊ�գ���д���豸���кż�����汾�ų�ֵ
    if(flash_isempty(MCU_SECTOR_NUM-1,24))
    {
        flash_write_physical((MCU_SECTOR_NUM-1)*MCU_SECTORSIZE+
        MCU_FLASH_ADDR_START,24,(uint8_t *)"0123456789ABCDEF20200716");
    }
    //��2���̳�BIOS�Ĵ����ж�
    void ** bios=(void *)MCU_FLASH_ADDR_START;   //��������ʼ��ַ
    uint32_t user[MCU_SECTORSIZE/4];     //�������������
    //��ȡUSER�ж���������жϴ�������ַ����user����
    flash_read_physical((uint8_t *)user,GEC_USER_SECTOR_START*
    MCU_SECTORSIZE+MCU_FLASH_ADDR_START,MCU_SECTORSIZE);
    //�ж�USER�д��ڸ����ж��Ƿ�ΪBIOS��Ӧ�жϵ�ַ�����ǣ����������������޸�
    //ΪBIOS��Ӧ�жϵ�ַ
    if(user[BIOS_UART_UPDATE_IRQn] != (uint32_t)bios[BIOS_UART_UPDATE_IRQn])
    {
        //�ı�User�ж���������BIOS_UART�ж�����
        flash_erase(GEC_USER_SECTOR_START);
        //��USER�ж��������BIOS����д�봮�ڵ��жϴ�������ַ
        user[BIOS_UART_UPDATE_IRQn]=(uint32_t)bios[BIOS_UART_UPDATE_IRQn];
        
        //  0=��BIOS����RT-Thread,User��������RT-Thread�ں�Դ��
        #if (RTThread_Start==0) 
        user[BIOS_SVC_IRQn]=(uint32_t)bios[BIOS_SVC_IRQn];
        user[BIOS_PendSV_Handler]=(uint32_t)bios[BIOS_PendSV_Handler];
        user[BIOS_SysTick_Handler]=(uint32_t)bios[BIOS_SysTick_Handler];
        #endif
        
        flash_write_physical(GEC_USER_SECTOR_START*MCU_SECTORSIZE+
        MCU_FLASH_ADDR_START,MCU_SECTORSIZE,(uint8_t *)user);
    }
    
    //��3���ж�������ָ���ض���USER������ж�������
    SCB->VTOR = (uint32_t)(GEC_USER_SECTOR_START*MCU_SECTORSIZE+
    MCU_FLASH_ADDR_START);  
    //��4����component_fun��ֵ��SYSTEM_FUNCTION������
    component_fun=(void **)(MCU_FLASH_ADDR_START+
    GEC_COMPONENT_LST_START*MCU_SECTORSIZE); 
    //��5��printf��ʾ
    printf("  ��User��ʾ����������User��main()ִ��...\r\n");
    
    #endif
}


//
void SYSTEM_FUNCTION(uint8_t * data)
{
    uint8_t userVersion[9];   //user�汾��
    uint8_t IP_port[48+16+9]; //��λ���ķ��͵�IP �˿ں� ����Ϣ
    switch(data[0])
    {
        //�û�������£���λͣ����BIOS����BIOS�ӹ��ж��������ӹ��û������ж�
        case '0':
        if(strncmp((char *)(data+1),(char *)((MCU_SECTOR_NUM-1)*
        MCU_SECTORSIZE+MCU_FLASH_ADDR_START+16),8) == 0)
        {
//            emuart_sendN(4,(uint8_t *)"same");
            break;
        }
        //��ת
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




