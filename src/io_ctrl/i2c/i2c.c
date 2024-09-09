/* *****************************************************************************
 * @brief FKMG I/O Control I2C implementation.
 * 
 */


/* *****************************************************************************
 * Includes
 */

#include "io-ctrl/i2c/i2c.h"
#include <zephyr/smf.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <assert.h>
#include <stdlib.h>
#include "io-ctrl/i2c/private/sm_evt.h"
#include "io-ctrl/i2c/private/module_data.h"

/* *****************************************************************************
 * Constants, Defines, and Macros
 */

#define SUCCESS (0)
#define FAIL    (-1)

#define DAC7578_ADDR    0x48

#if 0
/* TODO: This isn't working for some reason, but we need to get the i2c device
 * and address from the device tree in a way that is customizable and doesn't
 * require the user to change anything in here. */
#define ZEPHYR_USER_NODE    DT_PATH(zephyr_user)

#if (DT_NODE_HAS_PROP(ZEPHYR_USER_NODE, i2c) && \
     DT_NODE_HAS_PROP(ZEPHYR_USER_NODE, i2c_addr))
    #define I2C_NODE            DT_PHANDLE(ZEPHYR_USER_NODE, i2c)
    #define I2C_ADDR            DT_PROP(ZEPHYR_USER_NODE, i2c_addr)
#else
    #warning "Unsupported board: see README and check /zephyr,user node"
    #define I2C_NODE DT_INVALID_NODE
    #define I2C_ADDR 0
#endif
#endif

#define I2C_NODE    DT_NODELABEL(i2c1)
const struct device *i2c_dev = DEVICE_DT_GET(I2C_NODE);


/* *****************************************************************************
 * Debugging
 */

#if CONFIG_FKMG_IO_CTRL_NO_OPTIMIZATIONS
#pragma GCC push_options
#pragma GCC optimize ("Og")
#warning "i2c.c unoptimized!"
#endif

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(fkmg_io_ctrl_i2c);

/* *****************************************************************************
 * Structs
 */

static struct fkmg_i2c_module_data fkmg_i2c_md = {0};
#define md fkmg_i2c_md


/* *****************************************************************************
 * Private
 */

/* *****
 * Utils
 * *****/

static bool instance_contains_state_machine_ctx(
        struct FKMG_IO_CTRL_I2C_Instance * p_inst,
        struct smf_ctx      * p_sm_ctx)
{
    return(&p_inst->sm == p_sm_ctx);
}

/* Find the instance that contains the state machine context. */
static struct FKMG_IO_CTRL_I2C_Instance * sm_ctx_to_instance(struct smf_ctx * p_sm_ctx)
{
    /* Iterate thru the instances. */
    struct FKMG_IO_CTRL_I2C_Instance * p_inst = NULL;
    SYS_SLIST_FOR_EACH_CONTAINER(&md.list.instances, p_inst, node.instance){
        if(instance_contains_state_machine_ctx(p_inst, p_sm_ctx)) return p_inst;
    }
    return(NULL);
}

static enum FKMG_IO_CTRL_I2C_Id next_i2c_channel(enum FKMG_IO_CTRL_I2C_Id id)
{
    return((id + 1) % k_FKMG_IO_CTRL_I2C_Id_Cnt);
}

/* **************
 * Listener Utils
 * **************/

static void clear_listener(struct FKMG_IO_CTRL_I2C_Listener * p_lsnr)
{
    memset(p_lsnr, 0, sizeof(*p_lsnr));
}

static void config_listener(
        struct FKMG_IO_CTRL_I2C_Listener     * p_lsnr,
        struct FKMG_IO_CTRL_I2C_Listener_Cfg * p_cfg)
{
    /* Set listner's instance it is listening to. */
    p_lsnr->p_inst = p_cfg->p_inst;

    /* Set listner's callback. */ 
    p_lsnr->cb = p_cfg->cb;
}

static void init_listener(struct FKMG_IO_CTRL_I2C_Listener * p_lsnr)
{
    clear_listener(p_lsnr);
}

