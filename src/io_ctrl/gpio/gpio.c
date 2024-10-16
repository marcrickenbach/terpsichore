/* *****************************************************************************
 * @brief FKMG I/O Control GPIO implementation.
 * 
 */


/* *****************************************************************************
 * Includes
 */

#include "io-ctrl/gpio/gpio.h"
#include <zephyr/smf.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <assert.h>
#include <stdlib.h>
#include "io-ctrl/gpio/private/sm_evt.h"
#include "io-ctrl/gpio/private/module_data.h"

/* *****************************************************************************
 * Constants, Defines, and Macros
 */

#define SUCCESS (0)
#define FAIL    (-1)

#define FKMG_GPIO_PINS               DT_PATH(zephyr_user)

#if !defined(ARRAY_SIZE)
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

#define DT_SPEC_AND_COMMA(node_id, prop, idx) \
	GPIO_DT_SPEC_GET_BY_IDX(node_id, prop, idx),

/** GPIO OUTPUT ARRAY */
static const struct gpio_dt_spec fkmg_gpio[] = {
    DT_FOREACH_PROP_ELEM(FKMG_GPIO_PINS, fkmg_gpios, DT_SPEC_AND_COMMA)
};


/* *****************************************************************************
 * Debugging
 */

#if CONFIG_FKMG_IO_CTRL_NO_OPTIMIZATIONS
#pragma GCC push_options
#pragma GCC optimize ("Og")
#warning "gpio.c unoptimized!"
#endif

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(fkmg_io_ctrl_gpio);

/* *****************************************************************************
 * Structs
 */

static struct fkmg_gpio_module_data fkmg_gpio_md = {0};
#define md fkmg_gpio_md

#if !defined(ARRAY_SIZE)
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

#define DT_SPEC_AND_COMMA(node_id, prop, idx) \
	GPIO_DT_SPEC_GET_BY_IDX(node_id, prop, idx),

/** GPIO ARRAY */
static const struct gpio_dt_spec gpio_pins[] = {
    DT_FOREACH_PROP_ELEM(FKMG_GPIO_PINS, fkmg_gpios, DT_SPEC_AND_COMMA)
};

/* *****************************************************************************
 * Private
 */

/* *****
 * Utils
 * *****/

static bool instance_contains_state_machine_ctx(
        struct FKMG_IO_CTRL_GPIO_Instance * p_inst,
        struct smf_ctx      * p_sm_ctx)
{
    return(&p_inst->sm == p_sm_ctx);
}

/* Find the instance that contains the state machine context. */
static struct FKMG_IO_CTRL_GPIO_Instance * sm_ctx_to_instance(struct smf_ctx * p_sm_ctx)
{
    /* Iterate thru the instances. */
    struct FKMG_IO_CTRL_GPIO_Instance * p_inst = NULL;
    SYS_SLIST_FOR_EACH_CONTAINER(&md.list.instances, p_inst, node.instance){
        if(instance_contains_state_machine_ctx(p_inst, p_sm_ctx)) return p_inst;
    }
    return(NULL);
}


/* **************
 * Listener Utils
 * **************/

static void clear_listener(struct FKMG_IO_CTRL_GPIO_Listener * p_lsnr)
{
    memset(p_lsnr, 0, sizeof(*p_lsnr));
}

static void config_listener(
        struct FKMG_IO_CTRL_GPIO_Listener     * p_lsnr,
        struct FKMG_IO_CTRL_GPIO_Listener_Cfg * p_cfg)
{
    /* Set listner's instance it is listening to. */
    p_lsnr->p_inst = p_cfg->p_inst;

    /* Set listner's callback. */ 
    p_lsnr->cb = p_cfg->cb;
}

static void init_listener(struct FKMG_IO_CTRL_GPIO_Listener * p_lsnr)
{
    clear_listener(p_lsnr);
}

/* **************
 * Instance Utils
 * **************/

