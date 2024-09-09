/** ****************************************************************************
 * @brief FKMG GPIO Control id.
 */

#ifndef FKMG_IO_CTRL_GPIO_ID_H
#define FKMG_IO_CTRL_GPIO_ID_H

#ifdef __cplusplus
extern "C" {
#endif

/* *****************************************************************************
 * Includes
 */

#include <zephyr/kernel.h>

/* *****************************************************************************
 * Ids
 */

enum FKMG_IO_CTRL_GPIO_Id{
    k_FKMG_IO_CTRL_GPIO_Id_Beg,                   // Inclusive
    k_FKMG_IO_CTRL_GPIO_Id_Min = k_FKMG_IO_CTRL_GPIO_Id_Beg,    // Inclusive
    k_FKMG_IO_CTRL_GPIO_Id_1st = k_FKMG_IO_CTRL_GPIO_Id_Beg,    // Inclusive

    MODE_LED_1 = k_FKMG_IO_CTRL_GPIO_Id_Beg,
    MODE_LED_2,
    MODE_LED_3,
    MODE_LED_4,
    MODE_LED_5,

    MODE_SUB_LED_1,
    MODE_SUB_LED_2,
    MODE_SUB_LED_3,
    MODE_SUB_LED_4,
    MODE_SUB_LED_5,

    DAC_SWITCH_INPUT,
    MODE_SELECT_INPUT,
    SUB_SELECT_INPUT,

    k_FKMG_IO_CTRL_GPIO_Id_End,                   // Exclusive
    k_FKMG_IO_CTRL_GPIO_Id_Max = k_FKMG_IO_CTRL_GPIO_Id_End - 1,// Inclusive
    k_FKMG_IO_CTRL_GPIO_Id_Lst = k_FKMG_IO_CTRL_GPIO_Id_End - 1,// Inclusive
    k_FKMG_IO_CTRL_GPIO_Id_Cnt = k_FKMG_IO_CTRL_GPIO_Id_End
                 - k_FKMG_IO_CTRL_GPIO_Id_Beg,	// Inclusive
	k_FKMG_IO_CTRL_GPIO_Id_Ivd = k_FKMG_IO_CTRL_GPIO_Id_End
};

#ifdef __cplusplus
}
#endif

#endif /* FKMG_IO_CTRL_GPIO_ID_H */
