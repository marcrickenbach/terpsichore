/*******************************************************************************
 * @brief LED interface.
 *
 * This module is the public api to interface with leds.
 *
 * @example
 */

#ifndef FKMG_LED_H
#define FKMG_LED_H

/* In case C++ needs to use anything here */
#ifdef __cplusplus
extern “C” {
#endif

/* *****************************************************************************
 * Includes
 */

#include <zephyr/kernel.h>

#include "led/instance.h"
#include "led/instance_cfg.h"
#include "led/listener.h"
#include "led/listener_cfg.h"


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
void LED_Init_Instance(struct LED_Instance_Cfg * p_cfg);

/**
 * Deinitialize an instance.
 * @param[in] p_dcfg Pointer to the filled-in deconfiguration struct. See the
 * struct for details.
 */
#if CONFIG_FKMG_LED_ALLOW_SHUTDOWN
void LED_Deinit_Instance(struct LED_Instance_Dcfg * p_dcfg);
#endif

/**
 * Add an event signal listener to an interface.
 * @param[in] p_cfg Pointer to the filled-in configuration struct. See the
 * struct for details.
 */
void LED_Add_Listener(struct LED_Listener_Cfg * p_cfg);


void LED_Write_Modes(struct LED_Instance * p_inst, enum LED_Id row, uint8_t val); 

#if CONFIG_FKMG_LED_ALLOW_LISTENER_REMOVAL
void LED_Remove_Listener(struct LED_Listener * p_lsnr);
#endif

#ifdef __cplusplus
}
#endif

#endif /* FKMG_LED_H */
