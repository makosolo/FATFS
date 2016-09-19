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
#define EX_FLASH    0	//���0���ⲿflash
#define USB         1   //���1��USB�洢�豸

#define FLASH_SECTOR_SIZE 	512  //�����Ĵ�С
#define FLASH_BLOCK_SIZE  	8    //ÿ����������8������

u16 FLASH_SECTOR_COUNT = 2048*6; //����������2048����/1M��6M�ռ�

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
        case EX_FLASH: //�ⲿflash
        {
            bsp_InitFLASH();
            return 0;
        }//break;

        case USB: //USB�洢�豸
        {
//            result = USB_disk_initialize();
            return 0;
        }//break;
        
        default: break;//��֧��
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

    if (!count) return RES_PARERR;//count���ܵ���0�����򷵻ز�������
    
	switch (pdrv) 
    {
        case EX_FLASH: //�ⲿflash
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
        
        default: //��֧��
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

    if (!count) return RES_PARERR;//count���ܵ���0�����򷵻ز�������
    
	switch (pdrv) 
    {
        case EX_FLASH: //�ⲿflash
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
        
        default: break;//��֧��
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
        case EX_FLASH: //�ⲿflash
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
        
        default: //�����Ĳ�֧��
        {
            res = RES_ERROR;
        }break;
	}

	return res;
}
#endif
