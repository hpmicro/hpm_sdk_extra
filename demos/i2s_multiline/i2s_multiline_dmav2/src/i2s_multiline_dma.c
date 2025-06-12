/*
 * Copyright (c) 2025 HPMicro
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

/*
 * 这是一个I2S多通道DMA传输的示例程序
 * 该示例演示了如何使用DMA来实现I2S的4通道并行数据传输
 * 本示例使用DMAmux将一个DMA通道复用到4个I2S发送通道
 */

#include "board.h"
#include "hpm_i2s_drv.h"
#include "hpm_dmav2_drv.h"
#include "hpm_dmamux_drv.h"

/* DMA配置相关定义 */
#define TEST_I2S_DMA              HPM_XDMA
#define TEST_I2S_DMA_IRQ          IRQn_XDMA
#define TEST_I2S_DMA_TX_REQ       HPM_DMA_SRC_I2S0_TX

#define TEST_I2S_TX_DMA_CHANNEL     0   /* DMA通道号 */
#define TEST_I2S_TX_DMAMUX_CHANNEL  DMA_SOC_CHN_TO_DMAMUX_CHN(TEST_I2S_DMA, TEST_I2S_TX_DMA_CHANNEL)


/* I2S主设备配置 */
#define I2S_MASTER               HPM_I2S0
#define I2S_MASTER_CLOCK_NAME    clock_i2s0

#define I2S_TX_FIFO_THRESHOLD    (4)  /* I2S发送FIFO阈值设置 */

/* 音频数据配置结构体 */
typedef struct {
    uint32_t sample_rate;    /* 采样率(Hz) */
    uint8_t channel_num;     /* 通道数 (1: 单声道, 2: 立体声) */
    uint8_t audio_depth;     /* 音频位深 (32 bits) */
    const uint8_t *data;     /* 音频数据缓冲区指针 */
    uint32_t length;         /* 音频数据长度(字节) */
} audio_data_t;
audio_data_t audio_data;     /* 全局音频数据实例 */

/* 测试数据：每个数据重复4次，用于4个通道同步发送 */
uint32_t test_data[] = {
  0x11111111, 0x11111111, 0x11111111, 0x11111111, 0x22222222, 0x22222222, 0x22222222, 0x22222222,
  0x33333333, 0x33333333, 0x33333333, 0x33333333, 0x44444444, 0x44444444, 0x44444444, 0x44444444,
  0x55555555, 0x55555555, 0x55555555, 0x55555555, 0x66666666, 0x66666666, 0x66666666, 0x66666666,
  0x77777777, 0x77777777, 0x77777777, 0x77777777, 0x88888888, 0x88888888, 0x88888888, 0x88888888,
  0x99999999, 0x99999999, 0x99999999, 0x99999999, 0xAAAAAAAA, 0xAAAAAAAA, 0xAAAAAAAA, 0xAAAAAAAA,
  0xBBBBBBBB, 0xBBBBBBBB, 0xBBBBBBBB, 0xBBBBBBBB, 0xCCCCCCCC, 0xCCCCCCCC, 0xCCCCCCCC, 0xCCCCCCCC,
  0xDDDDDDDD, 0xDDDDDDDD, 0xDDDDDDDD, 0xDDDDDDDD, 0xEEEEEEEE, 0xEEEEEEEE, 0xEEEEEEEE, 0xEEEEEEEE,
  0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x5A5A5A5A, 0x5A5A5A5A, 0x5A5A5A5A, 0x5A5A5A5A,
};

/* DMA传输状态标志 */
volatile bool dma_transfer_done;    /* DMA传输完成标志 */
volatile bool dma_transfer_error;   /* DMA传输错误标志 */

/*
 * DMA中断处理函数
 * 处理DMA传输完成和错误事件
 */
SDK_DECLARE_EXT_ISR_M(TEST_I2S_DMA_IRQ, isr_dma)
void isr_dma(void)
{
    uint32_t stat;

    /* 检查并清除DMA传输状态 */
    stat = dma_check_transfer_status(TEST_I2S_DMA, TEST_I2S_TX_DMA_CHANNEL);

    if (stat & DMA_CHANNEL_STATUS_TC) {
        dma_transfer_done = true;  /* 传输完成 */
    } else if (stat & DMA_CHANNEL_STATUS_ERROR) {
        dma_transfer_error = true; /* 传输错误 */
    }
}

