/*******************************************************************************
 * @brief FKMG I2C Control interface.
 *
 * This module is the public api to interface with the Zephyr I2C Driver.
 *
 * @example
 */

#ifndef FKMG_IO_CTRL_I2C_H
#define FKMG_IO_CTRL_I2C_H

/* In case C++ needs to use anything here */
#ifdef __cplusplus
extern “C” {
#endif

/* *****************************************************************************
 * Includes
 */

#include <zephyr/kernel.h>

#include "instance.h"
#include "instance_cfg.h"
#include "listener.h"
#include "listener_cfg.h"


/* *****************************************************************************
 * Structs
 */

/* *****************************************************************************
 * Public
 */


/**
 * Initialize an instance. An instance contains everything required to declare
 * and define all the data/memory for threads, queues, etc., that runs the the
 * engine. Even though it might be possible to have multiple instances,
 * typically only one is required.
 * @param[in] p_cfg Pointer to the filled-in configuration struct. See the
 * struct for details.
 */
void FKMG_IO_CTRL_I2C_Init_Instance(struct FKMG_IO_CTRL_I2C_Instance_Cfg * p_cfg);

void FKMG_IO_CTRL_I2C_Add_Listener(struct FKMG_IO_CTRL_I2C_Listener_Cfg * p_cfg);


#ifdef __cplusplus
}
#endif

#endif /* FKMG_IO_CTRL_I2C_H */