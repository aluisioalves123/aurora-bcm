/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2025        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Basic definitions of FatFs */
#include "diskio.h"		/* Declarations FatFs MAI */

/* Example: Declarations of the platform and disk functions in the project */
#include "hal/sd_card/driver.h"

/* Example: Mapping of physical drive number for each drive */
#define DEV_FLASH	0	/* Map FTL to physical drive 0 */
#define DEV_MMC		1	/* Map MMC/SD card to physical drive 1 */
#define DEV_USB		2	/* Map USB MSD to physical drive 2 */
#define DEV_RAM		3	/* Map USB MSD to physical drive 2 */


/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
  if (pdrv != 0) return STA_NOINIT;

  if(!sd_health_check()) return STA_NOINIT;

  return 0;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
  if (pdrv != 0) return STA_NOINIT;

  sd_clock_reset_speed();

  sd_wake_up();
  
  if(!spi_mode_config()) return STA_NOINIT;

  if(!check_interface_condition()) return STA_NOINIT;

  if(!initialize_card()) return STA_NOINIT;

  sd_clock_speed_up();
	
  return 0;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	if (pdrv != 0) return RES_PARERR;
  sd_addressing_type_t addressing_type = sd_addressing_type();
  
  for(uint32_t i = 0; i < count; i++) {
    read_response_t current_read_response;
    uint32_t current_sector = sector + i;
    current_read_response = read_block(buff + i * SD_BLOCK_SIZE, current_sector, addressing_type);
    
    if (current_read_response != SD_READ_ACCEPTED) return RES_ERROR;
  }

  return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	if (pdrv != 0) return RES_PARERR;
  sd_addressing_type_t addressing_type = sd_addressing_type();
  
  for(uint32_t i = 0; i < count; i++) {
    write_response_t current_write_response;
    uint32_t current_sector = sector + i;
    current_write_response = write_block(buff + i * SD_BLOCK_SIZE, current_sector, addressing_type);
    
    if (current_write_response != SD_WRITE_ACCEPTED) return RES_ERROR;
  }

  return RES_OK;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
  (void)pdrv;
  (void)buff;

	switch (cmd)
  {
  case CTRL_SYNC:
    return RES_OK;
  default:
    return RES_PARERR;
  }
}

