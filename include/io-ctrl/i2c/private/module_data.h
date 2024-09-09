/*******************************************************************************
 * @brief FKMG I2C Control module data interface.
 *
 * This is the private module data.
 */

#ifndef FKMG_IO_CTRL_I2C_MODULE_DATA_H
#define FKMG_IO_CTRL_I2C_MODULE_DATA_H

#ifdef __cplusplus
extern "C" {
#endif

/* *****************************************************************************
 * Includes
 */

#include <zephyr/kernel.h>

/* *****************************************************************************
 * Enums
 */

/* *****************************************************************************
 * Structs
 */

struct fkmg_i2c_module_data{
    bool initialized;

    /* Singly linked lists to keep track of things. */
    struct{
        sys_slist_t instances;
    }list;
};

#ifdef __cplusplus
}
#endif

#endif /* FKMG_IO_CTRL_I2C_MODULE_DATA_H */