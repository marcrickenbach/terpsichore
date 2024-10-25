#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>

#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/dma.h>
#include <zephyr/drivers/gpio.h>


/* Address is dependent on address bits for 
 * daisy-chaining. FIXME: how do we take this
 * into account? */
#define DAC7578_ADDR 0x48

int dac7578_configure_ldac ();
int dac7578_init(void);
int dac7578_configure(uint16_t configuration);
int dac7578_write(uint8_t channel, uint16_t value);