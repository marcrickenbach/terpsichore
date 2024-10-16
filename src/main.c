/* *****************************************************************************
 * Includes
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/counter.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/clock_control/stm32_clock_control.h>
#include <stm32f4xx.h>
#include <stm32f4xx_ll_rcc.h>

#include <assert.h>
#include <string.h>

#include "main.h"

#include "io-ctrl/io_ctrl.h"

#include "io-ctrl/adc/id.h"
#include "io-ctrl/adc/private/sm_evt.h"

#include "io-ctrl/dac/id.h"
#include "io-ctrl/dac/private/sm_evt.h"

#include "io-ctrl/gpio/id.h"
#include "io-ctrl/gpio/private/sm_evt.h"

#include "io-ctrl/i2c/id.h"
#include "io-ctrl/i2c/private/sm_evt.h"

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

#define CONFIG_FKMG_IO_CTRL_ADC_THREAD      0
#define CONFIG_FKMG_IO_CTRL_DAC_THREAD      0
#define CONFIG_FKMG_IO_CTRL_GPIO_THREAD     0
#define CONFIG_FKMG_IO_CTRL_I2C_THREAD      0


/* *****************************************************************************
 * Threads
 */
/* FKMG IO CONTROL THREADS. */

#if CONFIG_FKMG_IO_CTRL_ADC_THREAD
    /* Declare threads, queues, and other data structures for fkmg io adc instance. */
    static struct k_thread fkmg_io_ctrl_adc_thread;
    #define FKMG_IO_CTRL_ADC_THREAD_STACK_SZ_BYTES   1024
    K_THREAD_STACK_DEFINE(fkmg_io_ctrl_adc_thread_stack, FKMG_IO_CTRL_ADC_THREAD_STACK_SZ_BYTES);
    #define MAX_QUEUED_FKMG_IO_CTRL_ADC_SM_EVTS  10
    #define FKMG_IO_CTRL_ADC_SM_QUEUE_ALIGNMENT  4
    K_MSGQ_DEFINE(fkmg_io_ctrl_adc_sm_evt_q, sizeof(struct FKMG_IO_CTRL_ADC_SM_Evt),
            MAX_QUEUED_FKMG_IO_CTRL_ADC_SM_EVTS, FKMG_IO_CTRL_ADC_SM_QUEUE_ALIGNMENT);
    static struct FKMG_IO_CTRL_ADC_Instance fkmg_io_ctrl_adc_inst;
#endif 

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

#if CONFIG_FKMG_IO_CTRL_GPIO_THREAD
    /* Declare threads, queues, and other data structures for fkmg io gpio instance. */
    static struct k_thread fkmg_io_ctrl_gpio_thread;
    #define FKMG_IO_CTRL_GPIO_THREAD_STACK_SZ_BYTES   1024
    K_THREAD_STACK_DEFINE(fkmg_io_ctrl_gpio_thread_stack, FKMG_IO_CTRL_GPIO_THREAD_STACK_SZ_BYTES);
    #define MAX_QUEUED_FKMG_IO_CTRL_GPIO_SM_EVTS  10
    #define FKMG_IO_CTRL_GPIO_SM_QUEUE_ALIGNMENT  4
    K_MSGQ_DEFINE(fkmg_io_ctrl_gpio_sm_evt_q, sizeof(struct FKMG_IO_CTRL_GPIO_SM_Evt),
            MAX_QUEUED_FKMG_IO_CTRL_GPIO_SM_EVTS, FKMG_IO_CTRL_GPIO_SM_QUEUE_ALIGNMENT);
    static struct FKMG_IO_CTRL_GPIO_Instance fkmg_io_ctrl_gpio_inst;
#endif

