/* *****************************************************************************
 * Includes
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/counter.h>
#include <zephyr/drivers/dac.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>

#include <assert.h>
#include <string.h>


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

/* Enable Thread Defines. */

#define CONFIG_FKMG_IO_CTRL_DAC_THREAD      0

/* *****************************************************************************
 * Threads
 */
/* FKMG IO CONTROL THREADS. */

#if CONFIG_FKMG_IO_CTRL_DAC_THREAD
    /* Declare threads, queues, and other data structures for fkmg io dac instance. */
    static struct k_thread fkmg_io_ctrl_dac_thread;
    #define FKMG_IO_CTRL_DAC_THREAD_STACK_SZ_BYTES   1024
    K_THREAD_STACK_DEFINE(fkmg_io_ctrl_dac_thread_stack, FKMG_IO_CTRL_DAC_THREAD_STACK_SZ_BYTES);
    #define MAX_QUEUED_FKMG_IO_CTRL_DAC_SM_EVTS  10
    #define FKMG_IO_CTRL_DAC_SM_QUEUE_ALIGNMENT  4
    K_MSGQ_DEFINE(fkmg_io_ctrl_dac_sm_evt_q, sizeof(struct FKMG_IO_CTRL_DAC_SM_Evt),
            MAX_QUEUED_FKMG_IO_CTRL_DAC_SM_EVTS, FKMG_IO_CTRL_DAC_SM_QUEUE_ALIGNMENT);
    static struct FKMG_IO_CTRL_DAC_Instance fkmg_io_ctrl_dac_inst;
#endif

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
// static void on_fkmg_io_ctrl_adc_converted(struct FKMG_IO_CTRL_ADC_Evt *p_evt)
// {
//     assert(p_evt->sig == k_FKMG_IO_CTRL_ADC_Evt_Sig_Converted);
//     /* Broadcast to where-ever. */
// }


/* ********************
 * ON INSTANCE INITS
 * ********************/

#if CONFIG_FKMG_IO_CTRL_DAC_THREAD
    static void on_fkmg_io_ctrl_dac_instance_initialized(struct FKMG_IO_CTRL_DAC_Evt *p_evt)
    {
        assert(p_evt->sig == k_FKMG_IO_CTRL_DAC_Evt_Sig_Instance_Initialized);
        assert(p_evt->data.initd.p_inst == &fkmg_io_ctrl_dac_inst);
        k_event_post(&events, EVT_FLAG_INSTANCE_INITIALIZED);
    }
#endif

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




int main(void) {
    int ret;

    const struct device *dac_dev = DEVICE_DT_GET(DT_NODELABEL(dac7578));
 
  if (!device_is_ready(dac_dev)) {
        printk("DAC7578 device not ready\n");
        return 0;
    }
/* 
    printk("DAC7578 device ready\n");
    return 0;


    // Initialize the DAC
    ret = dac7578_init();
    if (ret != 0) {
        printk("Failed to initialize DAC7578\n");
        return ret;
    }

    while(1) {
        static int v = 0; 

        for (int i = 0; i < 8; i++) {
            // Write to DAC
            ret = dac7578_write(i, 0);
            if (ret != 0) {
                printk("Failed to write to DAC7578\n");
                return ret;
            }
            k_sleep(K_MSEC(2));
        }

        // v += 250; 
        // if (v > 4095) {
        //     v = 0;
        // }
    }
*/




/* FKMG I/O Control Driver Initialization*/
#if CONFIG_FKMG_IO_CTRL_DAC_THREAD
/* Instance: DAC */
    struct FKMG_IO_CTRL_DAC_Instance_Cfg fkmg_io_ctrl_dac_inst_cfg = {
        .p_inst = &fkmg_io_ctrl_dac_inst,
        .task.sm.p_thread = &fkmg_io_ctrl_dac_inst,
        .task.sm.p_stack = fkmg_io_ctrl_dac_thread_stack,
        .task.sm.stack_sz = K_THREAD_STACK_SIZEOF(fkmg_io_ctrl_dac_thread_stack),
        .task.sm.prio = K_LOWEST_APPLICATION_THREAD_PRIO,
        .msgq.p_sm_evts = &fkmg_io_ctrl_dac_sm_evt_q,
        .cb = on_fkmg_io_ctrl_dac_instance_initialized,
    };
    FKMG_IO_CTRL_DAC_Init_Instance(&fkmg_io_ctrl_dac_inst_cfg);
    wait_on_instance_initialized();
#endif

};
