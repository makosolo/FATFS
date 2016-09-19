/**
  ******************************************************************************
  * @file    bsp_flash.c
  * @author  solo
  * @version V0.0.1
  * @date    2016-02-16
  * @brief   FLASH驱动
  *
  * @verbatim 对外API
  *           - bsp_InitFLASH           //SPI_FLASH初始化
  *           - bsp_FLASH_ReadID        //读取芯片ID  
  *           - bsp_FLASH_Read          //读数据  
  *           - bsp_FLASH_WriteNoCheck  //无检验写SPI FLASH   
  *           - bsp_FLASH_Write         //有校验写
  *           - bsp_FLASH_EraseChip     //擦除整个芯片
  *           - bsp_FLASH_EraseSector   //擦除一个扇区
  *           - bsp_FLASH_WriteData     //把采集数据写入FLASH，只能向前写
  *   
  * @note    
  *
  * @endverbatim  
  *                                  
  */

/* 头文件 ------------------------------------------------------------------*/
#include "sys.h"

#include "bsp_spi.h"
#include "bsp_flash.h"

#define  Dummy_byte 0xAA

#define	CS_0    GPIO_ResetBits(GPIOB, GPIO_Pin_12);
#define	CS_1    GPIO_SetBits(GPIOB, GPIO_Pin_12);

u16 W25QXX_TYPE = W25Q64;	//默认是W25Q64

//函数声明
static u8 bsp_FLASH_ReadSR(void);//读取SPI_FLASH的状态寄存器
static void bsp_FLASH_WriteEnable(void);//SPI_FLASH写使能
static void bsp_FLASH_WritePage(u8* buf, u32 add, u16 len);//SPI在一页(0~65535)内写入少于256个字节的数据
static void bsp_FLASH_WaitBusy(void);//等待空闲

/*******************************************************************************
* 函数名	: bsp_InitFLASH
* 描述  	: SPI1接口初始化
* 参数  	: 无
* 返回值	: 无
*******************************************************************************/
void bsp_InitFLASH(void)
{
    bsp_InitSpi2();//初始化SPI2
}

/*******************************************************************************
* 函数名	: bsp_FLASH_ReadSR
* 描述  	: 读取SPI_FLASH的状态寄存器
* 参数  	: 无
* 返回值	: //BIT7  6   5   4   3   2   1   0
*				SPR   RV  TB BP2 BP1 BP0 WEL BUSY
*				SPR:默认0,状态寄存器保护位,配合WP使用
*				TB,BP2,BP1,BP0:FLASH区域写保护设置
*				WEL:写使能锁定
*				BUSY:忙标记位(1,忙;0,空闲)
*				默认:0x00
*******************************************************************************/
static u8 bsp_FLASH_ReadSR(void)   
{  
	u8 r = 0;   
	
	CS_0
	
	bsp_SPI2_ReadWrite(W25X_ReadStatusReg);    //发送读取状态寄存器命令    
	r = bsp_SPI2_ReadWrite(Dummy_byte);        //读取一个字节  
	
	CS_1
	
	return(r);   
} 

/*******************************************************************************
* 函数名	: bsp_FLASH_WriteEnable
* 描述  	: SPI_FLASH写使能
* 参数  	: 无
* 返回值	: 无
*******************************************************************************/
static void bsp_FLASH_WriteEnable(void)
{
	CS_0
	
    bsp_SPI2_ReadWrite(W25X_WriteEnable);      //发送写使能  
	
	CS_1
} 

/*******************************************************************************
* 函数名	: bsp_FLASH_WaitBusy
* 描述  	: 等待空闲
* 参数  	: 无
* 返回值	: 无
*******************************************************************************/
static void bsp_FLASH_WaitBusy(void)
{
	while((bsp_FLASH_ReadSR()&0x01) == 0x01);//直到BUSY位清空
}

/*******************************************************************************
* 函数名	: bsp_FLASH_ReadID
* 描述  	: 读取芯片ID
* 参数  	: 无
* 返回值	: 芯片ID
*******************************************************************************/
u16 bsp_FLASH_ReadID(void)
{
	u16 ID = 0;	 
	
    #if FREERTOS_ENABLED 	//如FREERTOS_ENABLED定义了,说明使用FreeRTOS了.
    if(xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) FLASH_LOCK();//锁定//OS运行中
    #endif
    
	CS_0	
		    
	bsp_SPI2_ReadWrite(0x90);//发送读取ID命令	    
	bsp_SPI2_ReadWrite(0x00); 	    
	bsp_SPI2_ReadWrite(0x00); 	    
	bsp_SPI2_ReadWrite(0x00); 	 			   
	ID |= bsp_SPI2_ReadWrite(Dummy_byte)<<8;  
	ID |= bsp_SPI2_ReadWrite(Dummy_byte);	 

	CS_1
	
    #if FREERTOS_ENABLED 	//如FREERTOS_ENABLED定义了,说明使用FreeRTOS了.
    if(xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) FLASH_UNLOCK();//解锁//OS运行中
    #endif
    
	return(ID);
}   	