static void add_instance_to_instances(
        struct FKMG_IO_CTRL_GPIO_Instance  * p_inst)
{
    sys_slist_append(&md.list.instances, &p_inst->node.instance);
}

static void config_instance_queues(
        struct FKMG_IO_CTRL_GPIO_Instance     * p_inst,
        struct FKMG_IO_CTRL_GPIO_Instance_Cfg * p_cfg)
{
    p_inst->msgq.p_sm_evts = p_cfg->msgq.p_sm_evts;
}

static void config_instance_immediate(
        struct FKMG_IO_CTRL_GPIO_Instance     * p_inst,
        struct FKMG_IO_CTRL_GPIO_Instance_Cfg * p_cfg)
{
    config_instance_queues(p_inst, p_cfg);
}

static void init_instance_lists(struct FKMG_IO_CTRL_GPIO_Instance * p_inst)
{
    for(enum FKMG_IO_CTRL_GPIO_Evt_Sig sig = k_FKMG_IO_CTRL_GPIO_Evt_Sig_Beg;
                         sig < k_FKMG_IO_CTRL_GPIO_Evt_Sig_End;
                         sig++){
        sys_slist_init(&p_inst->list.listeners[sig]);
    }
}

static void clear_instance(struct FKMG_IO_CTRL_GPIO_Instance * p_inst)
{
    memset(p_inst, 0, sizeof(*p_inst));
}

static void init_instance(struct FKMG_IO_CTRL_GPIO_Instance * p_inst)
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
        struct FKMG_IO_CTRL_GPIO_Evt      * p_evt,
        struct FKMG_IO_CTRL_GPIO_Listener * p_lsnr)
{
    if(p_lsnr->cb)  p_lsnr->cb(p_evt);
}

static void broadcast_event_to_listeners(
        struct FKMG_IO_CTRL_GPIO_Instance * p_inst,
        struct FKMG_IO_CTRL_GPIO_Evt      * p_evt)
{
    enum FKMG_IO_CTRL_GPIO_Evt_Sig sig = p_evt->sig;
    struct FKMG_IO_CTRL_GPIO_Listener * p_lsnr = NULL;
    SYS_SLIST_FOR_EACH_CONTAINER(&p_inst->list.listeners[sig], p_lsnr, node.listener){
         broadcast(p_evt, p_lsnr);
    }
}

static void broadcast_instance_initialized(
        struct FKMG_IO_CTRL_GPIO_Instance * p_inst,
        FKMG_IO_CTRL_GPIO_Listener_Cb       cb)
{
    struct FKMG_IO_CTRL_GPIO_Evt evt = {
            .sig = k_FKMG_IO_CTRL_GPIO_Evt_Sig_Instance_Initialized,
            .data.initd.p_inst = p_inst
    };

    struct FKMG_IO_CTRL_GPIO_Listener lsnr = {
        .p_inst = p_inst,
        .cb = cb
    };

    broadcast(&evt, &lsnr);
}


/* **************
 * Listener Utils
 * **************/

static void add_listener_for_signal_to_listener_list(
    struct FKMG_IO_CTRL_GPIO_Listener_Cfg * p_cfg)
{
    /* Get pointer to interface to add listener to. */
    struct FKMG_IO_CTRL_GPIO_Instance * p_inst = p_cfg->p_inst;

    /* Get pointer to configured listener. */
    struct FKMG_IO_CTRL_GPIO_Listener * p_lsnr = p_cfg->p_lsnr;

    /* Get signal to listen for. */
    enum FKMG_IO_CTRL_GPIO_Evt_Sig sig = p_cfg->sig;

    /* Add listener to instance's specified signal. */
    sys_slist_t * p_list = &p_inst->list.listeners[sig];
    sys_snode_t * p_node = &p_lsnr->node.listener;
    sys_slist_append(p_list, p_node);
}


/* **************
 * Event Queueing
 * **************/

