/* *****************************************************************************
 * Includes
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>

#include <assert.h>
#include <string.h>

#include "main.h"


/* *****************************************************************************
 * Debugging
 */
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main);

/* *****************************************************************************
 * Defines
 */

/* Error if took longer than this to initialize. */
#define WAIT_MAX_MSECS_FOR_INITIALIZATION 50


/* *****************************************************************************
 * Threads
 */

/* Declare threads, queues, and other data structures for pot instance. */
// static struct k_thread pot_thread;
// #define POT_THREAD_STACK_SZ_BYTES   1024
// K_THREAD_STACK_DEFINE(pot_thread_stack, POT_THREAD_STACK_SZ_BYTES);
// #define MAX_QUEUED_POT_SM_EVTS  20
// #define POT_SM_QUEUE_ALIGNMENT  4
// K_MSGQ_DEFINE(pot_sm_evt_q, sizeof(struct Pot_SM_Evt),
//         MAX_QUEUED_POT_SM_EVTS, POT_SM_QUEUE_ALIGNMENT);
// static struct Pot_Instance pot_inst;


/* *****************************************************************************
 * Listeners
 */

#if 0
static struct Pot_Listener midi_rcvd_listener;
#endif

/* *****************************************************************************
 * Kernel Events
 *
 * Zephyr low-overhead kernel way for threads to wait on and post asynchronous
 * events.
 */

/* For handling asynchronous callbacks. */
K_EVENT_DEFINE(events);
#define EVT_FLAG_INSTANCE_INITIALIZED   (1 << 0)



/* *****************************************************************************
 * Private Functions.
 */

/* ********************
 * ON LISTENER INITS
 * ********************/


/* ********************
 * ON INSTANCE INITS
 * ********************/

// static void on_gpio_instance_initialized(struct GPIOEXT_Evt *p_evt)
// {
//     assert(p_evt->sig == k_GPIOEXT_Evt_Sig_Instance_Initialized);
//     assert(p_evt->data.initd.p_inst == &gpioext_inst);
//     k_event_post(&events, EVT_FLAG_INSTANCE_INITIALIZED);
// }

/* ********************
 * WAIT ON INSTANCE INIT
 * ********************/

static void wait_on_instance_initialized(void)
{
	uint32_t events_rcvd = k_event_wait(&events, EVT_FLAG_INSTANCE_INITIALIZED,
		true, K_MSEC(WAIT_MAX_MSECS_FOR_INITIALIZATION));
	assert(events_rcvd == EVT_FLAG_INSTANCE_INITIALIZED);
}


/* *****************************************************************************
 * Public.
 */

int main (void) {

    // /* Instance: Pot */
    // struct Pot_Instance_Cfg pot_inst_cfg = {
    //     .p_inst = &pot_inst,
    //     .task.sm.p_thread = &pot_thread,
    //     .task.sm.p_stack = pot_thread_stack,
    //     .task.sm.stack_sz = K_THREAD_STACK_SIZEOF(pot_thread_stack),
    //     .task.sm.prio = K_LOWEST_APPLICATION_THREAD_PRIO,
    //     .msgq.p_sm_evts = &pot_sm_evt_q,
    //     .cb = on_pot_instance_initialized,
    // };
    // Pot_Init_Instance(&pot_inst_cfg);
    // wait_on_instance_initialized();

    // static struct Pot_Listener pot_changed_lsnr;
    // struct Pot_Listener_Cfg pot_lsnr_cfg = {
    //     .p_inst = &pot_inst,
    //     .p_lsnr = &pot_changed_lsnr, 
    //     .sig     = k_Pot_Evt_Sig_Changed,
    //     .cb      = on_pot_changed
    // };
    // Pot_Add_Listener(&pot_lsnr_cfg);

};
