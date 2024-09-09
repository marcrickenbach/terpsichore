/** ****************************************************************************
 * @brief FKMG LED Control id.
 */

#ifndef FKMG_IO_CTRL_LED_ID_H
#define FKMG_IO_CTRL_LED_ID_H

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

enum FKMG_IO_CTRL_LED_Id{
    k_FKMG_IO_CTRL_LED_Id_Beg,                   // Inclusive
    k_FKMG_IO_CTRL_LED_Id_Min = k_FKMG_IO_CTRL_LED_Id_Beg,    // Inclusive
    k_FKMG_IO_CTRL_LED_Id_1st = k_FKMG_IO_CTRL_LED_Id_Beg,    // Inclusive

    LED_0 = k_FKMG_IO_CTRL_LED_Id_Beg,
    LED_1,
    LED_2,
    LED_3,
    LED_4,

    k_FKMG_IO_CTRL_LED_Id_End,                   // Exclusive
    k_FKMG_IO_CTRL_LED_Id_Max = k_FKMG_IO_CTRL_LED_Id_End - 1,// Inclusive
    k_FKMG_IO_CTRL_LED_Id_Lst = k_FKMG_IO_CTRL_LED_Id_End - 1,// Inclusive
    k_FKMG_IO_CTRL_LED_Id_Cnt = k_FKMG_IO_CTRL_LED_Id_End
                 - k_FKMG_IO_CTRL_LED_Id_Beg,	// Inclusive
	k_FKMG_IO_CTRL_LED_Id_Ivd = k_FKMG_IO_CTRL_LED_Id_End
};

#ifdef __cplusplus
}
#endif

#endif /* FKMG_IO_CTRL_LED_ID_H */
