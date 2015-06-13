/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for Petit FatFs (C)ChaN, 2009      */
/*-----------------------------------------------------------------------*/

#include "diskio.h"
#include "sdinterface.h"

/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (BYTE pdrv)
{
	// Put your code here
	DRESULT res;
	res = RES_OK;
//	if(init_card(2)==1)
	//	res = RES_ERROR;
	return res;
}

/*-----------------------------------------------------------------------*/
/* Read Partial Sector                                                   */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
		BYTE pdrv,		/* Physical drive nmuber (0..) */
		BYTE *buff,		/* Data buffer to store read data */
		DWORD sector,	/* Sector address (LBA) */
		BYTE count		/* Number of sectors to read (1..128) */
)
{
//	unsigned char buffer[10];
	DRESULT res;

	unsigned long int add2 = (unsigned long)(sector & 0xFFFF0000)>> 16;
	unsigned long int add1 = (sector & 0x0000FFFF);

	if(read_multibuffer(add1,add2,buff,count)==1)
//	if(read_buffer(add1,add2,buff,0,512)==1)


		res = RES_ERROR;

	// Put your code here
	res = RES_OK;
	return res;
}

/*-----------------------------------------------------------------------*/
/* Write Partial Sector                                                  */
/*-----------------------------------------------------------------------*/

DRESULT disk_write (
		BYTE pdrv,			/* Physical drive nmuber (0..) */
		BYTE *buff,	/* Data to be written */
		DWORD sector,		/* Sector address (LBA) */
		BYTE count			/* Number of sectors to write (1..128) */
)
{
	DRESULT res;
	unsigned long int add2 = (unsigned long)(sector & 0xFFFF0000)>> 16;
	unsigned long int add1 = (sector & 0x0000FFFF);
	if(write_multibuffer(add1,add2,count,buff)==1)
		res = RES_ERROR;
	res = RES_OK;
	return res;
}

//#if _USE_IOCTL
DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive data block */
)
{
	DRESULT res;
	BYTE csd[16];

	if (pdrv) return RES_PARERR;
	//if (Stat & STA_NOINIT) return RES_NOTRDY;

	res = RES_ERROR;
	switch (cmd) {
	case CTRL_SYNC :	/* Flush write-back cache, Wait for end of internal process */
		res = RES_OK;
		break;

	case GET_SECTOR_COUNT :	/* Get number of sectors on the disk (WORD) */
		*(DWORD*)buff = sectorsize;
		res = RES_OK;
		break;

	case GET_BLOCK_SIZE :	/* Get erase block size in unit of sectors (DWORD) */

		if ((cardtype >> 6) == 1) {	/* SDv2? */
				send_CMD55();
				send_ACMD13();		/* Read SD status */
				*(int *)&csd[0] = read16(0x10);
				*(int *)&csd[2] = read16(0x12);
				*(int *)&csd[4] = read16(0x14);
				*(int *)&csd[6] = read16(0x16);
				*(int *)&csd[8] = read16(0x18);
				*(int *)&csd[10] = read16(0x1A);
				*(int *)&csd[12] = read16(0x1C);
				*(int *)&csd[14] = read16(0x1E);

				*(DWORD*)buff = 16UL << (csd[14-10] >> 4);
					res = RES_OK;
		} else {					/* SDv1 or MMCv3 */
			send_CMD9();
			*(int *)&csd[0] = read16(0x10);
			*(int *)&csd[2] = read16(0x12);
			*(int *)&csd[4] = read16(0x14);
			*(int *)&csd[6] = read16(0x16);
			*(int *)&csd[8] = read16(0x18);
			*(int *)&csd[10] = read16(0x1A);
			*(int *)&csd[12] = read16(0x1C);
			*(int *)&csd[14] = read16(0x1E);
					*(DWORD*)buff = (((csd[14-10] & 63) << 1) + ((WORD)(csd[14-11] & 128) >> 7) + 1) << ((csd[14-13] >> 6) - 1);
				res = RES_OK;
			}
		break;

	default:
		res = RES_PARERR;
	}

	return res;
}

DSTATUS disk_status (
  BYTE pdrv     /* [IN] Physical drive number */
)
{
	return 0;
}
//#endif

DWORD get_fattime()
{
	return 0;
}
