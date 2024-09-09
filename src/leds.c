/* *****************************************************************************
 * @brief LED implementation.
 * 
 */


/* *****************************************************************************
 * Includes
 */

#include "leds.h"
#include <zephyr/smf.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <assert.h>
#include <stdlib.h>
#include "terpsichore/leds/private/sm_evt.h"
#include "terpsichore/leds/private/module_data.h"

/* *****************************************************************************
 * Constants, Defines, and Macros
 */

#define SUCCESS (0)
#define FAIL    (-1)

#define LED_PINS               DT_PATH(zephyr_user)
#define MODE_BITS              5
#define SUB_BITS               5
#define MODE_MAX               (1 << MODE_BITS)
/* *****************************************************************************
 * Debugging
 */

#if CONFIG_FKMG_LED_NO_OPTIMIZATIONS
#pragma GCC push_options
#pragma GCC optimize ("Og")
#warning "led.c unoptimized!"
#endif

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(led);

/* *****************************************************************************
 * Structs
 */

static struct led_module_data led_md = {0};
#define md led_md

#if !defined(ARRAY_SIZE)
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

#define DT_SPEC_AND_COMMA(node_id, prop, idx) \
	GPIO_DT_SPEC_GET_BY_IDX(node_id, prop, idx),

/** LED ARRAY */
static const struct gpio_dt_spec led_pins[] = {
    DT_FOREACH_PROP_ELEM(LED_PINS, led_gpios, DT_SPEC_AND_COMMA)
};

/* *****************************************************************************
 * Private
 */

/* *****
 * Utils
 * *****/

static bool instance_contains_state_machine_ctx(
        struct LED_Instance * p_inst,
        struct smf_ctx      * p_sm_ctx)
{
    return(&p_inst->sm == p_sm_ctx);
}

/* Find the instance that contains the state machine context. */
static struct LED_Instance * sm_ctx_to_instance(struct smf_ctx * p_sm_ctx)
{
    /* Iterate thru the instances. */
    struct LED_Instance * p_inst = NULL;
    SYS_SLIST_FOR_EACH_CONTAINER(&md.list.instances, p_inst, node.instance){
        if(instance_contains_state_machine_ctx(p_inst, p_sm_ctx)) return p_inst;
    }
    return(NULL);
}


/* **************
 * Listener Utils
 * **************/

static void clear_listener(struct LED_Listener * p_lsnr)
{
    memset(p_lsnr, 0, sizeof(*p_lsnr));
}

static void config_listener(
        struct LED_Listener     * p_lsnr,
        struct LED_Listener_Cfg * p_cfg)
{
    /* Set listner's instance it is listening to. */
    p_lsnr->p_inst = p_cfg->p_inst;

    /* Set listner's callback. */ 
    p_lsnr->cb = p_cfg->cb;
}

static void init_listener(struct LED_Listener * p_lsnr)
{
    clear_listener(p_lsnr);
}

/* **************
 * Instance Utils
 * **************/

static void add_instance_to_instances(
        struct LED_Instance  * p_inst)
{
    sys_slist_append(&md.list.instances, &p_inst->node.instance);
}

static void config_instance_queues(
        struct LED_Instance     * p_inst,
        struct LED_Instance_Cfg * p_cfg)
{
    p_inst->msgq.p_sm_evts = p_cfg->msgq.p_sm_evts;
}

static void config_instance_deferred(
        struct LED_Instance     * p_inst,
        struct LED_Instance_Cfg * p_cfg)
{
}

/* Since configuration starts on caller's thread, configure fields that require
 * immediate and/or inconsequential configuration and defer rest to be handled
 * by our own thread later. */
static void config_instance_immediate(
        struct LED_Instance     * p_inst,
        struct LED_Instance_Cfg * p_cfg)
{
    config_instance_queues(p_inst, p_cfg);
}

static void init_instance_lists(struct LED_Instance * p_inst)
{
    for(enum LED_Evt_Sig sig = k_LED_Evt_Sig_Beg;
                         sig < k_LED_Evt_Sig_End;
                         sig++){
        sys_slist_init(&p_inst->list.listeners[sig]);
    }
}

