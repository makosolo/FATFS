/**
  ******************************************************************************
  * @file    bsp_flash.c
  * @author  solo
  * @version V0.0.1
  * @date    2016-02-16
  * @brief   FLASH����
  *
  * @verbatim ����API
  *           - bsp_InitFLASH           //SPI_FLASH��ʼ��
  *           - bsp_FLASH_ReadID        //��ȡоƬID  
  *           - bsp_FLASH_Read          //������  
  *           - bsp_FLASH_WriteNoCheck  //�޼���дSPI FLASH   
  *           - bsp_FLASH_Write         //��У��д
  *           - bsp_FLASH_EraseChip     //��������оƬ
  *           - bsp_FLASH_EraseSector   //����һ������
  *           - bsp_FLASH_WriteData     //�Ѳɼ�����д��FLASH��ֻ����ǰд
  *   
  * @note    
  *
  * @endverbatim  
  *                                  
  */

/* ͷ�ļ� ------------------------------------------------------------------*/
#include "sys.h"

#include "bsp_spi.h"
#include "bsp_flash.h"

#define  Dummy_byte 0xAA

#define	CS_0    GPIO_ResetBits(GPIOB, GPIO_Pin_12);
#define	CS_1    GPIO_SetBits(GPIOB, GPIO_Pin_12);

u16 W25QXX_TYPE = W25Q64;	//Ĭ����W25Q64

//��������
static u8 bsp_FLASH_ReadSR(void);//��ȡSPI_FLASH��״̬�Ĵ���
static void bsp_FLASH_WriteEnable(void);//SPI_FLASHдʹ��
static void bsp_FLASH_WritePage(u8* buf, u32 add, u16 len);//SPI��һҳ(0~65535)��д������256���ֽڵ�����
static void bsp_FLASH_WaitBusy(void);//�ȴ�����

/*******************************************************************************
* ������	: bsp_InitFLASH
* ����  	: SPI1�ӿڳ�ʼ��
* ����  	: ��
* ����ֵ	: ��
*******************************************************************************/
void bsp_InitFLASH(void)
{
    bsp_InitSpi2();//��ʼ��SPI2
}

/*******************************************************************************
* ������	: bsp_FLASH_ReadSR
* ����  	: ��ȡSPI_FLASH��״̬�Ĵ���
* ����  	: ��
* ����ֵ	: //BIT7  6   5   4   3   2   1   0
*				SPR   RV  TB BP2 BP1 BP0 WEL BUSY
*				SPR:Ĭ��0,״̬�Ĵ�������λ,���WPʹ��
*				TB,BP2,BP1,BP0:FLASH����д��������
*				WEL:дʹ������
*				BUSY:æ���λ(1,æ;0,����)
*				Ĭ��:0x00
*******************************************************************************/
static u8 bsp_FLASH_ReadSR(void)   
{  
	u8 r = 0;   
	
	CS_0
	
	bsp_SPI2_ReadWrite(W25X_ReadStatusReg);    //���Ͷ�ȡ״̬�Ĵ�������    
	r = bsp_SPI2_ReadWrite(Dummy_byte);        //��ȡһ���ֽ�  
	
	CS_1
	
	return(r);   
} 

/*******************************************************************************
* ������	: bsp_FLASH_WriteEnable
* ����  	: SPI_FLASHдʹ��
* ����  	: ��
* ����ֵ	: ��
*******************************************************************************/
static void bsp_FLASH_WriteEnable(void)
{
	CS_0
	
    bsp_SPI2_ReadWrite(W25X_WriteEnable);      //����дʹ��  
	
	CS_1
} 

/*******************************************************************************
* ������	: bsp_FLASH_WaitBusy
* ����  	: �ȴ�����
* ����  	: ��
* ����ֵ	: ��
*******************************************************************************/
static void bsp_FLASH_WaitBusy(void)
{
	while((bsp_FLASH_ReadSR()&0x01) == 0x01);//ֱ��BUSYλ���
}

/*******************************************************************************
* ������	: bsp_FLASH_ReadID
* ����  	: ��ȡоƬID
* ����  	: ��
* ����ֵ	: оƬID
*******************************************************************************/
u16 bsp_FLASH_ReadID(void)
{
	u16 ID = 0;	 
	
    #if FREERTOS_ENABLED 	//��FREERTOS_ENABLED������,˵��ʹ��FreeRTOS��.
    if(xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) FLASH_LOCK();//����//OS������
    #endif
    
	CS_0	
		    
	bsp_SPI2_ReadWrite(0x90);//���Ͷ�ȡID����	    
	bsp_SPI2_ReadWrite(0x00); 	    
	bsp_SPI2_ReadWrite(0x00); 	    
	bsp_SPI2_ReadWrite(0x00); 	 			   
	ID |= bsp_SPI2_ReadWrite(Dummy_byte)<<8;  
	ID |= bsp_SPI2_ReadWrite(Dummy_byte);	 

	CS_1
	
    #if FREERTOS_ENABLED 	//��FREERTOS_ENABLED������,˵��ʹ��FreeRTOS��.
    if(xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) FLASH_UNLOCK();//����//OS������
    #endif
    
	return(ID);
}   	