#if CONFIG_FKMG_IO_CTRL_I2C_THREAD
    /* Declare threads, queues, and other data structures for fkmg io i2c instance. */
    static struct k_thread fkmg_io_ctrl_i2c_thread;
    #define FKMG_IO_CTRL_I2C_THREAD_STACK_SZ_BYTES   1024
    K_THREAD_STACK_DEFINE(fkmg_io_ctrl_i2c_thread_stack, FKMG_IO_CTRL_I2C_THREAD_STACK_SZ_BYTES);
    #define MAX_QUEUED_FKMG_IO_CTRL_I2C_SM_EVTS  10
    #define FKMG_IO_CTRL_I2C_SM_QUEUE_ALIGNMENT  4
    K_MSGQ_DEFINE(fkmg_io_ctrl_i2c_sm_evt_q, sizeof(struct FKMG_IO_CTRL_I2C_SM_Evt),
            MAX_QUEUED_FKMG_IO_CTRL_I2C_SM_EVTS, FKMG_IO_CTRL_I2C_SM_QUEUE_ALIGNMENT);
    static struct FKMG_IO_CTRL_I2C_Instance fkmg_io_ctrl_i2c_inst;
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
static void on_fkmg_io_ctrl_adc_converted(struct FKMG_IO_CTRL_ADC_Evt *p_evt)
{
    assert(p_evt->sig == k_FKMG_IO_CTRL_ADC_Evt_Sig_Converted);
    /* Broadcast to where-ever. */
}

static void on_fkmg_io_ctrl_gpio_interrupt(struct FKMG_IO_CTRL_GPIO_Evt *p_evt)
{
    assert(p_evt->sig == k_FKMG_IO_CTRL_GPIO_Evt_Sig_GPIO_Interrupt);
    /* Broadcast to where-ever. */
}

/* ********************
 * ON INSTANCE INITS
 * ********************/

#if CONFIG_FKMG_IO_CTRL_ADC_THREAD
    static void on_fkmg_io_ctrl_adc_instance_initialized(struct FKMG_IO_CTRL_ADC_Evt *p_evt)
    {
        assert(p_evt->sig == k_FKMG_IO_CTRL_ADC_Evt_Sig_Instance_Initialized);
        assert(p_evt->data.initd.p_inst == &fkmg_io_ctrl_adc_inst);
        k_event_post(&events, EVT_FLAG_INSTANCE_INITIALIZED);
    }
#endif

#if CONFIG_FKMG_IO_CTRL_DAC_THREAD
    static void on_fkmg_io_ctrl_dac_instance_initialized(struct FKMG_IO_CTRL_DAC_Evt *p_evt)
    {
        assert(p_evt->sig == k_FKMG_IO_CTRL_DAC_Evt_Sig_Instance_Initialized);
        assert(p_evt->data.initd.p_inst == &fkmg_io_ctrl_dac_inst);
        k_event_post(&events, EVT_FLAG_INSTANCE_INITIALIZED);
    }
#endif

#if CONFIG_FKMG_IO_CTRL_GPIO_THREAD
    static void on_fkmg_io_ctrl_gpio_instance_initialized(struct FKMG_IO_CTRL_GPIO_Evt *p_evt)
    {
        assert(p_evt->sig == k_FKMG_IO_CTRL_GPIO_Evt_Sig_Instance_Initialized);
        assert(p_evt->data.initd.p_inst == &fkmg_io_ctrl_gpio_inst);
        k_event_post(&events, EVT_FLAG_INSTANCE_INITIALIZED);
    }
#endif