/*
 * I2S DMA传输配置函数
 * 配置DMA通道参数，使用burst模式， 一个dma通道完成4个数据线的发送
 */
hpm_stat_t i2s_dma_transfer_config(DMAV2_Type *dma_ptr, uint8_t dma_channel, audio_data_t *audio_data)
{
    hpm_stat_t stat;
    dma_channel_config_t ch_config;

    /* 验证音频位深为32位 */
    assert(audio_data->audio_depth == 32U);

    /* 配置DMA通道参数 */
    dma_default_channel_config(dma_ptr, &ch_config);
    ch_config.src_addr = core_local_mem_to_sys_address(HPM_CORE0, (uint32_t)audio_data->data); /* 源地址 */
    ch_config.dst_addr = (uint32_t)&I2S_MASTER->TXD[0];                                       /* 目标地址 */
    ch_config.src_width = DMA_TRANSFER_WIDTH_WORD;                                            /* 源数据宽度：32位 */
    ch_config.dst_width = DMA_TRANSFER_WIDTH_WORD;                                            /* 目标数据宽度：32位 */
    ch_config.src_addr_ctrl = DMA_ADDRESS_CONTROL_INCREMENT;                                  /* 源地址递增 */
    ch_config.dst_addr_ctrl = DMA_ADDRESS_CONTROL_INCREMENT;                                  /* 目标地址递增 */
    ch_config.size_in_byte = audio_data->length;                                             /* 传输数据长度 */
    ch_config.dst_mode = DMA_HANDSHAKE_MODE_HANDSHAKE;                                       /* 硬件握手模式 */
    ch_config.en_dst_burst_in_fixed_trans = true;                                            /* 使能burst传输 */
    ch_config.src_burst_size = DMA_NUM_TRANSFER_PER_BURST_4T;                               /* burst大小：4次传输 */

    /* 设置DMA通道 */
    stat = dma_setup_channel(dma_ptr, dma_channel, &ch_config, false);

    /* 配置DMAMUX，将DMA通道连接到I2S发送请求 */
    uint8_t dmamux_channel = DMA_SOC_CHN_TO_DMAMUX_CHN(dma_ptr, dma_channel);
    dmamux_config(BOARD_APP_DMAMUX, dmamux_channel, TEST_I2S_DMA_TX_REQ, true);

    return stat;
}

/*
 * I2S主模式多通道配置函数
 * 配置I2S 4通道进行发送
 */
void i2s_master_multiline_config(void)
{
    i2s_config_t i2s_config;
    i2s_multiline_transfer_config_t transfer;
    uint32_t i2s_mclk_hz;

    /* 配置I2S接口 */
    i2s_get_default_config(I2S_MASTER, &i2s_config);
    i2s_config.tx_fifo_threshold = I2S_TX_FIFO_THRESHOLD;  /* 设置发送FIFO阈值 */
    i2s_config.enable_mclk_out = true;                     /* 使能主时钟输出 */
    i2s_init(I2S_MASTER, &i2s_config);

    /* 配置I2S传输参数 */
    i2s_get_default_multiline_transfer_config(&transfer);
    transfer.sample_rate = audio_data.sample_rate;         /* 设置采样率 */
    transfer.channel_num_per_frame = 2;                    /* 每帧通道数 */
    transfer.audio_depth = audio_data.audio_depth;         /* 设置位深 */
    transfer.channel_length = i2s_channel_length_32_bits;  /* 通道长度 */
    transfer.master_mode = true;                           /* 主模式 */
    transfer.protocol = I2S_PROTOCOL_MSB_JUSTIFIED;        /* MSB对齐协议 */
    
    /* 使能4个发送数据线 */
    transfer.tx_data_line_en[0] = true;
    transfer.tx_data_line_en[1] = true;
    transfer.tx_data_line_en[2] = true;
    transfer.tx_data_line_en[3] = true;
    
    /* 配置通道槽掩码 */
    transfer.tx_channel_slot_mask[0] = (1 << audio_data.channel_num) - 1;
    transfer.tx_channel_slot_mask[1] = (1 << audio_data.channel_num) - 1;
    transfer.tx_channel_slot_mask[2] = (1 << audio_data.channel_num) - 1;
    transfer.tx_channel_slot_mask[3] = (1 << audio_data.channel_num) - 1;

    /* 配置I2S数据格式 */
    i2s_mclk_hz = clock_get_frequency(I2S_MASTER_CLOCK_NAME);
    if (status_success != i2s_config_multiline_transfer(I2S_MASTER, i2s_mclk_hz, &transfer)) {
        printf("I2S config failed!\n");
    }
}

