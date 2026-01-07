/*
 * Copyright (c) 2021-2023 HPMicro
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "ffconf.h"
#include "diskio.h"
#include "hpm_l1c_drv.h"
#include "hpm_serial_nor.h"
#include "hpm_serial_nor_host_port.h"
#include "board.h"

#if defined(USE_FREERTOS) && USE_FREERTOS
#include "FreeRTOS.h"
#include "task.h"
#endif

#define MAX_ALIGNED_BUF_SIZE (4096U)

ATTR_ALIGN(HPM_L1C_CACHELINE_SIZE) uint8_t g_aligned_buf[MAX_ALIGNED_BUF_SIZE];
static hpm_serial_nor_t nor_flash_dev = {0};
static hpm_serial_nor_info_t flash_info;
static hpm_stat_t nor_init_sta;

DSTATUS spi_nor_disk_initialize(BYTE pdrv)
{
    if (pdrv != DEV_SPI_NOR) {
        return STA_NOINIT;
    }
    serial_nor_get_board_host(&nor_flash_dev.host);
    board_init_spi_clock(nor_flash_dev.host.host_param.param.host_base);
    serial_nor_spi_pins_init(nor_flash_dev.host.host_param.param.host_base);
    nor_init_sta = hpm_serial_nor_init(&nor_flash_dev, &flash_info);
    return (nor_init_sta == status_success) ? RES_OK : RES_ERROR;
}

DSTATUS spi_nor_disk_deinitialize(BYTE pdrv)
{
    if (pdrv != DEV_SPI_NOR) {
        return STA_NOINIT;
    }
    /* currently there is no need to operate */
    return RES_OK;
}

DSTATUS spi_nor_disk_status(BYTE pdrv)
{
    if (pdrv != DEV_SPI_NOR) {
        return STA_NOINIT;
    }
    return (nor_init_sta == status_success) ? RES_OK : RES_NOTRDY;
}

DSTATUS spi_nor_disk_read(BYTE pdrv, BYTE *buff, LBA_t sector, UINT count)
{
    if (pdrv != DEV_SPI_NOR) {
        return RES_PARERR;
    }
    hpm_stat_t stat;
    uint32_t sector_size = FF_MAX_SS;
    uint32_t remaining_size = sector_size * count;
    uint32_t start_sector_addr = sector_size * sector;
    if (((uint32_t)buff % HPM_L1C_CACHELINE_SIZE) != 0) {
        uint32_t sys_aligned_buf_addr = core_local_mem_to_sys_address(BOARD_RUNNING_CORE, (uint32_t)&g_aligned_buf);
        while (remaining_size > 0) {
            uint32_t read_size = MIN(MAX_ALIGNED_BUF_SIZE, remaining_size);
            stat = hpm_serial_nor_read(&nor_flash_dev, (uint8_t *) sys_aligned_buf_addr, read_size, start_sector_addr);
            if (stat != status_success) {
                return RES_ERROR;
            }
            l1c_dc_invalidate(sys_aligned_buf_addr, read_size);
            memcpy(buff, g_aligned_buf, read_size);
            buff += read_size;
            start_sector_addr += read_size;
            remaining_size -= read_size;
        }
    } else {
        stat = hpm_serial_nor_read(&nor_flash_dev, (uint8_t *) buff, remaining_size, start_sector_addr);
        if (stat != status_success) {
            return RES_ERROR;
        }
    }
    return RES_OK;
}

DSTATUS spi_nor_disk_write(BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count)
{
    if (pdrv != DEV_SPI_NOR) {
        return RES_PARERR;
    }
    hpm_stat_t stat;
    uint32_t sector_size = FF_MAX_SS;
    uint32_t remaining_size = sector_size * count;
    uint32_t start_sector_addr = sector_size * sector;
    uint16_t page_size = nor_flash_dev.flash_info.page_size;
    stat = hpm_serial_nor_erase_blocking(&nor_flash_dev, start_sector_addr, remaining_size);
    if (stat != status_success) {
        return RES_ERROR;
    }
    if (((uint32_t)buff % HPM_L1C_CACHELINE_SIZE) != 0) {
        uint32_t sys_aligned_buf_addr = core_local_mem_to_sys_address(BOARD_RUNNING_CORE, (uint32_t)&g_aligned_buf);
        while (remaining_size > 0) {
            uint32_t write_size = MIN(page_size, remaining_size);
            memcpy((uint8_t *)&g_aligned_buf, buff, write_size);
            l1c_dc_flush(sys_aligned_buf_addr, write_size);
            stat = hpm_serial_nor_page_program_nonblocking(&nor_flash_dev, (uint8_t *)sys_aligned_buf_addr, write_size, start_sector_addr);
            if (stat != status_success) {
                return RES_ERROR;
            }
            while (hpm_serial_nor_is_busy(&nor_flash_dev) == status_spi_nor_flash_is_busy) {
#if defined(USE_FREERTOS) && USE_FREERTOS
                vTaskDelay(pdMS_TO_TICKS(1));
#endif
            };
            buff += write_size;
            start_sector_addr += write_size;
            remaining_size -= write_size;
        }
    } else {
        stat = hpm_serial_nor_program_blocking(&nor_flash_dev, (uint8_t *) buff, remaining_size, start_sector_addr);
        if (stat != status_success) {
            return RES_ERROR;
        }
    }

    return RES_OK;
}

DRESULT spi_nor_disk_ioctl(BYTE pdrv, BYTE cmd, void *buff)
{
    DRESULT result = RES_PARERR;
    do {
        HPM_BREAK_IF((pdrv != DEV_SPI_NOR) || ((cmd != CTRL_SYNC) && (buff == NULL)));
        result = RES_OK;
        switch (cmd) {
        case GET_SECTOR_COUNT:
            *(uint32_t *) buff = (flash_info.size_in_kbytes / flash_info.sector_size_kbytes) * 1024;
            break;
        case GET_SECTOR_SIZE:
            *(uint32_t *) buff = FF_MAX_SS;
            break;
        case GET_BLOCK_SIZE:
            *(uint32_t *) buff = (flash_info.sector_size_kbytes * 1024);
            break;
        case CTRL_SYNC:
            result = RES_OK;
            break;
        default:
            result = RES_PARERR;
            break;
        }

    } while (false);

    return result;
}
