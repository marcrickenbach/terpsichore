/** ****************************************************************************
 * @brief FKMG I2C Control id.
 */

#ifndef FKMG_IO_CTRL_I2C_ID_H
#define FKMG_IO_CTRL_I2C_ID_H

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

enum FKMG_IO_CTRL_I2C_Id{
    k_FKMG_IO_CTRL_I2C_Id_Beg = 0,                  // Inclusive
    k_FKMG_IO_CTRL_I2C_Id_Min = k_FKMG_IO_CTRL_I2C_Id_Beg,    // Inclusive
    k_FKMG_IO_CTRL_I2C_Id_1st = k_FKMG_IO_CTRL_I2C_Id_Beg,    // Inclusive

    k_FKMG_IO_CTRL_I2C_Id_0 = k_FKMG_IO_CTRL_I2C_Id_Beg,
    k_FKMG_IO_CTRL_I2C_Id_1,
    k_FKMG_IO_CTRL_I2C_Id_2,
    k_FKMG_IO_CTRL_I2C_Id_3,
    k_FKMG_IO_CTRL_I2C_Id_4,
    k_FKMG_IO_CTRL_I2C_Id_5,
    k_FKMG_IO_CTRL_I2C_Id_6,
    k_FKMG_IO_CTRL_I2C_Id_7,
    k_FKMG_IO_CTRL_I2C_Id_8,
    k_FKMG_IO_CTRL_I2C_Id_9,
    k_FKMG_IO_CTRL_I2C_Id_10,
    k_FKMG_IO_CTRL_I2C_Id_11,
    k_FKMG_IO_CTRL_I2C_Id_12,

    k_FKMG_IO_CTRL_I2C_Id_End,                   // Exclusive
    k_FKMG_IO_CTRL_I2C_Id_Max = k_FKMG_IO_CTRL_I2C_Id_End - 1,// Inclusive
    k_FKMG_IO_CTRL_I2C_Id_Lst = k_FKMG_IO_CTRL_I2C_Id_End - 1,// Inclusive
    k_FKMG_IO_CTRL_I2C_Id_Cnt = k_FKMG_IO_CTRL_I2C_Id_End
                 - k_FKMG_IO_CTRL_I2C_Id_Beg,	// Inclusive
	k_FKMG_IO_CTRL_I2C_Id_Ivd = k_FKMG_IO_CTRL_I2C_Id_End
};

#ifdef __cplusplus
}
#endif

#endif /* FKMG_IO_CTRL_I2C_ID_H */
