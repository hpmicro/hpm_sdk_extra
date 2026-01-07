# Copyright (c) 2021-2025  HPMicro
# SPDX-License-Identifier: BSD-3-Clause
sdk_inc(port)
sdk_inc(spi_flash/common/port)
sdk_inc(spi_flash/common/port/${BOARD})
sdk_app_src(port/diskio.c)
sdk_app_src(port/spi_nor_disk.c)