/* **************
 * Instance Utils
 * **************/

static void add_instance_to_instances(
        struct FKMG_IO_CTRL_I2C_Instance  * p_inst)
{
    sys_slist_append(&md.list.instances, &p_inst->node.instance);
}

static void config_instance_queues(
        struct FKMG_IO_CTRL_I2C_Instance     * p_inst,
        struct FKMG_IO_CTRL_I2C_Instance_Cfg * p_cfg)
{
    p_inst->msgq.p_sm_evts = p_cfg->msgq.p_sm_evts;
}

/* Forward reference */
static void on_conversion_timer_expiry(struct k_timer * p_timer);
/* Cause conversion event to occur immediately and then regularly after that. */
static void init_conversion_timer(struct FKMG_IO_CTRL_I2C_Instance * p_inst)
{
    #define ON_CONVERSION_TIMER_EXPIRY  on_conversion_timer_expiry
    #define ON_CONVERSION_TIMER_STOPPED NULL
    k_timer_init(&p_inst->timer.conversion, ON_CONVERSION_TIMER_EXPIRY,
            ON_CONVERSION_TIMER_STOPPED);
}

/* Forward reference */
static int init_io_ctrl_i2c_device(struct FKMG_IO_CTRL_I2C_Instance * p_inst); 

static void config_instance_deferred(
        struct FKMG_IO_CTRL_I2C_Instance     * p_inst,
        struct FKMG_IO_CTRL_I2C_Instance_Cfg * p_cfg)
{
    p_inst->i2c_t_duration_ms = p_cfg->i2c_t_duration_ms;
    p_inst->i2c_t_period_ms = p_cfg->i2c_t_period_ms;
    init_io_ctrl_i2c_device(p_inst);
}

/* Since configuration starts on caller's thread, configure fields that require
 * immediate and/or inconsequential configuration and defer rest to be handled
 * by our own thread later. */
static void config_instance_immediate(
        struct FKMG_IO_CTRL_I2C_Instance     * p_inst,
        struct FKMG_IO_CTRL_I2C_Instance_Cfg * p_cfg)
{
    config_instance_queues(p_inst, p_cfg);
}

static void init_instance_lists(struct FKMG_IO_CTRL_I2C_Instance * p_inst)
{
    for(enum FKMG_IO_CTRL_I2C_Evt_Sig sig = k_FKMG_IO_CTRL_I2C_Evt_Sig_Beg;
                         sig < k_FKMG_IO_CTRL_I2C_Evt_Sig_End;
                         sig++){
        sys_slist_init(&p_inst->list.listeners[sig]);
    }
}

static void clear_instance(struct FKMG_IO_CTRL_I2C_Instance * p_inst)
{
    memset(p_inst, 0, sizeof(*p_inst));
}

static void init_instance(struct FKMG_IO_CTRL_I2C_Instance * p_inst)
{
    clear_instance(p_inst);
    init_instance_lists(p_inst);
}



/* ************
 * Module Utils
 * ************/

static void init_module_lists(void)
{
    sys_slist_init(&md.list.instances);
}

static void clear_module(void)
{
    memset(&md, 0, sizeof(md));
}

static void init_module(void)
{
    if(md.initialized) return;
    clear_module();
    init_module_lists();
    md.initialized = true;
}


/* ************
 * Broadcasting
 * ************/

static void broadcast(
        struct FKMG_IO_CTRL_I2C_Evt      * p_evt,
        struct FKMG_IO_CTRL_I2C_Listener * p_lsnr)
{
    /* call the listener, passing the event */
    if(p_lsnr->cb)  p_lsnr->cb(p_evt);
}

