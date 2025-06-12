/*
 * Copyright (c) 2025 HPMicro
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

/*
 * 这是一个I2S多通道中断传输的示例程序
 * 该示例演示了如何使用中断方式实现I2S的4通道并行数据传输
 */

#include "board.h"
#include "hpm_i2s_drv.h"

/* I2S主设备配置 */
#define I2S_MASTER               HPM_I2S0
#define I2S_MASTER_CLOCK_NAME    clock_i2s0
#define I2S_MASTER_IRQ           IRQn_I2S0
#define I2S_TX_FIFO_THRESHOLD    (4)  /* I2S发送FIFO阈值设置 */

/* 音频数据配置结构体 */
typedef struct {
    uint32_t sample_rate;    /* 采样率(Hz) */
    uint8_t channel_num;     /* 通道数 (1: 单声道, 2: 立体声) */
    uint8_t audio_depth;     /* 音频位深 (16/32 bits) */
    const uint8_t *data;     /* 音频数据缓冲区指针 */
    uint32_t length;         /* 音频数据长度(字节) */
} audio_data_t;
audio_data_t audio_data;     /* 全局音频数据实例 */

/* 测试数据 */
uint32_t test_data[] = {
  0x11111111, 0x22222222, 0x33333333, 0x44444444, 0x55555555, 0x66666666, 0x77777777, 0x88888888,
  0x99999999, 0xAAAAAAAA, 0xBBBBBBBB, 0xCCCCCCCC, 0xDDDDDDDD, 0xEEEEEEEE, 0xFFFFFFFF, 0x5A5A5A5A,
};

/* 音频播放控制变量 */
volatile uint32_t audio_data_index;         /* 当前播放数据索引 */
volatile uint32_t audio_data_play_count;    /* 播放次数计数 */
volatile bool     audio_data_play_complete; /* 播放完成标志 */
volatile bool     i2s_tx_undeflow;         /* FIFO下溢标志 */


/*
 * I2S中断处理函数
 * 处理FIFO阈值中断，管理音频数据传输
 */
SDK_DECLARE_EXT_ISR_M(I2S_MASTER_IRQ, isr_i2s)
void isr_i2s(void)
{
    volatile uint32_t stat = i2s_get_irq_status(I2S_MASTER);
    int32_t data;

    /* 检查发送FIFO阈值中断 */
    /* 因为4个data line是同步的， 所以只需要判断其中一个通道的中断标志位即可 */
    if ((stat & (1 << I2S_STA_TX_DN_SHIFT)) != 0) {
        /* 填充FIFO直到阈值以上 */
        for (uint8_t i = 0; i < I2S_SOC_MAX_TX_FIFO_DEPTH - I2S_TX_FIFO_THRESHOLD; i++) {
            /* 将数据左对齐到32位寄存器 */
            data = (*((uint32_t *)&audio_data.data[audio_data_index])) << (32 - audio_data.audio_depth);
            /* 向4个通道发送相同的数据 */
            i2s_send_data(I2S_MASTER, 0, data);
            i2s_send_data(I2S_MASTER, 1, data);
            i2s_send_data(I2S_MASTER, 2, data);
            i2s_send_data(I2S_MASTER, 3, data);
            audio_data_index += audio_data.audio_depth / 8;  /* 更新数据索引，按字节计算 */
        }

        /* 检查是否播放完一轮 */
        if (audio_data_index >= audio_data.length) {
            audio_data_index = 0;
            audio_data_play_count++;
        }

        /* 检查是否达到指定播放次数 */
        if (audio_data_play_count >= 10) {
            audio_data_play_complete = true;
            i2s_disable_irq(I2S_MASTER, i2s_tx_fifo_threshold_irq_mask);
        }
    }

    /* 检查FIFO下溢中断 */
    if ((stat & (I2S_STA_TX_UD_MASK << I2S_STA_TX_UD_SHIFT)) != 0) {
        i2s_tx_undeflow = true;
        i2s_disable_irq(I2S_MASTER, i2s_tx_fifo_threshold_irq_mask);
    }

    /* 清除所有通道的中断标志位 */
    i2s_clear_irq_status(I2S_MASTER, stat);
}

/*
 * I2S主模式多通道配置函数
 * 配置I2S接口和传输参数
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
 * I2S主模式多通道中断传输测试函数
 */
void test_i2s_master_multiline_interrupt(void)
{
    /* 初始化控制变量 */
    audio_data_index = 0;
    audio_data_play_count = 0;
    audio_data_play_complete = false;
    i2s_tx_undeflow = false;

    i2s_master_multiline_config();

    /* 使能I2S中断 */
    i2s_enable_irq(I2S_MASTER, i2s_tx_fifo_threshold_irq_mask);
    intc_m_enable_irq_with_priority(I2S_MASTER_IRQ, 1);

    /* 启动I2S传输 */
    i2s_start(I2S_MASTER);

    /* 等待传输完成或出错 */
    while ((!audio_data_play_complete) && (!i2s_tx_undeflow)) {
        __asm("nop");
    }

    /* 此时尚有数据在FIFO中未完全发出，可以等待一段时间 */

    /* 停止I2S传输 */
    i2s_stop(I2S_MASTER);

    /* 检查传输结果 */
    if (i2s_tx_undeflow) {
        printf("ERROR: I2S play undeflow\n");
    } else {
        printf("I2S play done\n");
    }
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
    printf("I2S Master multiline interrupt example\n");

    /* 配置音频参数 */
    audio_data.sample_rate = 48000;    /* 采样率48kHz */
    audio_data.channel_num = 2;        /* 双声道 */
    audio_data.audio_depth = 32;       /* 32位数据宽度 */
    audio_data.data = (uint8_t *)test_data;
    audio_data.length = sizeof(test_data);

    /* 配置I2S时钟和引脚 */
    board_config_i2s_clock(I2S_MASTER, audio_data.sample_rate);
    init_i2s_multiline_pin();

    /* 执行I2S多通道中断传输测试 */
    test_i2s_master_multiline_interrupt();

    /* 主循环 */
    while (1) {
        __asm("nop");
    }

    return 0;
}
