/** ****************************************************************************
 * @brief FKMG I2C Control errors.
 *
 * The errors pot module generates.
 */

#ifndef FKMG_IO_CTRL_I2C_ERR_H
#define FKMG_IO_CTRL_I2C_ERR_H

#ifdef __cplusplus
extern "C" {
#endif

/* *****************************************************************************
 * Enums
 */

/* Errors generated. */
enum FKMG_IO_CTRL_I2C_Err_Id{
    k_FKMG_IO_CTRL_I2C_Err_Id_Beg,                   // Inclusive
    k_FKMG_IO_CTRL_I2C_Err_Id_Min = k_FKMG_IO_CTRL_I2C_Err_Id_Beg,// Inclusive
    k_FKMG_IO_CTRL_I2C_Err_Id_1st = k_FKMG_IO_CTRL_I2C_Err_Id_Beg,// Inclusive

    k_FKMG_IO_CTRL_I2C_Err_Id_None = k_FKMG_IO_CTRL_I2C_Err_Id_Beg,
    k_FKMG_IO_CTRL_I2C_Err_Id_Unknown,
    k_FKMG_IO_CTRL_I2C_Err_Id_Unimplemented,
    k_FKMG_IO_CTRL_I2C_Err_Id_Ptr_Invalid,
    k_FKMG_IO_CTRL_I2C_Err_Id_Configuration_Invalid,

    k_FKMG_IO_CTRL_I2C_Err_Id_End,                        // Exclusive
    k_FKMG_IO_CTRL_I2C_Err_Id_Max = k_FKMG_IO_CTRL_I2C_Err_Id_End - 1, // Inclusive
    k_FKMG_IO_CTRL_I2C_Err_Id_Lst = k_FKMG_IO_CTRL_I2C_Err_Id_End - 1, // Inclusive
    k_FKMG_IO_CTRL_I2C_Err_Id_Cnt = k_FKMG_IO_CTRL_I2C_Err_Id_End
                     - k_FKMG_IO_CTRL_I2C_Err_Id_Beg,     // Inclusive
    k_FKMG_IO_CTRL_I2C_Err_Id_Ivd = k_FKMG_IO_CTRL_I2C_Err_Id_End
};

#ifdef __cplusplus
}
#endif

#endif /* FKMG_IO_CTRL_I2C_ERR_H */
