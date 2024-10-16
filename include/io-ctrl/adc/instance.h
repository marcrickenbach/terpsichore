/** ****************************************************************************
 * @brief FKMG ADC Control Instance Interface.
 *
 * The ADC control instance interface interacts directly on a driver level with
 * the ADC peripheral. Configuration of the ADC channels is done in the
 * device tree overlay file, while all initialization and control is handled from
 * within this module. The ADC control module is responsible for reading the
 * values from the ADC channels and broadcasting the values to listeners.
 * 
 * To use the ADC control module, the following steps are required:
 * 1. Include the header file for the ADC control module.
 * 2. Initialize the module by calling the init function with the instance
 *   pointer in main.c or any other file.
 * 3. Add listeners to the instance to receive the ADC values.
 * 
 */

#ifndef FKMG_IO_CTRL_ADC_INSTANCE_H
#define FKMG_IO_CTRL_ADC_INSTANCE_H

#ifdef __cplusplus
extern "C" {
#endif

/* *****************************************************************************
 * Includes
 */

#include <zephyr/kernel.h>
#include <zephyr/smf.h>

#include "err.h"
#include "id.h"
#include "evt.h"
#include "private/sm_evt.h"

#include <zephyr/drivers/adc.h>

/* *****************************************************************************
 * Enums
 */

/* *****************************************************************************
 * Instance
 */

struct FKMG_IO_CTRL_ADC_Instance{
    #if CONFIG_FKMG_POT_RUNTIME_ERROR_CHECKING
    /* Error status. */
	enum FKMG_IO_CTRL_ADC_Err_Id err;
    #endif

    /* Threads used. */
    struct{
        struct{
            struct k_thread * p_thread;
        }sm;
    }task;

    /* Queues used. */
    struct{
        /* State machine event message queue. */
        struct k_msgq * p_sm_evts;
    }msgq;

    /* State machine. */
	struct smf_ctx sm;

    /* Current sm event. */
    struct FKMG_IO_CTRL_ADC_SM_Evt sm_evt;

    /* Singly linked lists to keep track of things. */
    struct{
        sys_slist_t listeners[k_FKMG_IO_CTRL_ADC_Evt_Sig_Cnt];
    }list;

    /* For adding this instance to singly linked lists. */
    struct{
        sys_snode_t instance;
    }node;

    /* Timers used. */
    struct{
        struct k_timer conversion;
    }timer;
    
    struct adc_sequence sequence; 

    uint32_t adc_t_duration_us;
    uint32_t adc_t_period_us;

    /* Current element. */
    enum FKMG_IO_CTRL_ADC_Id id;

    int16_t * adc_buffer; 

};

#ifdef __cplusplus
}
#endif

#endif /* FKMG_IO_CTRL_ADC_INSTANCE_H */
