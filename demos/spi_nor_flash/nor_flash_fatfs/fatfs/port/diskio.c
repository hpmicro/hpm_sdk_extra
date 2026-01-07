/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various existing       */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

/*
 * Copyright (c) 2021-2023 HPMicro
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "diskio.h"          /* Declarations of disk functions */

#include "spi_nor_disk.h"

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/
DSTATUS disk_status(
        BYTE pdrv   /* Physical drive number to identify the drive */
)
{
    DSTATUS stat = STA_NOINIT;

    switch (pdrv) {
    case DEV_SPI_NOR :
        stat = spi_nor_disk_status(pdrv);
        break;

    default:
        break;
    }

    return stat;
}

/*-----------------------------------------------------------------------*/
/* Initialize a Drive                                                    */
/*-----------------------------------------------------------------------*/
void disk_deinitialize(
        BYTE pdrv   /* Physical drive number to identify the drive */
)
{
    switch (pdrv) {
    case DEV_SPI_NOR :
        spi_nor_disk_deinitialize(pdrv);
        break;

    default:
        break;
    }
}

DSTATUS disk_initialize(
        BYTE pdrv   /* Physical drive number to identify the drive */
)
{
    DSTATUS stat = STA_NOINIT;

    switch (pdrv) {
    case DEV_SPI_NOR :
        stat = spi_nor_disk_initialize(pdrv);
        break;

    default:
        break;
    }

    return stat;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/
DRESULT disk_read(
        BYTE pdrv,      /* Physical drive number to identify the drive */
        BYTE *buff,     /* Data buffer to store read data */
        LBA_t sector,   /* Start sector in LBA */
        UINT count      /* Number of sectors to read */
)
{
    DRESULT res = RES_ERROR;

    switch (pdrv) {
    case DEV_SPI_NOR :
        res = spi_nor_disk_read(pdrv, buff, sector, count);
        break;

    default:
        res = RES_PARERR;
        break;
    }

    return res;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write(
        BYTE pdrv,          /* Physical drive number to identify the drive */
        const BYTE *buff,   /* Data to be written */
        LBA_t sector,       /* Start sector in LBA */
        UINT count          /* Number of sectors to write */
)
{
    DRESULT res = RES_ERROR;

    switch (pdrv) {
    case DEV_SPI_NOR :
        res = spi_nor_disk_write(pdrv, buff, sector, count);
        break;

    default:
        res = RES_PARERR;
        break;
    }

    return res;
}

#endif

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/
DRESULT disk_ioctl(
        BYTE pdrv,      /* Physical drive number (0..) */
        BYTE cmd,       /* Control code */
        void *buff      /* Buffer to send/receive control data */
)
{
    DRESULT res = RES_ERROR;

    switch (pdrv) {
    case DEV_SPI_NOR :
        res = spi_nor_disk_ioctl(pdrv, cmd, buff);
        break;

    default:
        res = RES_ERROR;
        break;
    }

    return res;
}
