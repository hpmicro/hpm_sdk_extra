# FATFS SPI Nor Flash

## Overview

- This example project demonstrates reading and writing to nor flash storage using the FATFS file system.
- The nor flash storage used in this example is W25Q64JVSSIQ
- The serial_nor component supports nor flash storage that complies with sfdp, not limited to W25Q64JVSSIQ
- Default SPI SCLK frequency is 50M
- Default SPI IO mode is dual-line SPI

## Hardware Setup
- Check [SPI pins](lab_board_app_spi_pin) for specific information according to the board model
- Connect SPI pins to the nor flash (module) pins accordingly
- Reference module on Taobao: https://item.taobao.com/item.htm?spm=a1z10.3-c-s.w4002-24145583300.20.7e233746j65hgo&id=43071752309
- When using quad-line mode, ensure that the board has IO2 and IO3 pins of SPI exposed and perform relevant pin initialization.

## Running Results

- After confirming the hardware is correct and the project runs successfully

```console
hpm_sdk: 1.11.0
----------------------------------------------------------------------
$$\   $$\ $$$$$$$\  $$\      $$\ $$\
$$ |  $$ |$$  __$$\ $$$\    $$$ |\__|
$$ |  $$ |$$ |  $$ |$$$$\  $$$$ |$$\  $$$$$$$\  $$$$$$\   $$$$$$\
$$$$$$$$ |$$$$$$$  |$$\$$\$$ $$ |$$ |$$  _____|$$  __$$\ $$  __$$\
$$  __$$ |$$  ____/ $$ \$$$  $$ |$$ |$$ /      $$ |  \__|$$ /  $$ |
$$ |  $$ |$$ |      $$ |\$  /$$ |$$ |$$ |      $$ |      $$ |  $$ |
$$ |  $$ |$$ |      $$ | \_/ $$ |$$ |\$$$$$$$\ $$ |      \$$$$$$  |
\__|  \__|\__|      \__|     \__|\__| \_______|\__|       \______/
----------------------------------------------------------------------
the flash size:8192 KB
the flash page_size:256 Byte
the flash sector_size:4 KB
the flash block_size:64 KB
the flash sector_erase_cmd:0x20
the flash block_erase_cmd:0xd8
spi nor flash init ok
```

- If there is no FAT format built-in, it will automatically format to FAT, please wait for the formatting to complete

```console
Failed to mount SPI Nor flash, cause: There is no valid FAT volume
Formatting the SPI Nor flash, depending on the SPI Nor flash capacity, the formatting process may take a long time
Making file system is successful
SPI Nor flash has been mounted successfully
```

- After normal operation, you can test according to the menu

```console

---------------------------------------------------------------
*                                                             *
*                   spi nor flash fatfs demo                  *
*                                                             *
*        1 - Create hello.txt                                 *
*        2 - Read 1st line from hello.txt                     *
*        3 - Directory related test                           *
*        4 - Large file write test                            *
*        5 - format fs system                            *
*-------------------------------------------------------------*
1
Create new file successfully, status=0
Write file　operation is successfully
2
Open file successfully
Hello, this is SPI SPI Nor flash FATFS demo

3
Creating new directory succeeded
Removing new directory succeeded

4
Create new file successfully, now writing.....
Write file operation is successful

```
