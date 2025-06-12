# I2S Multi-line Interrupt Transfer Example

## Overview

- This example project demonstrates the functionality of multi-line interrupt transfer using the I2S interface

## Requirements and Limitations

- None

## Working Principle

In I2S 4-line transmission mode:
- Configure I2S FIFO to generate interrupt requests
- Write data to TXD registers of multiple I2S lines in the interrupt handler function

## Hardware Requirements

- I2S pins need to be configured according to the actual hardware, and tools like logic analyzers should be used to observe pin waveforms

## Expected Results

After the program runs:
- All 4 I2S lines will begin data transmission simultaneously, and the pin waveforms can be observed as shown below:

![](doc/i2s_logic.png) 