static void q_sm_event(struct FKMG_IO_CTRL_GPIO_Instance * p_inst, struct FKMG_IO_CTRL_GPIO_SM_Evt * p_evt)
{
    bool queued = k_msgq_put(p_inst->msgq.p_sm_evts, p_evt, K_NO_WAIT) == 0;

    if(!queued) assert(false);
}

static void q_init_instance_event(struct FKMG_IO_CTRL_GPIO_Instance_Cfg * p_cfg)
{
    struct FKMG_IO_CTRL_GPIO_SM_Evt evt = {
            .sig = k_FKMG_IO_CTRL_GPIO_SM_Evt_Sig_Init_Instance,
            .data.init_inst.cfg = *p_cfg
    };
    struct FKMG_IO_CTRL_GPIO_Instance * p_inst = p_cfg->p_inst;

    q_sm_event(p_inst, &evt);
}

static void gpio_callback_handler(const struct device * port, struct gpio_callback * cb, uint32_t pins)
{
    struct FKMG_IO_CTRL_GPIO_Instance * p_inst = CONTAINER_OF(&cb, struct FKMG_IO_CTRL_GPIO_Instance, cb);

    struct FKMG_IO_CTRL_GPIO_Evt evt = {
            .sig = k_FKMG_IO_CTRL_GPIO_Evt_Sig_GPIO_Interrupt,
            .data.interrupt.pin = pins,
    };
    broadcast_event_to_listeners(p_inst, &evt);
}

static int init_io_ctrl_gpio_device(struct FKMG_IO_CTRL_GPIO_Instance * p_inst)
{
    int ret; 

    p_inst->cb[ARRAY_SIZE(fkmg_gpio)];

    for (int i = 0; i < ARRAY_SIZE(fkmg_gpio); i++) {
        
        /* Iterate through fkmg_gpio. Check to see if device(s) is ready. */
        if(!device_is_ready(fkmg_gpio[i].port)){
            LOG_ERR("Device %s not ready\n", fkmg_gpio[i].port->name);
            return ret;
        }

        uint32_t flags = fkmg_gpio[i].dt_flags;

        gpio_pin_configure(fkmg_gpio[i].port, fkmg_gpio[i].pin, flags);

        /* If flag shows INPUT configure callbacks. */
        uint32_t cb_flag;

        if(flags & GPIO_INPUT){
            gpio_pin_interrupt_configure(fkmg_gpio[i].port, fkmg_gpio[i].pin, GPIO_INT_EDGE_TO_ACTIVE);
            gpio_init_callback(&p_inst->cb[i], gpio_callback_handler, BIT(fkmg_gpio[i].pin));
            gpio_add_callback(fkmg_gpio[i].port, &p_inst->cb[i]);
        }
    }
    return SUCCESS;
}


/* **********
 * FSM States
 * **********/

static const struct smf_state states[];
enum state{
    init,
    run,
    deinit,
};

static void state_init_run(void * o)
{
    struct smf_ctx * p_sm = o;
    struct FKMG_IO_CTRL_GPIO_Instance * p_inst = sm_ctx_to_instance(p_sm);

    struct FKMG_IO_CTRL_GPIO_SM_Evt * p_evt = &p_inst->sm_evt;

    assert(p_evt->sig == k_FKMG_IO_CTRL_GPIO_SM_Evt_Sig_Init_Instance);

    struct FKMG_IO_CTRL_GPIO_SM_Evt_Sig_Init_Instance * p_ii = &p_evt->data.init_inst;
    
    broadcast_instance_initialized(p_inst, p_ii->cfg.cb);

    int ret = init_io_ctrl_gpio_device(p_inst);
    if (ret != SUCCESS) {
        LOG_ERR("Failed to initialize GPIO device\n");
        return;
    }

    smf_set_state(SMF_CTX(p_sm), &states[run]);
}

