# FATFS SPI Nor Flash

## 概述

- 该实例工程展示了使用FATFS文件系统读写nor flash存储器。
- 该实例的nor flash存储器使用的W25Q64JVSSIQ
- 组件serial_nor支持遵守sfdp的nor flash存储器，不限定W25Q64JVSSIQ
- 默认SPI SCLK频率为50M
- 默认SPI的IO模式为双线SPI

## 硬件设置
- [SPI引脚](lab_board_app_spi_pin)根据板子型号查看具体信息
- SPI引脚对应好nor flash(模块)引脚
- 可参考淘宝链接模块: https://item.taobao.com/item.htm?spm=a1z10.3-c-s.w4002-24145583300.20.7e233746j65hgo&id=43071752309
- 使用四线的时候，需要确保板载有引出SPI的IO2，IO3引脚并做好相关引脚初始化。

## 运行现象

- 确认硬件无误后，工程正确运行后

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

- 如果内置没有FAT格式，会自动格式化为FAT，需要等待格式化完毕

```console
Failed to mount SPI Nor flash, cause: There is no valid FAT volume
Formatting the SPI Nor flash, depending on the SPI Nor flash capacity, the formatting process may take a long time
Making file system is successful
SPI Nor flash has been mounted successfully
```

- 正常之后可以按照菜单进行测试

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