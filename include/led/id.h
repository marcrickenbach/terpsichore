/** ****************************************************************************
 * @brief LED id.
 */

#ifndef FKMG_LED_ID_H
#define FKMG_LED_ID_H

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

enum LED_Id{
    k_LED_Id_Beg,                   // Inclusive
    k_LED_Id_Min = k_LED_Id_Beg,    // Inclusive
    k_LED_Id_1st = k_LED_Id_Beg,    // Inclusive

    Mode = k_LED_Id_Beg,
    Sub,

    k_LED_Id_End,                   // Exclusive
    k_LED_Id_Max = k_LED_Id_End - 1,// Inclusive
    k_LED_Id_Lst = k_LED_Id_End - 1,// Inclusive
    k_LED_Id_Cnt = k_LED_Id_End
                 - k_LED_Id_Beg,	// Inclusive
	k_LED_Id_Ivd = k_LED_Id_End
};

#ifdef __cplusplus
}
#endif

#endif /* FKMG_LED_ID_H */