static void state_run_entry(void * o)
{
    struct smf_ctx * p_sm = o;
    struct FKMG_IO_CTRL_GPIO_Instance * p_inst = sm_ctx_to_instance(p_sm);
}

static void state_run_run(void * o)
{
    struct smf_ctx * p_sm = o;
    struct FKMG_IO_CTRL_GPIO_Instance * p_inst = sm_ctx_to_instance(p_sm);

    struct FKMG_IO_CTRL_GPIO_SM_Evt * p_evt = &p_inst->sm_evt;

    switch(p_evt->sig){
        default: break;
        case k_FKMG_IO_CTRL_GPIO_SM_Evt_Sig_Init_Instance:
            assert(false);
            break;
        case k_FKMG_IO_CTRL_GPIO_SM_Evt_Sig_Write_State:

            break;

    }
}

static const struct smf_state states[] = {
    [  init] = SMF_CREATE_STATE(NULL, state_init_run, NULL, NULL, NULL),
    [   run] = SMF_CREATE_STATE(state_run_entry, state_run_run, NULL, NULL, NULL),
};

/* ******
 * Thread
 * ******/

static void thread(void * p_1, /* struct GPIO Instance* */
        void * p_2_unused, void * p_3_unused)
{
    struct FKMG_IO_CTRL_GPIO_Instance * p_inst = p_1;
    struct smf_ctx * p_sm = &p_inst->sm;
    smf_set_initial(SMF_CTX(p_sm), &states[init]);

    struct k_msgq * p_msgq = p_inst->msgq.p_sm_evts;
    struct FKMG_IO_CTRL_GPIO_SM_Evt * p_evt = &p_inst->sm_evt;

    bool run = true;

    while(run){
        k_msgq_get(p_msgq, p_evt, K_FOREVER);
        run = smf_run_state(SMF_CTX(p_sm)) == 0;
    }
}

static void start_thread(
        struct FKMG_IO_CTRL_GPIO_Instance     * p_inst,
        struct FKMG_IO_CTRL_GPIO_Instance_Cfg * p_cfg)
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
    
void FKMG_IO_CTRL_GPIO_Init_Instance(struct FKMG_IO_CTRL_GPIO_Instance_Cfg * p_cfg)
{
    /* Get pointer to instance to configure. */
    struct FKMG_IO_CTRL_GPIO_Instance * p_inst = p_cfg->p_inst;

    init_module();
    init_instance(p_inst);
    config_instance_immediate(p_inst, p_cfg);
    add_instance_to_instances(p_inst);
    start_thread(p_inst, p_cfg);
    q_init_instance_event(p_cfg);
}

void FKMG_IO_CTRL_GPIO_Add_Listener(struct FKMG_IO_CTRL_GPIO_Listener_Cfg * p_cfg)
{
    struct FKMG_IO_CTRL_GPIO_Listener * p_lsnr = p_cfg->p_lsnr;
    init_listener(p_lsnr);
    config_listener(p_lsnr, p_cfg);
    add_listener_for_signal_to_listener_list(p_cfg);
}

void FKMG_IO_CTRL_GPIO_Write_State(struct FKMG_IO_CTRL_GPIO_Instance * p_inst,
                                   enum FKMG_IO_CTRL_GPIO_Id id,
                                   enum FKMG_IO_CTRL_GPIO_State state)
{
    /* Check that GPIO is Output. */
    if (fkmg_gpio[id].dt_flags & GPIO_INPUT) return;

    struct FKMG_IO_CTRL_GPIO_SM_Evt evt = {
            .sig = k_FKMG_IO_CTRL_GPIO_SM_Evt_Sig_Write_State,
            .data.write.id = id,
            .data.write.state = state
    };
    q_sm_event(p_inst, &evt);
}
 
#if CONFIG_FKMG_IO_CTRL_NO_OPTIMIZATIONS
#pragma GCC pop_options
#endif
