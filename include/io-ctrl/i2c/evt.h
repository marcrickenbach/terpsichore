/** ****************************************************************************
 * @brief Events generated by FKMG I2C Control module.
 *
 * The public events pot module generates.
 */

#ifndef FKMG_IO_CTRL_I2C_EVT_H
#define FKMG_IO_CTRL_I2C_EVT_H

#ifdef __cplusplus
extern "C" {
#endif

/* *****************************************************************************
 * Includes
 */

#include <zephyr/kernel.h>

#include "id.h"

/* *****************************************************************************
 * Events
 */

/* Signals that can be generated as part of an event. */
enum FKMG_IO_CTRL_I2C_Evt_Sig{
    k_FKMG_IO_CTRL_I2C_Evt_Sig_Beg,                        // Inclusive
    k_FKMG_IO_CTRL_I2C_Evt_Sig_Min = k_FKMG_IO_CTRL_I2C_Evt_Sig_Beg,    // Inclusive
    k_FKMG_IO_CTRL_I2C_Evt_Sig_1st = k_FKMG_IO_CTRL_I2C_Evt_Sig_Beg,    // Inclusive

    k_FKMG_IO_CTRL_I2C_Evt_Sig_Instance_Initialized = k_FKMG_IO_CTRL_I2C_Evt_Sig_Beg,
    #if CONFIG_FKMG_IO_CTRL_I2C_SHUTDOWN_ENABLED
    k_FKMG_IO_CTRL_I2C_Evt_Sig_Instance_Deinitialized,
    #endif
    k_FKMG_IO_CTRL_I2C_Evt_Sig_Changed, // Value changed

    k_FKMG_IO_CTRL_I2C_Evt_Sig_End,                        // Exclusive
    k_FKMG_IO_CTRL_I2C_Evt_Sig_Max = k_FKMG_IO_CTRL_I2C_Evt_Sig_End - 1,// Inclusive
    k_FKMG_IO_CTRL_I2C_Evt_Sig_Lst = k_FKMG_IO_CTRL_I2C_Evt_Sig_End - 1,// Inclusive
    k_FKMG_IO_CTRL_I2C_Evt_Sig_Cnt = k_FKMG_IO_CTRL_I2C_Evt_Sig_End
                      - k_FKMG_IO_CTRL_I2C_Evt_Sig_Beg,	  // Inclusive
	k_FKMG_IO_CTRL_I2C_Evt_Sig_Ivd = k_FKMG_IO_CTRL_I2C_Evt_Sig_End
};

/* Forward references to prevent include interdependent items getting declared
 * out-of-order. */
struct Instance;

/* Data signal k_FKMG_IO_CTRL_I2C_Evt_Sig_Instance_Initialized can generate. */
struct FKMG_IO_CTRL_I2C_Evt_Data_Instance_Initialized{
    struct FKMG_IO_CTRL_I2C_Instance * p_inst;
};

/* Data signal k_FKMG_IO_CTRL_I2C_Evt_Sig_Changed can generate. */
struct FKMG_IO_CTRL_I2C_Evt_Data_Changed{
    enum FKMG_IO_CTRL_I2C_Id id;
    uint32_t    val;
};

/* Events (i.e. signal + signal's data if any) that can be generated. */
struct FKMG_IO_CTRL_I2C_Evt{
	enum FKMG_IO_CTRL_I2C_Evt_Sig sig;
	union FKMG_IO_CTRL_I2C_Evt_Data{
        struct FKMG_IO_CTRL_I2C_Evt_Data_Instance_Initialized initd;
        struct FKMG_IO_CTRL_I2C_Evt_Data_Changed              changed;
	}data;
};

#ifdef __cplusplus
}
#endif

#endif /* FKMG_IO_CTRL_I2C_EVT_H */
