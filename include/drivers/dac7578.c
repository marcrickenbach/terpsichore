/**
 * @file Driver for DAC7578 I2C-based 8-Channel DAC.
 */


#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>

#include "dac7578.h"

#define LOG_LEVEL CONFIG_GPIO_LOG_LEVEL
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(i2c_dac7578);



#define I2C_NODE            DT_NODELABEL(i2c2)
#define GPIO_PINS           DT_PATH(zephyr_user)
#define LDAC_PIN	        ldac_gpios

static const struct gpio_dt_spec ldac = GPIO_DT_SPEC_GET(GPIO_PINS, LDAC_PIN);
const struct device *i2c_dev;

int dac7578_configure_ldac () {
    int ret; 
    if (!device_is_ready(ldac.port)) {
        printk("LDAC GPIO device is not ready\n");
        return -ENODEV;
    }

    // Configure the GPIO pin
    ret = gpio_pin_configure(ldac.port, ldac.pin, GPIO_OUTPUT | GPIO_ACTIVE_LOW);
    if (ret != 0) {
        printk("Failed to configure LDAC GPIO pin\n");
        return ret;
    }

    ret = gpio_pin_set(ldac.port, ldac.pin, 1);
    if (ret != 0) {
        printk("Failed to set LDAC GPIO pin\n");
        return ret;
    }
    return 0;
}

int dac7578_init(void) {
    i2c_dev = DEVICE_DT_GET(I2C_NODE);
    if (!device_is_ready(i2c_dev)) {
        printk("I2C device is not ready\n");
        return -1;
    }

    int ret = i2c_configure(i2c_dev, I2C_SPEED_SET(I2C_SPEED_FAST));
    if (ret != 0) {
        printk("Failed to configure I2C\n");
        return -1;
    }

    ret = dac7578_configure_ldac();
    if (ret != 0) {
        printk("Failed to configure LDAC\n");
        return -1;
    }

    printk("DAC7578 I2C initialized successfully\n");
    return 0;
}


int dac7578_configure(uint16_t configuration) {
    // Example: if the DAC requires configuration, you can write the configuration value
    // For DAC7578, configuration might be minimal, so this is likely unused
    return 0;
}


int dac7578_write(uint8_t channel, uint16_t value) {
    uint8_t buffer[3];

    // Format the buffer according to the DAC7578 protocol
    buffer[0] = (0x00 << 4) | (channel & 0x07);  // Channel n (A-H)
    buffer[1] = (value >> 4) & 0xFF;    // Upper 8 bits of the 12-bit DAC value
    buffer[2] = (value & 0x0F) << 4;    // Lower 4 bits of the DAC value, shifted left

    // Send the buffer over I2C to the DAC
    int ret = i2c_write(i2c_dev, buffer, sizeof(buffer), DAC7578_ADDR);
    if (ret != 0) {
        printk("I2C write failed\n");
        return -1;
    }

    return 0;
}