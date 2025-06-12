# I2S Multi-line DMA Transfer Example

## Overview

- This example project demonstrates the functionality of multi-line DMA transfer using the I2S interface

## Requirements and Limitations

- Each I2S line supports independent DMA requests (e.g., HPM6P00 series)

## Working Principle

In I2S 4-line transmission mode:
- The system utilizes 4 independent DMA channels
- Each DMA channel specifically responds to the DMA transmission request of one I2S Line
- The DMA controller automatically transfers data from the source to the corresponding I2S Line's TXD register

## Hardware Requirements

- I2S pins need to be configured according to the actual hardware, and tools like logic analyzers should be used to observe pin waveforms

## Expected Results

After the program runs:
- All 4 I2S lines will begin data transmission simultaneously, and the pin waveforms can be observed as shown below:

![](doc/i2s_logic.png) 