static void clear_instance(struct LED_Instance * p_inst)
{
    memset(p_inst, 0, sizeof(*p_inst));
}

static void init_instance(struct LED_Instance * p_inst)
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
        struct LED_Evt      * p_evt,
        struct LED_Listener * p_lsnr)
{
    /* call the listener, passing the event */
    if(p_lsnr->cb)  p_lsnr->cb(p_evt);
}

static void broadcast_event_to_listeners(
        struct LED_Instance * p_inst,
        struct LED_Evt      * p_evt)
{
    enum LED_Evt_Sig sig = p_evt->sig;
    struct LED_Listener * p_lsnr = NULL;
    SYS_SLIST_FOR_EACH_CONTAINER(&p_inst->list.listeners[sig], p_lsnr, node.listener){
         broadcast(p_evt, p_lsnr);
    }
}

/* There's only 1 listener for instance initialization, and it is provided in
 * the cfg struct. */
static void broadcast_instance_initialized(
        struct LED_Instance * p_inst,
        LED_Listener_Cb       cb)
{
    struct LED_Evt evt = {
            .sig = k_LED_Evt_Sig_Instance_Initialized,
            .data.initd.p_inst = p_inst
    };

    struct LED_Listener lsnr = {
        .p_inst = p_inst,
        .cb = cb
    };

    broadcast(&evt, &lsnr);
}


static void broadcast_led_changed(
        struct LED_Instance * p_inst,
        enum LED_Id           id,
        uint16_t              val)
{

    struct LED_Evt evt = {
            .sig = k_LED_Evt_Sig_Changed,
            .data.changed.id = id,
            .data.changed.val = val
    };

    broadcast_event_to_listeners(p_inst, &evt);

}

/* **************
 * Listener Utils
 * **************/

static void add_listener_for_signal_to_listener_list(
    struct LED_Listener_Cfg * p_cfg)
{
    /* Get pointer to interface to add listener to. */
    struct LED_Instance * p_inst = p_cfg->p_inst;

    /* Get pointer to configured listener. */
    struct LED_Listener * p_lsnr = p_cfg->p_lsnr;

    /* Get signal to listen for. */
    enum LED_Evt_Sig sig = p_cfg->sig;

    /* Add listener to instance's specified signal. */
    sys_slist_t * p_list = &p_inst->list.listeners[sig];
    sys_snode_t * p_node = &p_lsnr->node.listener;
    sys_slist_append(p_list, p_node);
}



/* **************
 * Event Queueing
 * **************/

static void q_sm_event(struct LED_Instance * p_inst, struct LED_SM_Evt * p_evt)
{
    bool queued = k_msgq_put(p_inst->msgq.p_sm_evts, p_evt, K_NO_WAIT) == 0;

    if(!queued) assert(false);
}

static void q_init_instance_event(struct LED_Instance_Cfg * p_cfg)
{
    struct LED_SM_Evt evt = {
            .sig = k_LED_SM_Evt_Sig_Init_Instance,
            .data.init_inst.cfg = *p_cfg
    };
    struct LED_Instance * p_inst = p_cfg->p_inst;

    q_sm_event(p_inst, &evt);
}

static int LED_Array_Init(struct LED_Instance *p_inst) 
{
    int ret = -1; 
    for (int i = 0; i < ARRAY_SIZE(led_pins); i++) {
        
        if(!device_is_ready(led_pins[i].port)){
            LOG_ERR("Device %s not ready\n", led_pins[i].port->name);
            return ret;
        }

        gpio_pin_configure(led_pins[i].port, led_pins[i].pin, GPIO_OUTPUT | GPIO_PULL_DOWN);
    }
    return ret = 0; 
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
    struct LED_Instance * p_inst = sm_ctx_to_instance(p_sm);

    /* Get the event. */
    struct LED_SM_Evt * p_evt = &p_inst->sm_evt;

    /* Expecting only an "init instance" event. Anything else is an error. */
    assert(p_evt->sig == k_LED_SM_Evt_Sig_Init_Instance);

    /* We init'd required params on the caller's thread (i.e.
     * LED_Init_Instance()), now finish the job. Since this is an
     * Init_Instance event the data contains the intance cfg. */
    struct LED_SM_Evt_Sig_Init_Instance * p_ii = &p_evt->data.init_inst;
    
    config_instance_deferred(p_inst, &p_ii->cfg);
    broadcast_instance_initialized(p_inst, p_ii->cfg.cb);

    LED_Array_Init(p_inst);

    smf_set_state(SMF_CTX(p_sm), &states[run]);
}

