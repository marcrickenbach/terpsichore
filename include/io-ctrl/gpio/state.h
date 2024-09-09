/** ****************************************************************************
 * @brief FKMG GPIO Control State.
 */

#ifndef FKMG_IO_CTRL_GPIO_STATE_H
#define FKMG_IO_CTRL_GPIO_STATE_H

#ifdef __cplusplus
extern "C" {
#endif

/* *****************************************************************************
 * Includes
 */

#include <zephyr/kernel.h>

/* *****************************************************************************
 * States
 */

enum FKMG_IO_CTRL_GPIO_State{
    k_FKMG_IO_CTRL_GPIO_State_Beg,                   // Inclusive
    k_FKMG_IO_CTRL_GPIO_State_Min = k_FKMG_IO_CTRL_GPIO_State_Beg,    // Inclusive
    k_FKMG_IO_CTRL_GPIO_State_1st = k_FKMG_IO_CTRL_GPIO_State_Beg,    // Inclusive

    k_FKMG_IO_CTRL_GPIO_State_Low = k_FKMG_IO_CTRL_GPIO_State_Beg,
    k_FKMG_IO_CTRL_GPIO_State_High,

    k_FKMG_IO_CTRL_GPIO_State_End,                   // Exclusive
    k_FKMG_IO_CTRL_GPIO_State_Max = k_FKMG_IO_CTRL_GPIO_State_End - 1,// Inclusive
    k_FKMG_IO_CTRL_GPIO_State_Lst = k_FKMG_IO_CTRL_GPIO_State_End - 1,// Inclusive
    k_FKMG_IO_CTRL_GPIO_State_Cnt = k_FKMG_IO_CTRL_GPIO_State_End
                 - k_FKMG_IO_CTRL_GPIO_State_Beg,	// Inclusive
	k_FKMG_IO_CTRL_GPIO_State_Ivd = k_FKMG_IO_CTRL_GPIO_State_End
};

#ifdef __cplusplus
}
#endif

#endif /* FKMG_IO_CTRL_GPIO_STATE_H */