/*******************************************************************************
* 函数名	: bsp_FLASH_Read
* 描述  	: 读数据(最大65535)
* 参数  	: -buf:数据缓存区指针
*			  -add:数据地址
*			  -len:读取长度
* 返回值	: 无
*******************************************************************************/
void bsp_FLASH_Read(u8* buf, u32 add,u16 len)   //为什么要限制长度为u16呢？
{ 
 	u16 i;   
	 
    #if FREERTOS_ENABLED 	//如FREERTOS_ENABLED定义了,说明使用FreeRTOS了.
    if(xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) FLASH_LOCK();//锁定//OS运行中
    #endif
    
	CS_0

    bsp_SPI2_ReadWrite(W25X_ReadData);    //发送读取命令   
	
    bsp_SPI2_ReadWrite((u8)((add)>>16));  //发送24bit地址    
    bsp_SPI2_ReadWrite((u8)((add)>>8));   
    bsp_SPI2_ReadWrite((u8)add);   
	
    for(i=0;i<len;i++)
	{ 
        buf[i] = bsp_SPI2_ReadWrite(Dummy_byte);   //循环读数
    }

	CS_1

    #if FREERTOS_ENABLED 	//如FREERTOS_ENABLED定义了,说明使用FreeRTOS了.
    if(xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) FLASH_UNLOCK();//解锁//OS运行中
    #endif
}

/*******************************************************************************
* 函数名	: bsp_FLASH_WritePage
* 描述  	: SPI在一页(0~65535)内写入少于256个字节的数据
* 参数  	: -buf:数据缓存区指针
*			  -add:数据地址
*			  -len:写入长度
* 返回值	: 无
*******************************************************************************/
static void bsp_FLASH_WritePage(u8* buf, u32 add, u16 len)
{
 	u16 i;  
	
    bsp_FLASH_WriteEnable();         
	
	CS_0
	   
    bsp_SPI2_ReadWrite(W25X_PageProgram); //发送写页命令   
	
    bsp_SPI2_ReadWrite((u8)((add)>>16)); //发送24bit地址    
    bsp_SPI2_ReadWrite((u8)((add)>>8));   
    bsp_SPI2_ReadWrite((u8)add);   
	
    for(i=0;i<len;i++)
	{
		bsp_SPI2_ReadWrite(buf[i]);//循环写数  
	}
	
	CS_1
	
	bsp_FLASH_WaitBusy();					   //等待写入结束
}

/*******************************************************************************
* 函数名	: bsp_FLASH_WriteNoCheck
* 描述  	: 无检验写SPI FLASH 
* 参数  	: -buf:数据缓存区指针
*			  -add:数据地址
*			  -len:写入长度
* 返回值	: 无
*******************************************************************************/
void bsp_FLASH_WriteNoCheck(u8* buf, u32 add, u16 len)
{ 			 		 
	u16 start;

	start = 256 - (add & 255); //单页剩余的字节数
			 	    
	if(len <= start)
	{
		start = len;//不大于256个字节
	}
    
    #if FREERTOS_ENABLED 	//如FREERTOS_ENABLED定义了,说明使用FreeRTOS了.
    if(xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) FLASH_LOCK();//锁定//OS运行中
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
				start = len; 	  //不够256个字节了
			}
		}
	}
    
    #if FREERTOS_ENABLED 	//如FREERTOS_ENABLED定义了,说明使用FreeRTOS了.
    if(xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) FLASH_UNLOCK();//解锁//OS运行中
    #endif
} 