#if CONFIG_FKMG_IO_CTRL_I2C_THREAD
    static void on_fkmg_io_ctrl_i2c_instance_initialized(struct FKMG_IO_CTRL_I2C_Evt *p_evt)
    {
        assert(p_evt->sig == k_FKMG_IO_CTRL_I2C_Evt_Sig_Instance_Initialized);
        assert(p_evt->data.initd.p_inst == &fkmg_io_ctrl_i2c_inst);
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

int main (void) {

/* FKMG I/O Control Driver Initialization*/

#if CONFIG_FKMG_IO_CTRL_ADC_THREAD
/* Instance: ADC */
    struct FKMG_IO_CTRL_ADC_Instance_Cfg fkmg_io_ctrl_adc_inst_cfg = {
        .p_inst = &fkmg_io_ctrl_adc_inst,
        .task.sm.p_thread = &fkmg_io_ctrl_adc_inst,
        .task.sm.p_stack = fkmg_io_ctrl_adc_thread_stack,
        .task.sm.stack_sz = K_THREAD_STACK_SIZEOF(fkmg_io_ctrl_adc_thread_stack),
        .task.sm.prio = K_LOWEST_APPLICATION_THREAD_PRIO,
        .msgq.p_sm_evts = &fkmg_io_ctrl_adc_sm_evt_q,
        .cb = on_fkmg_io_ctrl_adc_instance_initialized,
        .adc_t_duration_us = 100,
        .adc_t_period_us = 100
    };
    FKMG_IO_CTRL_ADC_Init_Instance(&fkmg_io_ctrl_adc_inst_cfg);
    wait_on_instance_initialized();

    static struct FKMG_IO_CTRL_ADC_Listener fkmg_io_ctrl_adc_converted_lsnr;
    struct FKMG_IO_CTRL_ADC_Listener_Cfg fkmg_io_ctrl_adc_lsnr_cfg = {
        .p_inst = &fkmg_io_ctrl_adc_inst,
        .p_lsnr = &fkmg_io_ctrl_adc_lsnr_cfg, 
        .sig     = k_FKMG_IO_CTRL_ADC_Evt_Sig_Converted,
        .cb      = on_fkmg_io_ctrl_adc_converted
    };
    FKMG_IO_CTRL_ADC_Add_Listener(&fkmg_io_ctrl_adc_lsnr_cfg);
#endif

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

#if CONFIG_FKMG_IO_CTRL_GPIO_THREAD
/* Instance: GPIO */
    struct FKMG_IO_CTRL_GPIO_Instance_Cfg fkmg_io_ctrl_gpio_inst_cfg = {
        .p_inst = &fkmg_io_ctrl_gpio_inst,
        .task.sm.p_thread = &fkmg_io_ctrl_gpio_inst,
        .task.sm.p_stack = fkmg_io_ctrl_gpio_thread_stack,
        .task.sm.stack_sz = K_THREAD_STACK_SIZEOF(fkmg_io_ctrl_gpio_thread_stack),
        .task.sm.prio = K_LOWEST_APPLICATION_THREAD_PRIO,
        .msgq.p_sm_evts = &fkmg_io_ctrl_gpio_sm_evt_q,
        .cb = on_fkmg_io_ctrl_gpio_instance_initialized,
    };
    FKMG_IO_CTRL_GPIO_Init_Instance(&fkmg_io_ctrl_gpio_inst_cfg);
    wait_on_instance_initialized();

    static struct FKMG_IO_CTRL_GPIO_Listener fkmg_io_ctrl_gpio_lsnr;
    struct FKMG_IO_CTRL_GPIO_Listener_Cfg fkmg_io_ctrl_gpio_lsnr_cfg = {
        .p_inst = &fkmg_io_ctrl_gpio_inst,
        .p_lsnr = &fkmg_io_ctrl_gpio_lsnr,
        .sig     = k_FKMG_IO_CTRL_GPIO_Evt_Sig_GPIO_Interrupt,
        .cb      = on_fkmg_io_ctrl_gpio_interrupt
    };
#endif

#if CONFIG_FKMG_IO_CTRL_I2C_THREAD
/* Instance: I2C */
    struct FKMG_IO_CTRL_I2C_Instance_Cfg fkmg_io_ctrl_i2c_inst_cfg = {
        .p_inst = &fkmg_io_ctrl_i2c_inst,
        .task.sm.p_thread = &fkmg_io_ctrl_i2c_inst,
        .task.sm.p_stack = fkmg_io_ctrl_i2c_thread_stack,
        .task.sm.stack_sz = K_THREAD_STACK_SIZEOF(fkmg_io_ctrl_i2c_thread_stack),
        .task.sm.prio = K_LOWEST_APPLICATION_THREAD_PRIO,
        .msgq.p_sm_evts = &fkmg_io_ctrl_i2c_sm_evt_q,
        .cb = on_fkmg_io_ctrl_i2c_instance_initialized,
    };
    FKMG_IO_CTRL_I2C_Init_Instance(&fkmg_io_ctrl_i2c_inst_cfg);
    wait_on_instance_initialized();
#endif

};
