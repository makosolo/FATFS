#ifndef __BSP_FLASH_H
#define __BSP_FLASH_H    

#define FLASH_LOCK()      xSemaphoreTake(xQueueHandle_FLASH, portMAX_DELAY)//锁定
#define FLASH_UNLOCK()    xSemaphoreGive(xQueueHandle_FLASH)//解锁

//W25X系列/Q系列芯片列表
#define W25Q80 	0XEF13 	
#define W25Q16 	0XEF14
#define W25Q32 	0XEF15
#define W25Q64 	0XEF16
#define W25Q128	0XEF17

#define    W25X_WriteEnable 		0x06 
#define    W25X_WriteDisable		0x04 
#define    W25X_ReadStatusReg		0x05 
#define    W25X_WriteStatusReg		0x01 
#define    W25X_ReadData			0x03 
#define    W25X_FastReadData		0x0B 
#define    W25X_FastReadDual		0x3B 
#define    W25X_PageProgram	    	0x02 
#define    W25X_BlockErase			0xD8 
#define    W25X_SectorErase		    0x20 
#define    W25X_ChipErase			0xC7 
#define    W25X_PowerDown			0xB9 
#define    W25X_ReleasePowerDown	0xAB 
#define    W25X_DeviceID			0xAB 
#define    W25X_ManufactDeviceID	0x90 
#define    W25X_JedecDeviceID		0x9F 

extern u16 W25QXX_TYPE;

void bsp_InitFLASH(void);//SPI_FLASH初始化

u16 bsp_FLASH_ReadID(void);//读取芯片ID
void bsp_FLASH_Read(u8* buf, u32 add,u16 len);//读数据
void bsp_FLASH_WriteNoCheck(u8* buf, u32 add, u16 len);//无检验写SPI FLASH 
void bsp_FLASH_Write(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite);//有校验写
void bsp_FLASH_EraseChip(void);//擦除整个芯片
void bsp_FLASH_EraseSector(u32 Dst_Addr);//擦除一个扇区

void bsp_FLASH_WriteData(u8 *buf, u32 add, u16 len);//把采集数据写入FLASH，只能向前写

#endif //__BSP_FLASH_H
