/*******************************************************************************
* ������	: bsp_FLASH_Read
* ����  	: ������(���65535)
* ����  	: -buf:���ݻ�����ָ��
*			  -add:���ݵ�ַ
*			  -len:��ȡ����
* ����ֵ	: ��
*******************************************************************************/
void bsp_FLASH_Read(u8* buf, u32 add,u16 len)   //ΪʲôҪ���Ƴ���Ϊu16�أ�
{ 
 	u16 i;   
	 
    #if FREERTOS_ENABLED 	//��FREERTOS_ENABLED������,˵��ʹ��FreeRTOS��.
    if(xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) FLASH_LOCK();//����//OS������
    #endif
    
	CS_0

    bsp_SPI2_ReadWrite(W25X_ReadData);    //���Ͷ�ȡ����   
	
    bsp_SPI2_ReadWrite((u8)((add)>>16));  //����24bit��ַ    
    bsp_SPI2_ReadWrite((u8)((add)>>8));   
    bsp_SPI2_ReadWrite((u8)add);   
	
    for(i=0;i<len;i++)
	{ 
        buf[i] = bsp_SPI2_ReadWrite(Dummy_byte);   //ѭ������
    }

	CS_1

    #if FREERTOS_ENABLED 	//��FREERTOS_ENABLED������,˵��ʹ��FreeRTOS��.
    if(xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) FLASH_UNLOCK();//����//OS������
    #endif
}

/*******************************************************************************
* ������	: bsp_FLASH_WritePage
* ����  	: SPI��һҳ(0~65535)��д������256���ֽڵ�����
* ����  	: -buf:���ݻ�����ָ��
*			  -add:���ݵ�ַ
*			  -len:д�볤��
* ����ֵ	: ��
*******************************************************************************/
static void bsp_FLASH_WritePage(u8* buf, u32 add, u16 len)
{
 	u16 i;  
	
    bsp_FLASH_WriteEnable();         
	
	CS_0
	   
    bsp_SPI2_ReadWrite(W25X_PageProgram); //����дҳ����   
	
    bsp_SPI2_ReadWrite((u8)((add)>>16)); //����24bit��ַ    
    bsp_SPI2_ReadWrite((u8)((add)>>8));   
    bsp_SPI2_ReadWrite((u8)add);   
	
    for(i=0;i<len;i++)
	{
		bsp_SPI2_ReadWrite(buf[i]);//ѭ��д��  
	}
	
	CS_1
	
	bsp_FLASH_WaitBusy();					   //�ȴ�д�����
}

/*******************************************************************************
* ������	: bsp_FLASH_WriteNoCheck
* ����  	: �޼���дSPI FLASH 
* ����  	: -buf:���ݻ�����ָ��
*			  -add:���ݵ�ַ
*			  -len:д�볤��
* ����ֵ	: ��
*******************************************************************************/
void bsp_FLASH_WriteNoCheck(u8* buf, u32 add, u16 len)
{ 			 		 
	u16 start;

	start = 256 - (add & 255); //��ҳʣ����ֽ���
			 	    
	if(len <= start)
	{
		start = len;//������256���ֽ�
	}
    
    #if FREERTOS_ENABLED 	//��FREERTOS_ENABLED������,˵��ʹ��FreeRTOS��.
    if(xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) FLASH_LOCK();//����//OS������
    #endif
    
	while(1)
	{	   
		bsp_FLASH_WritePage(buf, add, start);
		
		if(len == start)
		{
			break;
		}
	 	else
		{
			buf += start;
			add += start;	

			len -= start;		
			
			if(len > 256)
			{
				start = 256;
			}
			else
			{
				start = len; 	  //����256���ֽ���
			}
		}
	}
    
    #if FREERTOS_ENABLED 	//��FREERTOS_ENABLED������,˵��ʹ��FreeRTOS��.
    if(xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) FLASH_UNLOCK();//����//OS������
    #endif
} 