static void broadcast_event_to_listeners(
        struct FKMG_IO_CTRL_I2C_Instance * p_inst,
        struct FKMG_IO_CTRL_I2C_Evt      * p_evt)
{
    enum FKMG_IO_CTRL_I2C_Evt_Sig sig = p_evt->sig;
    struct FKMG_IO_CTRL_I2C_Listener * p_lsnr = NULL;
    SYS_SLIST_FOR_EACH_CONTAINER(&p_inst->list.listeners[sig], p_lsnr, node.listener){
         broadcast(p_evt, p_lsnr);
    }
}

/* There's only 1 listener for instance initialization, and it is provided in
 * the cfg struct. */
static void broadcast_instance_initialized(
        struct FKMG_IO_CTRL_I2C_Instance * p_inst,
        FKMG_IO_CTRL_I2C_Listener_Cb       cb)
{
    struct FKMG_IO_CTRL_I2C_Evt evt = {
            .sig = k_FKMG_IO_CTRL_I2C_Evt_Sig_Instance_Initialized,
            .data.initd.p_inst = p_inst
    };

    struct FKMG_IO_CTRL_I2C_Listener lsnr = {
        .p_inst = p_inst,
        .cb = cb
    };

    broadcast(&evt, &lsnr);
}


/* **************
 * Listener Utils
 * **************/

static void add_listener_for_signal_to_listener_list(
    struct FKMG_IO_CTRL_I2C_Listener_Cfg * p_cfg)
{
    /* Get pointer to interface to add listener to. */
    struct FKMG_IO_CTRL_I2C_Instance * p_inst = p_cfg->p_inst;

    /* Get pointer to configured listener. */
    struct FKMG_IO_CTRL_I2C_Listener * p_lsnr = p_cfg->p_lsnr;

    /* Get signal to listen for. */
    enum FKMG_IO_CTRL_I2C_Evt_Sig sig = p_cfg->sig;

    /* Add listener to instance's specified signal. */
    sys_slist_t * p_list = &p_inst->list.listeners[sig];
    sys_snode_t * p_node = &p_lsnr->node.listener;
    sys_slist_append(p_list, p_node);
}


/* **************
 * Event Queueing
 * **************/

static void q_sm_event(struct FKMG_IO_CTRL_I2C_Instance * p_inst, struct FKMG_IO_CTRL_I2C_SM_Evt * p_evt)
{
    bool queued = k_msgq_put(p_inst->msgq.p_sm_evts, p_evt, K_NO_WAIT) == 0;

    if(!queued) assert(false);
}

static void q_init_instance_event(struct FKMG_IO_CTRL_I2C_Instance_Cfg * p_cfg)
{
    struct FKMG_IO_CTRL_I2C_SM_Evt evt = {
            .sig = k_FKMG_IO_CTRL_I2C_SM_Evt_Sig_Init_Instance,
            .data.init_inst.cfg = *p_cfg
    };
    struct FKMG_IO_CTRL_I2C_Instance * p_inst = p_cfg->p_inst;

    q_sm_event(p_inst, &evt);
}

static void on_conversion_timer_expiry(struct k_timer * p_timer)
{
    struct FKMG_IO_CTRL_I2C_Instance * p_inst =
        CONTAINER_OF(p_timer, struct FKMG_IO_CTRL_I2C_Instance, timer.conversion);

    enum FKMG_IO_CTRL_I2C_Id nextI2CId = next_i2c_channel(p_inst->id);
    p_inst->id = nextI2CId;
    q_convert(p_inst, nextI2CId);

}

/* Cause conversion event to occur immediately and then regularly after that. */
static void start_conversion_timer(struct FKMG_IO_CTRL_I2C_Instance * p_inst)
{
    #define CONVERSION_INITIAL_DURATION     K_MSEC(p_inst->i2c_t_duration_ms)
    #define CONVERSION_AUTO_RELOAD_PERIOD   K_MSEC(p_inst->i2c_t_period_ms)
    k_timer_start(&p_inst->timer.conversion, CONVERSION_INITIAL_DURATION,
            CONVERSION_AUTO_RELOAD_PERIOD);

}

