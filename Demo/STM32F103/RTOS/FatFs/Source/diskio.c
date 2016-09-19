/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2014        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/
#include "sys.h"
#include "bsp_flash.h"

#include "diskio.h"		/* FatFs lower layer API */

/* Definitions of physical drive number for each drive */
#define EX_FLASH    0	//卷标0，外部flash
#define USB         1   //卷标1，USB存储设备

#define FLASH_SECTOR_SIZE 	512  //扇区的大小
#define FLASH_BLOCK_SIZE  	8    //每个擦出块有8个扇区

u16 FLASH_SECTOR_COUNT = 2048*6; //总扇区数，2048扇区/1M，6M空间

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
    return 0;
}

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
//	DSTATUS stat;
//	int result;

	switch (pdrv) 
    {
        case EX_FLASH: //外部flash
        {
            bsp_InitFLASH();
            return 0;
        }//break;

        case USB: //USB存储设备
        {
//            result = USB_disk_initialize();
            return 0;
        }//break;
        
        default: break;//不支持
	}
    
	return STA_NOINIT;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address in LBA */
	UINT count		/* Number of sectors to read */
)
{
//	DRESULT res;
//	int result;

    if (!count) return RES_PARERR;//count不能等于0，否则返回参数错误
    
	switch (pdrv) 
    {
        case EX_FLASH: //外部flash
        {
            while(count--)
            {
                bsp_FLASH_Read(buff, sector*FLASH_SECTOR_SIZE, FLASH_SECTOR_SIZE);
                sector++;
                buff += 512;
            }
            
            return RES_OK;
        }//break;

        case USB :
        {
		// translate the arguments here

//		result = USB_disk_read(buff, sector, count);

		// translate the reslut code here
            return RES_OK;
        }//break;
        
        default: //不支持
        {
            return RES_ERROR;
        }//break;
	}
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _USE_WRITE
DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address in LBA */
	UINT count			/* Number of sectors to write */
)
{
//	DRESULT res;
//	int result;

    if (!count) return RES_PARERR;//count不能等于0，否则返回参数错误
    
	switch (pdrv) 
    {
        case EX_FLASH: //外部flash
        {
			for(;count>0;count--)
			{
				bsp_FLASH_Write((u8*)buff, sector*FLASH_SECTOR_SIZE, FLASH_SECTOR_SIZE);
				sector++;
				buff+=FLASH_SECTOR_SIZE;
			}
            
            return RES_OK;
        }//break;
        
        case USB:
        {
            // translate the arguments here

//            result = USB_disk_write(buff, sector, count);

            // translate the reslut code here

            return RES_OK;
        }//break;
        
        default: break;//不支持
	}

	return RES_PARERR;
}
#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL
DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;
//	int result;

	switch (pdrv) 
    {
        case EX_FLASH: //外部flash
        {
            switch(cmd)
            {
                case CTRL_SYNC:
                {
                    res = RES_OK; 
                }break;
                
                case GET_SECTOR_SIZE:
                {
                    *(WORD*)buff = FLASH_SECTOR_SIZE;
                    res = RES_OK;
                }break;
                
                case GET_BLOCK_SIZE:
                {
                    *(WORD*)buff = FLASH_BLOCK_SIZE;
                    res = RES_OK;
                }break;
                
                case GET_SECTOR_COUNT:
                {
                    *(DWORD*)buff = FLASH_SECTOR_COUNT;
                    res = RES_OK;
                }break;
                
                default:
                {
                    res = RES_PARERR;
                }break;
            }
        }break;
        
        case USB :
        {
            // Process of the command the USB drive
            return res;
        }//break;
        
        default: //其他的不支持
        {
            res = RES_ERROR;
        }break;
	}

	return res;
}
#endif