/*******************************************************************************
* ������	: bsp_FLASH_Write
* ����  	: �޼���дSPI FLASH 
* ����  	: -pBuffer:���ݴ洢��
*			  -WriteAddr:��ʼд��ĵ�ַ(24bit)
*			  -NumByteToWrite:Ҫд����ֽ���(���65535)
* ����ֵ	: ��
*******************************************************************************/
u8 SPI_FLASH_BUF[4096];
void bsp_FLASH_Write(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)   
{ 
	u32 secpos;
	u16 secoff;
	u16 secremain;	   
 	u16 i;    

	secpos=WriteAddr/4096;//������ַ 0~511 for w25x16
	secoff=WriteAddr%4096;//�������ڵ�ƫ��
	secremain=4096-secoff;//����ʣ��ռ��С

	if(NumByteToWrite<=secremain)secremain=NumByteToWrite;//������4096���ֽ�
        
	while(1) 
	{	
		bsp_FLASH_Read(SPI_FLASH_BUF,secpos*4096,4096);//������������������
		for(i=0;i<secremain;i++)//У������
		{
			if(SPI_FLASH_BUF[secoff+i]!=0XFF)break;
		}
		if(i<secremain)//��Ҫ����
		{			
			bsp_FLASH_EraseSector(secpos*4096);//�����������
			for(i=0;i<secremain;i++)	   //����
			{
				SPI_FLASH_BUF[i+secoff]=pBuffer[i];	  
			}
			bsp_FLASH_WriteNoCheck(SPI_FLASH_BUF,secpos*4096,4096);//д����������

		}else bsp_FLASH_WriteNoCheck(pBuffer,WriteAddr,secremain);//д�Ѿ������˵�,ֱ��д������ʣ������. 				   
		if(NumByteToWrite==secremain)break;//д�������
		else//д��δ����
		{
			secpos++;//������ַ��1
			secoff=0;//ƫ��λ��Ϊ0 	 

		   	pBuffer+=secremain;  //ָ��ƫ��
			WriteAddr+=secremain;//д��ַƫ��	   
		   	NumByteToWrite-=secremain;				//�ֽ����ݼ�
			if(NumByteToWrite>4096)secremain=4096;	//��һ����������д����
			else secremain=NumByteToWrite;			//��һ����������д����
		}	 
	}
}

/*******************************************************************************
* ������	: bsp_FLASH_EraseChip
* ����  	: ��������оƬ,�ȴ�ʱ�䳬��...
* ����  	: ��
* ����ֵ	: ��
*******************************************************************************/
void bsp_FLASH_EraseChip(void)
{
    #if FREERTOS_ENABLED 	//��FREERTOS_ENABLED������,˵��ʹ��FreeRTOS��.
    if(xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) FLASH_LOCK();//����//OS������
    #endif
    
    bsp_FLASH_WriteEnable();                  //SET WEL 
    bsp_FLASH_WaitBusy();   

	CS_0

    bsp_SPI2_ReadWrite(W25X_ChipErase);        //����Ƭ��������  

	CS_1

	bsp_FLASH_WaitBusy();   				   //�ȴ�оƬ��������
    
    #if FREERTOS_ENABLED 	//��FREERTOS_ENABLED������,˵��ʹ��FreeRTOS��.
    if(xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) FLASH_UNLOCK();//����//OS������
    #endif
}

/*******************************************************************************
* ������	: bsp_FLASH_EraseSector
* ����  	: ����һ������:150ms
* ����  	: Dst_Addr:������ַ
* ����ֵ	: ��
*******************************************************************************/
void bsp_FLASH_EraseSector(u32 Dst_Addr)   
{   
    #if FREERTOS_ENABLED 	//��FREERTOS_ENABLED������,˵��ʹ��FreeRTOS��.
    if(xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) FLASH_LOCK();//����//OS������
    #endif
    
    bsp_FLASH_WriteEnable();                  //SET WEL 	 
    bsp_FLASH_WaitBusy();   

	CS_0

    bsp_SPI2_ReadWrite(W25X_SectorErase);      //������������ָ�� 
	
    bsp_SPI2_ReadWrite((u8)((Dst_Addr)>>16));  //����24bit��ַ    
    bsp_SPI2_ReadWrite((u8)((Dst_Addr)>>8));   
    bsp_SPI2_ReadWrite((u8)Dst_Addr);

	CS_1

    #if FREERTOS_ENABLED 	//��FREERTOS_ENABLED������,˵��ʹ��FreeRTOS��.
    if(xTaskGetSchedulerState() == taskSCHEDULER_RUNNING)//OS������
    {
        vTaskDelay(150/portTICK_RATE_MS);//�ȴ�150ms
    }
    #endif
    
    bsp_FLASH_WaitBusy();   				   //�ȴ��������
    
    #if FREERTOS_ENABLED 	//��FREERTOS_ENABLED������,˵��ʹ��FreeRTOS��.
    if(xTaskGetSchedulerState() == taskSCHEDULER_RUNNING)  FLASH_UNLOCK();//����//OS������
    #endif
} 