/*******************************************************************************
* 函数名	: bsp_FLASH_Write
* 描述  	: 无检验写SPI FLASH 
* 参数  	: -pBuffer:数据存储区
*			  -WriteAddr:开始写入的地址(24bit)
*			  -NumByteToWrite:要写入的字节数(最大65535)
* 返回值	: 无
*******************************************************************************/
u8 SPI_FLASH_BUF[4096];
void bsp_FLASH_Write(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)   
{ 
	u32 secpos;
	u16 secoff;
	u16 secremain;	   
 	u16 i;    

	secpos=WriteAddr/4096;//扇区地址 0~511 for w25x16
	secoff=WriteAddr%4096;//在扇区内的偏移
	secremain=4096-secoff;//扇区剩余空间大小

	if(NumByteToWrite<=secremain)secremain=NumByteToWrite;//不大于4096个字节
        
	while(1) 
	{	
		bsp_FLASH_Read(SPI_FLASH_BUF,secpos*4096,4096);//读出整个扇区的内容
		for(i=0;i<secremain;i++)//校验数据
		{
			if(SPI_FLASH_BUF[secoff+i]!=0XFF)break;
		}
		if(i<secremain)//需要擦除
		{			
			bsp_FLASH_EraseSector(secpos*4096);//擦除这个扇区
			for(i=0;i<secremain;i++)	   //复制
			{
				SPI_FLASH_BUF[i+secoff]=pBuffer[i];	  
			}
			bsp_FLASH_WriteNoCheck(SPI_FLASH_BUF,secpos*4096,4096);//写入整个扇区

		}else bsp_FLASH_WriteNoCheck(pBuffer,WriteAddr,secremain);//写已经擦除了的,直接写入扇区剩余区间. 				   
		if(NumByteToWrite==secremain)break;//写入结束了
		else//写入未结束
		{
			secpos++;//扇区地址增1
			secoff=0;//偏移位置为0 	 

		   	pBuffer+=secremain;  //指针偏移
			WriteAddr+=secremain;//写地址偏移	   
		   	NumByteToWrite-=secremain;				//字节数递减
			if(NumByteToWrite>4096)secremain=4096;	//下一个扇区还是写不完
			else secremain=NumByteToWrite;			//下一个扇区可以写完了
		}	 
	}
}

/*******************************************************************************
* 函数名	: bsp_FLASH_EraseChip
* 描述  	: 擦除整个芯片,等待时间超长...
* 参数  	: 无
* 返回值	: 无
*******************************************************************************/
void bsp_FLASH_EraseChip(void)
{
    #if FREERTOS_ENABLED 	//如FREERTOS_ENABLED定义了,说明使用FreeRTOS了.
    if(xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) FLASH_LOCK();//锁定//OS运行中
    #endif
    
    bsp_FLASH_WriteEnable();                  //SET WEL 
    bsp_FLASH_WaitBusy();   

	CS_0

    bsp_SPI2_ReadWrite(W25X_ChipErase);        //发送片擦除命令  

	CS_1

	bsp_FLASH_WaitBusy();   				   //等待芯片擦除结束
    
    #if FREERTOS_ENABLED 	//如FREERTOS_ENABLED定义了,说明使用FreeRTOS了.
    if(xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) FLASH_UNLOCK();//解锁//OS运行中
    #endif
}

/*******************************************************************************
* 函数名	: bsp_FLASH_EraseSector
* 描述  	: 擦除一个扇区:150ms
* 参数  	: Dst_Addr:扇区地址
* 返回值	: 无
*******************************************************************************/
void bsp_FLASH_EraseSector(u32 Dst_Addr)   
{   
    #if FREERTOS_ENABLED 	//如FREERTOS_ENABLED定义了,说明使用FreeRTOS了.
    if(xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) FLASH_LOCK();//锁定//OS运行中
    #endif
    
    bsp_FLASH_WriteEnable();                  //SET WEL 	 
    bsp_FLASH_WaitBusy();   

	CS_0

    bsp_SPI2_ReadWrite(W25X_SectorErase);      //发送扇区擦除指令 
	
    bsp_SPI2_ReadWrite((u8)((Dst_Addr)>>16));  //发送24bit地址    
    bsp_SPI2_ReadWrite((u8)((Dst_Addr)>>8));   
    bsp_SPI2_ReadWrite((u8)Dst_Addr);

	CS_1

    #if FREERTOS_ENABLED 	//如FREERTOS_ENABLED定义了,说明使用FreeRTOS了.
    if(xTaskGetSchedulerState() == taskSCHEDULER_RUNNING)//OS运行中
    {
        vTaskDelay(150/portTICK_RATE_MS);//等待150ms
    }
    #endif
    
    bsp_FLASH_WaitBusy();   				   //等待擦除完成
    
    #if FREERTOS_ENABLED 	//如FREERTOS_ENABLED定义了,说明使用FreeRTOS了.
    if(xTaskGetSchedulerState() == taskSCHEDULER_RUNNING)  FLASH_UNLOCK();//解锁//OS运行中
    #endif
} 