/* Run state responsibility is to be the root state to handle everything else
 * other than instance initialization. */

static void state_run_entry(void * o)
{
    struct smf_ctx * p_sm = o;
    struct LED_Instance * p_inst = sm_ctx_to_instance(p_sm);
}

static void state_run_run(void * o)
{
    struct smf_ctx * p_sm = o;
    struct LED_Instance * p_inst = sm_ctx_to_instance(p_sm);

    /* Get the event. */
    struct LED_SM_Evt * p_evt = &p_inst->sm_evt;

    switch(p_evt->sig){
        default: break;
        case k_LED_SM_Evt_Sig_Init_Instance:
            /* Should never occur. */
            assert(false);
            break;
        case k_LED_SM_Evt_Sig_Write:
            
            struct LED_SM_Evt_Sig_Write * p_convert = &p_evt->data.write;

            /* Write the value to the LED. */
            if (p_convert->row == Mode) {

                for (int i = 0; i < MODE_BITS; i++) 
                {
                    bool state = (p_convert->val >> i) & 1;
                    gpio_pin_set(led_pins[i].port, led_pins[i].pin, state);
                }

            } else {
                for (int k = SUB_BITS; k < ARRAY_SIZE(led_pins); k++) 
                {
                    bool state = (p_convert->val >> k) & 1;
                    gpio_pin_set(led_pins[k].port, led_pins[k].pin, state);
                }
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

static void thread(void * p_1, /* struct LED_Instance* */
        void * p_2_unused, void * p_3_unused)
{
    struct LED_Instance * p_inst = p_1;
    /* NOTE: smf_set_initial() executes the entry state. */
    struct smf_ctx * p_sm = &p_inst->sm;
    smf_set_initial(SMF_CTX(p_sm), &states[init]);

    /* Get the state machine event queue and point to where to put the dequeued
     * event. */
    struct k_msgq * p_msgq = p_inst->msgq.p_sm_evts;
    struct LED_SM_Evt * p_evt = &p_inst->sm_evt;

    bool run = true;

    while(run){
        /* Wait on state machine event. Cache it then run state machine. */
        k_msgq_get(p_msgq, p_evt, K_FOREVER);
        run = smf_run_state(SMF_CTX(p_sm)) == 0;
    }
}

static void start_thread(
        struct LED_Instance     * p_inst,
        struct LED_Instance_Cfg * p_cfg)
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

void LED_Init_Instance(struct LED_Instance_Cfg * p_cfg)
{
    /* Get pointer to instance to configure. */
    struct LED_Instance * p_inst = p_cfg->p_inst;

    init_module();
    init_instance(p_inst);
    config_instance_immediate(p_inst, p_cfg);
    add_instance_to_instances(p_inst);
    start_thread(p_inst, p_cfg);
    q_init_instance_event(p_cfg);
}

void LED_Add_Listener(struct LED_Listener_Cfg * p_cfg)
{
    struct LED_Listener * p_lsnr = p_cfg->p_lsnr;
    init_listener(p_lsnr);
    config_listener(p_lsnr, p_cfg);
    add_listener_for_signal_to_listener_list(p_cfg);
}

void LED_Write_Modes(struct LED_Instance * p_inst, enum LED_Id row, uint8_t val)
{
    if(val > MODE_MAX || row >= k_LED_Id_End) return;

    struct LED_SM_Evt evt = {
            .sig = k_LED_SM_Evt_Sig_Write,
            .data.write.row = row,
            .data.write.val = val
    };
    q_sm_event(p_inst, &evt);
}

#if CONFIG_FKMG_POT_NO_OPTIMIZATIONS
#pragma GCC pop_options
#endif