static int init_io_ctrl_i2c_device(struct FKMG_IO_CTRL_I2C_Instance * p_inst)
{
    /* Initialize the I2C device over i2c. */
    if(!device_is_ready(i2c_dev)){
        LOG_ERR("I2C device not ready");
        return FAIL;
    }

    /* Configure i2c device. */
    int ret = i2c_configure(i2c_dev, I2C_SPEED_SET(I2C_SPEED_FAST_PLUS));
    if(ret){
        LOG_ERR("Failed to configure I2C device: %d", ret);
        return FAIL;
    }

    return SUCCESS;
}


/* **********
 * FSM States
 * **********/

/* Forward declaration of state table */
static const struct smf_state states[];

enum state{
    init,   // Init instance - should only occur once, after thread start
    run,    // Run - handles all events while running (e.g. conversion, etc.)
    deinit, // Deinit instance - should only occur once, after deinit event
            // (if implemented)
};


/* Init state responsibility is to complete initialization of the instance, let
 * an instance config listener know we've completed initialization (i.e. thread
 * is up and running), and then transition to next state. Init state occurs
 * immediately after thread start and is expected to only occur once. */

static void state_init_run(void * o)
{
    struct smf_ctx * p_sm = o;
    struct FKMG_IO_CTRL_I2C_Instance * p_inst = sm_ctx_to_instance(p_sm);

    /* Get the event. */
    struct FKMG_IO_CTRL_I2C_SM_Evt * p_evt = &p_inst->sm_evt;

    /* Expecting only an "init instance" event. Anything else is an error. */
    assert(p_evt->sig == k_FKMG_IO_CTRL_I2C_SM_Evt_Sig_Init_Instance);

    /* We init'd required params on the caller's thread (i.e.
     * xxx_Init_Instance()), now finish the job. Since this is an
     * Init_Instance event the data contains the intance cfg. */
    struct FKMG_IO_CTRL_I2C_SM_Evt_Sig_Init_Instance * p_ii = &p_evt->data.init_inst;
    
    config_instance_deferred(p_inst, &p_ii->cfg);
    broadcast_instance_initialized(p_inst, p_ii->cfg.cb);

    smf_set_state(SMF_CTX(p_sm), &states[run]);
}

/* Run state responsibility is to be the root state to handle everything else
 * other than instance initialization. */

static void state_run_entry(void * o)
{
    struct smf_ctx * p_sm = o;
    struct FKMG_IO_CTRL_I2C_Instance * p_inst = sm_ctx_to_instance(p_sm);
    start_conversion_timer(p_inst);
}

static void state_run_run(void * o)
{
    struct smf_ctx * p_sm = o;
    struct FKMG_IO_CTRL_I2C_Instance * p_inst = sm_ctx_to_instance(p_sm);

    /* Get the event. */
    struct FKMG_IO_CTRL_I2C_SM_Evt * p_evt = &p_inst->sm_evt;

    switch(p_evt->sig){
        default: break;
        case k_FKMG_IO_CTRL_I2C_SM_Evt_Sig_Init_Instance:
            /* Should never occur. */
            assert(false);
            break;
        case k_FKMG_IO_CTRL_I2C_SM_Evt_Sig_Write_Sample:
            /* Write the sample to the I2C. */

            enum FKMG_IO_CTRL_I2C_Id id = p_evt->data.write.id;
            uint16_t sample = p_evt->data.write.sample;
            uint8_t data[3];

            /* First byte: Control byte */
            data[0] = 0x30 | (id & 0x0F);
            /* Second byte: Upper 8 bits of DAC value */
            data[1] = (sample >> 4) & 0xFF;
            /* Third byte: Lower 4 bits of DAC value, padded with 4 zero bits */
            data[2] = (sample & 0x0F) << 4;

            int ret = i2c_burst_write(i2c_dev, DAC7578_ADDR, 0, data, sizeof(data));
            if(ret){
                LOG_ERR("Failed to write to I2C: %d", ret);
            }

            break;

    }
}

