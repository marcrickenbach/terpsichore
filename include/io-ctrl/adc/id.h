/** ****************************************************************************
 * @brief FKMG ADC Control id.
 */

#ifndef FKMG_IO_CTRL_ADC_ID_H
#define FKMG_IO_CTRL_ADC_ID_H

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

enum FKMG_IO_CTRL_ADC_Id{
    k_FKMG_IO_CTRL_ADC_Id_Beg = 0,                  // Inclusive
    k_FKMG_IO_CTRL_ADC_Id_Min = k_FKMG_IO_CTRL_ADC_Id_Beg,    // Inclusive
    k_FKMG_IO_CTRL_ADC_Id_1st = k_FKMG_IO_CTRL_ADC_Id_Beg,    // Inclusive

    k_FKMG_IO_CTRL_ADC_Id_0 = k_FKMG_IO_CTRL_ADC_Id_Beg,
    k_FKMG_IO_CTRL_ADC_Id_1,
    k_FKMG_IO_CTRL_ADC_Id_2,
    k_FKMG_IO_CTRL_ADC_Id_3,
    k_FKMG_IO_CTRL_ADC_Id_4,
    k_FKMG_IO_CTRL_ADC_Id_5,
    k_FKMG_IO_CTRL_ADC_Id_6,
    k_FKMG_IO_CTRL_ADC_Id_7,
    k_FKMG_IO_CTRL_ADC_Id_8,
    k_FKMG_IO_CTRL_ADC_Id_9,
    k_FKMG_IO_CTRL_ADC_Id_10,
    k_FKMG_IO_CTRL_ADC_Id_11,
    k_FKMG_IO_CTRL_ADC_Id_12,
    k_FKMG_IO_CTRL_ADC_Id_13,
    k_FKMG_IO_CTRL_ADC_Id_14,
    k_FKMG_IO_CTRL_ADC_Id_15,
    k_FKMG_IO_CTRL_ADC_Id_16,
    k_FKMG_IO_CTRL_ADC_Id_17,
    k_FKMG_IO_CTRL_ADC_Id_18,
    k_FKMG_IO_CTRL_ADC_Id_19,
    k_FKMG_IO_CTRL_ADC_Id_20,
    k_FKMG_IO_CTRL_ADC_Id_21,
    k_FKMG_IO_CTRL_ADC_Id_22,
    k_FKMG_IO_CTRL_ADC_Id_23,
    k_FKMG_IO_CTRL_ADC_Id_24,

    k_FKMG_IO_CTRL_ADC_Id_End,                   // Exclusive
    k_FKMG_IO_CTRL_ADC_Id_Max = k_FKMG_IO_CTRL_ADC_Id_End - 1,// Inclusive
    k_FKMG_IO_CTRL_ADC_Id_Lst = k_FKMG_IO_CTRL_ADC_Id_End - 1,// Inclusive
    k_FKMG_IO_CTRL_ADC_Id_Cnt = k_FKMG_IO_CTRL_ADC_Id_End
                 - k_FKMG_IO_CTRL_ADC_Id_Beg,	// Inclusive
	k_FKMG_IO_CTRL_ADC_Id_Ivd = k_FKMG_IO_CTRL_ADC_Id_End
};

#ifdef __cplusplus
}
#endif

#endif /* FKMG_IO_CTRL_ADC_ID_H */
