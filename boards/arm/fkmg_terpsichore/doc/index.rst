.. _stm32f4_disco_board:

FKMG Terpsichore
####################

Overview
********

Terpsichore makes use of an ARM Cortex-M4 based STM32F405RGT6 MCU:

- STM32F405RGT6 in LQFP64 package
- ARM 32-bit Cortex -M4 CPU with FPU
- 168 MHz max CPU frequency
- VDD from 1.8 V to 3.6 V
- 1 MB Flash
- 192+4 KB SRAM including 64-Kbyte of core coupled memory
- GPIO with external interrupt capability
- 3x12-bit ADC with 24 channels
- 2x12-bit D/A converters
- RTC
- Advanced-control Timer
- General Purpose Timers (17)
- Watchdog Timers (2)
- USART/UART (6)
- I2C (3)
- SPI (3)
- SDIO
- 2xCAN
- USB 2.0 OTG FS with on-chip PHY
- USB 2.0 OTG HS/FS with dedicated DMA, on-chip full-speed PHY and ULPI
- 10/100 Ethernet MAC with dedicated DMA
- 8- to 14-bit parallel camera
- CRC calculation unit
- True random number generator
- DMA Controller


Pin Mapping
===========

- i2c2 : SDA: PB11,
         SCL: PB10

- spi2 : MISO: PB14,
         MOSI: PB15,
         SCK: PB13

- dac  : DAC1: PA4,
         DAC2: PA5,

- adc  : ADC1: PA0,
         ADC2: PA1,
         ADC3: PA2,
         ADC4: PA3,
         ADC5: PA6,
         ADC6: PA7,
         ADC7: PB0,
         ADC8: PB1,
         ADC9: PC0

System Clock
============

System Clock is driven by PLL clock at 168MHz,
driven by 8MHz high speed external clock.