static const struct smf_state states[] = {
    /*                                      entry               run  exit */
    [  init] = SMF_CREATE_STATE(           NULL,   state_init_run, NULL, NULL, NULL),
    [   run] = SMF_CREATE_STATE(state_run_entry,    state_run_run, NULL, NULL, NULL),
};

/* ******
 * Thread
 * ******/

static void thread(void * p_1, /* struct I2C Instance* */
        void * p_2_unused, void * p_3_unused)
{
    struct FKMG_IO_CTRL_I2C_Instance * p_inst = p_1;
    /* NOTE: smf_set_initial() executes the entry state. */
    struct smf_ctx * p_sm = &p_inst->sm;
    smf_set_initial(SMF_CTX(p_sm), &states[init]);

    /* Get the state machine event queue and point to where to put the dequeued
     * event. */
    struct k_msgq * p_msgq = p_inst->msgq.p_sm_evts;
    struct FKMG_IO_CTRL_I2C_SM_Evt * p_evt = &p_inst->sm_evt;

    bool run = true;

    while(run){
        /* Wait on state machine event. Cache it then run state machine. */
        k_msgq_get(p_msgq, p_evt, K_FOREVER);
        run = smf_run_state(SMF_CTX(p_sm)) == 0;
    }
}

static void start_thread(
        struct FKMG_IO_CTRL_I2C_Instance     * p_inst,
        struct FKMG_IO_CTRL_I2C_Instance_Cfg * p_cfg)
{
    struct k_thread  * p_thread = p_cfg->task.sm.p_thread;
    k_thread_stack_t * p_stack  = p_cfg->task.sm.p_stack;
    size_t stack_sz_bytes = p_cfg->task.sm.stack_sz;
    int prio = p_cfg->task.sm.prio;

    /* Start the state machine thread. */
    p_inst->task.sm.p_thread = k_thread_create(
        p_thread,       // ptr to uninitialized struct k_thread
        p_stack,        // ptr to the stack space
        stack_sz_bytes, // stack size in bytes
        thread,         // thread entry function
        p_inst,         // 1st entry point parameter
        NULL,           // 2nd entry point parameter
        NULL,           // 3rd entry point parameter
        prio,           // thread priority 
        0,              // thread options
        K_NO_WAIT);     // scheduling delay
    assert(p_inst->task.sm.p_thread == p_thread);
}

/* *****************************************************************************
 * Public
 */
    
void FKMG_IO_CTRL_I2C_Init_Instance(struct FKMG_IO_CTRL_I2C_Instance_Cfg * p_cfg)
{
    /* Get pointer to instance to configure. */
    struct FKMG_IO_CTRL_I2C_Instance * p_inst = p_cfg->p_inst;

    init_module();
    init_instance(p_inst);
    config_instance_immediate(p_inst, p_cfg);
    add_instance_to_instances(p_inst);
    start_thread(p_inst, p_cfg);
    q_init_instance_event(p_cfg);
}

void FKMG_IO_CTRL_I2C_Add_Listener(struct FKMG_IO_CTRL_I2C_Listener_Cfg * p_cfg)
{
    struct FKMG_IO_CTRL_I2C_Listener * p_lsnr = p_cfg->p_lsnr;
    init_listener(p_lsnr);
    config_listener(p_lsnr, p_cfg);
    add_listener_for_signal_to_listener_list(p_cfg);
}

void FKMG_IO_CTRL_I2C_Write_Sample(struct FKMG_IO_CTRL_I2C_Instance * p_inst,
                                   enum FKMG_IO_CTRL_I2C_Id id,
                                   uint16_t sample)
{
    struct FKMG_IO_CTRL_I2C_SM_Evt evt = {
            .sig = k_FKMG_IO_CTRL_I2C_SM_Evt_Sig_Write_Sample,
            .data.write.id = id,
            .data.write.sample = sample
    };
    q_sm_event(p_inst, &evt);
}


#if CONFIG_FKMG_IO_CTRL_NO_OPTIMIZATIONS
#pragma GCC pop_options
#endif
