#include <zephyr.h>
#include <device.h>
#include "stm32f4xx_hal.h"

#define DAC7578_I2C_ADDRESS 0x4C

I2C_HandleTypeDef hi2c1;
DMA_HandleTypeDef hdma_i2c1_tx;


int dac7578_init(I2C_HandleTypeDef * hi2c1,
                 DMA_HandleTypeDef * hdma_i2c1_tx) {
    __HAL_RCC_I2C1_CLK_ENABLE();  // Enable I2C clock
    
    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = 100000;  // 100kHz standard I2C speed
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
        printk("I2C initialization failed\n");
        return -1;
    }

    // Optionally: Initialize DMA for I2C if needed
    hdma_i2c1_tx.Instance = DMA1_Stream6;
    hdma_i2c1_tx.Init.Channel = DMA_CHANNEL_1;
    hdma_i2c1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_i2c1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_i2c1_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_i2c1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_i2c1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_i2c1_tx.Init.Mode = DMA_NORMAL;
    hdma_i2c1_tx.Init.Priority = DMA_PRIORITY_LOW;

    if (HAL_DMA_Init(&hdma_i2c1_tx) != HAL_OK) {
        printk("DMA initialization failed\n");
        return -1;
    }

    __HAL_LINKDMA(&hi2c1, hdmatx, hdma_i2c1_tx);

    return 0;
}


// Function to set the DAC value using HAL I2C
int dac7578_set_value(I2C_HandleTypeDef * hi2c1, 
                      uint8_t channel, 
                      uint16_t value) {
    uint8_t buffer[3];

    buffer[0] = (channel & 0x07) << 4;
    buffer[1] = (value >> 4) & 0xFF;
    buffer[2] = (value & 0x0F) << 4;

    if (HAL_I2C_Master_Transmit(&hi2c1, DAC7578_I2C_ADDRESS << 1, buffer, sizeof(buffer), HAL_MAX_DELAY) != HAL_OK) {
        printk("I2C write failed\n");
        return -1;
    }

    return 0;
}


// Function to set DAC value using DMA
int dac7578_set_value_dma(I2C_HandleTypeDef * hi2c1, 
                          uint8_t channel, 
                          uint16_t value) {
    uint8_t buffer[3];

    buffer[0] = (channel & 0x07) << 4;
    buffer[1] = (value >> 4) & 0xFF;
    buffer[2] = (value & 0x0F) << 4;

    if (HAL_I2C_Master_Transmit_DMA(&hi2c1, DAC7578_I2C_ADDRESS << 1, buffer, sizeof(buffer)) != HAL_OK) {
        printk("I2C DMA write failed\n");
        return -1;
    }

    return 0;
}
