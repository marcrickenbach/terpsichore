/*******************************************************************************
 * @brief FKMG LED Control interface.
 *
 * This module is the public api to interface with the Zephyr GPIO Driver.
 *
 * @example
 */

#ifndef FKMG_IO_CTRL_LED_H
#define FKMG_IO_CTRL_LED_H

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
void FKMG_IO_CTRL_LED_Init_Instance(struct FKMG_IO_CTRL_LED_Instance_Cfg * p_cfg);

void FKMG_IO_CTRL_LED_Add_Listener(struct FKMG_IO_CTRL_LED_Listener_Cfg * p_cfg);

void FKMG_IO_CTRL_LED_Write_LED(struct FKMG_IO_CTRL_LED_Instance * p_inst, enum FKMG_IO_CTRL_LED_Id id, bool status);


#ifdef __cplusplus
}
#endif

#endif /* FKMG_IO_CTRL_LED_H */