/*
 * I2S主模式多通道DMA传输测试函数
 */
void test_i2s_master_multiline_dma(void)
{
    hpm_stat_t stat;

    i2s_master_multiline_config();

    /* 使能DMA中断 */
    intc_m_enable_irq_with_priority(TEST_I2S_DMA_IRQ, 1);

    /* 使能I2S DMA请求 */
    i2s_enable_tx_dma_request(I2S_MASTER);

    /* 执行10次DMA传输测试 */
    for (int i = 0; i < 10; i++) {
        dma_transfer_error = false;
        dma_transfer_done = false;

        /* 配置DMA传输 */
        stat = i2s_dma_transfer_config(TEST_I2S_DMA, TEST_I2S_TX_DMA_CHANNEL, &audio_data);
        if (status_success != stat) {
            printf("I2S DMA transfer config failed!\n");
            break;
        }

        /* 使能DMA通道 */
        dma_enable_channel(TEST_I2S_DMA, TEST_I2S_TX_DMA_CHANNEL);

        /* 首次传输时启动I2S */
        if (i == 0) {
            i2s_start(I2S_MASTER);
        }

        /* 等待传输完成或出错 */
        while ((!dma_transfer_error) && (!dma_transfer_done)) {
            __asm("nop");
        }

        if (dma_transfer_error) {
            printf("dma transfer i2s data failed\n");
            break;
        }
    }

    /* 此时尚有数据在FIFO中未完全发出，可以等待一段时间 */

    /* 停止I2S传输 */
    i2s_stop(I2S_MASTER);
}

/*
 * I2S多通道引脚初始化函数
 * 配置I2S相关的GPIO引脚功能
 */
void init_i2s_multiline_pin(void)
{
    /* 配置I2S引脚功能 */
    HPM_IOC->PAD[IOC_PAD_PB11].FUNC_CTL = IOC_PB11_FUNC_CTL_I2S0_MCLK;  /* 主时钟 */
    HPM_IOC->PAD[IOC_PAD_PB01].FUNC_CTL = IOC_PB01_FUNC_CTL_I2S0_BCLK;  /* 位时钟 */
    HPM_IOC->PAD[IOC_PAD_PB10].FUNC_CTL = IOC_PB10_FUNC_CTL_I2S0_FCLK;  /* 帧时钟 */
    HPM_IOC->PAD[IOC_PAD_PB00].FUNC_CTL = IOC_PB00_FUNC_CTL_I2S0_TXD_0; /* 数据线0 */
    HPM_IOC->PAD[IOC_PAD_PB03].FUNC_CTL = IOC_PB03_FUNC_CTL_I2S0_TXD_1; /* 数据线1 */
    HPM_IOC->PAD[IOC_PAD_PB05].FUNC_CTL = IOC_PB05_FUNC_CTL_I2S0_TXD_2; /* 数据线2 */
    HPM_IOC->PAD[IOC_PAD_PB02].FUNC_CTL = IOC_PB02_FUNC_CTL_I2S0_TXD_3; /* 数据线3 */
}

/*
 * 主函数
 */
int main(void)
{
    /* 初始化板级设备和时钟 */
    board_init();
    printf("I2S Master multiline DMA example\n");

    /* 配置音频参数 */
    audio_data.sample_rate = 48000;    /* 采样率48kHz */
    audio_data.channel_num = 2;        /* 双声道 */
    audio_data.audio_depth = 32;       /* 32位数据宽度 */
    audio_data.data = (uint8_t *)test_data;
    audio_data.length = sizeof(test_data);

    /* 配置I2S时钟和引脚 */
    board_config_i2s_clock(I2S_MASTER, audio_data.sample_rate);
    init_i2s_multiline_pin();

    /* 执行I2S多通道DMA传输测试 */
    test_i2s_master_multiline_dma();

    /* 主循环 */
    while (1) {
        __asm("nop");
    }

    return 0;
}
