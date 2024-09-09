/** ****************************************************************************
 * @brief Internal state machine events generated by FKMG LED Control.
 */

#ifndef FKMG_IO_CTRL_LED_SM_EVT_H
#define FKMG_IO_CTRL_LED_SM_EVT_H

#ifdef __cplusplus
extern "C" {
#endif

/* *****************************************************************************
 * Includes
 */

#include <zephyr/kernel.h>

#include "../id.h"
#include "../instance_cfg.h"

/* *****************************************************************************
 * Events
 */

/* Signals that can be generated as part of an event. */
enum FKMG_IO_CTRL_LED_SM_Evt_Sig{
    k_FKMG_IO_CTRL_LED_SM_Evt_Sig_Beg,                           // Inclusive
    k_FKMG_IO_CTRL_LED_SM_Evt_Sig_Min = k_FKMG_IO_CTRL_LED_SM_Evt_Sig_Beg,    // Inclusive
    k_FKMG_IO_CTRL_LED_SM_Evt_Sig_1st = k_FKMG_IO_CTRL_LED_SM_Evt_Sig_Beg,    // Inclusive

	k_FKMG_IO_CTRL_LED_SM_Evt_Sig_Init_Instance = k_FKMG_IO_CTRL_LED_SM_Evt_Sig_Beg,
    #if CONFIG_FKMG_IO_CTRL_LED_ALLOW_SHUTDOWN
	k_FKMG_IO_CTRL_LED_SM_Evt_Sig_Deinit_Instance,
    #endif
	k_FKMG_IO_CTRL_LED_SM_Evt_Sig_Write,
	k_FKMG_IO_CTRL_LED_SM_Evt_Sig_Clear,

    k_FKMG_IO_CTRL_LED_SM_Evt_Sig_End,                           // Exclusive
    k_FKMG_IO_CTRL_LED_SM_Evt_Sig_Max = k_FKMG_IO_CTRL_LED_SM_Evt_Sig_End - 1,// Inclusive
    k_FKMG_IO_CTRL_LED_SM_Evt_Sig_Lst = k_FKMG_IO_CTRL_LED_SM_Evt_Sig_End - 1,// Inclusive
    k_FKMG_IO_CTRL_LED_SM_Evt_Sig_Cnt = k_FKMG_IO_CTRL_LED_SM_Evt_Sig_End
                         - k_FKMG_IO_CTRL_LED_SM_Evt_Sig_Beg,	// Inclusive
	k_FKMG_IO_CTRL_LED_SM_Evt_Sig_Ivd = k_FKMG_IO_CTRL_LED_SM_Evt_Sig_End
};

/* Data signal k_LED_SM_Evt_Sig_Init_Instance can generate. */
struct FKMG_IO_CTRL_LED_SM_Evt_Sig_Init_Instance{
    struct FKMG_IO_CTRL_LED_Instance_Cfg cfg;
};

/* Data signal k_LED_SM_Sig_Convert can generate. */
struct FKMG_IO_CTRL_LED_SM_Evt_Sig_Write{
    enum FKMG_IO_CTRL_LED_Id row;
    uint8_t val;
};

/* Events (i.e. signal + signal's data if any) LED State Machine generates. */
struct FKMG_IO_CTRL_LED_SM_Evt{
	enum FKMG_IO_CTRL_LED_SM_Evt_Sig sig;
	union FKMG_IO_CTRL_LED_SM_Evt_Data{
        struct FKMG_IO_CTRL_LED_SM_Evt_Sig_Init_Instance     init_inst;
        struct FKMG_IO_CTRL_LED_SM_Evt_Sig_Write             write;
	}data;
};

#ifdef __cplusplus
}
#endif

#endif /* FKMG_IO_CTRL_LED_SM_EVT_H */
