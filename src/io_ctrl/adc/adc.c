/* *****************************************************************************
 * @brief FKMG I/O Control ADC implementation.
 * 
 */


/* *****************************************************************************
 * Includes
 */

#include "io-ctrl/adc/adc.h"
#include <zephyr/smf.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/adc.h>
#include <assert.h>
#include <stdlib.h>
#include "io-ctrl/adc/private/sm_evt.h"
#include "io-ctrl/adc/private/module_data.h"

/* *****************************************************************************
 * Constants, Defines, and Macros
 */

#define SUCCESS (0)
#define FAIL    (-1)

#if 1
#if !DT_NODE_EXISTS(DT_PATH(zephyr_user)) || \
	!DT_NODE_HAS_PROP(DT_PATH(zephyr_user), io_channels)
#error "No suitable devicetree overlay specified"
#endif

#define DT_SPEC_AND_COMMA(node_id, prop, idx) \
	ADC_DT_SPEC_GET_BY_IDX(node_id, idx),

/* Data of ADC io-channels specified in devicetree. */
static const struct adc_dt_spec adc_channels[] = {
	DT_FOREACH_PROP_ELEM(DT_PATH(zephyr_user), io_channels,
			     DT_SPEC_AND_COMMA)
};
#endif 

/* *****************************************************************************
 * Debugging
 */

#if CONFIG_FKMG_IO_CTRL_NO_OPTIMIZATIONS
#pragma GCC push_options
#pragma GCC optimize ("Og")
#warning "adc.c unoptimized!"
#endif

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(fkmg_io_ctrl_adc);

/* *****************************************************************************
 * Structs
 */

static struct fkmg_adc_module_data fkmg_adc_md = {0};
#define md fkmg_adc_md


/* *****************************************************************************
 * Private
 */

/* *****
 * Utils
 * *****/

static bool instance_contains_state_machine_ctx(
        struct FKMG_IO_CTRL_ADC_Instance * p_inst,
        struct smf_ctx      * p_sm_ctx)
{
    return(&p_inst->sm == p_sm_ctx);
}

/* Find the instance that contains the state machine context. */
static struct FKMG_IO_CTRL_ADC_Instance * sm_ctx_to_instance(struct smf_ctx * p_sm_ctx)
{
    /* Iterate thru the instances. */
    struct FKMG_IO_CTRL_ADC_Instance * p_inst = NULL;
    SYS_SLIST_FOR_EACH_CONTAINER(&md.list.instances, p_inst, node.instance){
        if(instance_contains_state_machine_ctx(p_inst, p_sm_ctx)) return p_inst;
    }
    return(NULL);
}

static enum FKMG_IO_CTRL_ADC_Id next_adc_channel(enum FKMG_IO_CTRL_ADC_Id id)
{
    return((id + 1) % k_FKMG_IO_CTRL_ADC_Id_Cnt);
}

/* **************
 * Listener Utils
 * **************/

static void clear_listener(struct FKMG_IO_CTRL_ADC_Listener * p_lsnr)
{
    memset(p_lsnr, 0, sizeof(*p_lsnr));
}

static void config_listener(
        struct FKMG_IO_CTRL_ADC_Listener     * p_lsnr,
        struct FKMG_IO_CTRL_ADC_Listener_Cfg * p_cfg)
{
    p_lsnr->p_inst = p_cfg->p_inst;
    p_lsnr->cb = p_cfg->cb;
}

static void init_listener(struct FKMG_IO_CTRL_ADC_Listener * p_lsnr)
{
    clear_listener(p_lsnr);
}

/* **************
 * Instance Utils
 * **************/

static void add_instance_to_instances(
        struct FKMG_IO_CTRL_ADC_Instance  * p_inst)
{
    sys_slist_append(&md.list.instances, &p_inst->node.instance);
}

static void config_instance_queues(
        struct FKMG_IO_CTRL_ADC_Instance     * p_inst,
        struct FKMG_IO_CTRL_ADC_Instance_Cfg * p_cfg)
{
    p_inst->msgq.p_sm_evts = p_cfg->msgq.p_sm_evts;
}

/* Forward reference */
static void on_conversion_timer_expiry(struct k_timer * p_timer);
/* Cause conversion event to occur immediately and then regularly after that. */
static void init_conversion_timer(struct FKMG_IO_CTRL_ADC_Instance * p_inst)
{
    #define ON_CONVERSION_TIMER_EXPIRY  on_conversion_timer_expiry
    #define ON_CONVERSION_TIMER_STOPPED NULL
    k_timer_init(&p_inst->timer.conversion, ON_CONVERSION_TIMER_EXPIRY,
            ON_CONVERSION_TIMER_STOPPED);
}

/* Forward reference */
static int init_io_ctrl_adc_device(struct FKMG_IO_CTRL_ADC_Instance * p_inst); 

static void config_instance_deferred(
        struct FKMG_IO_CTRL_ADC_Instance     * p_inst,
        struct FKMG_IO_CTRL_ADC_Instance_Cfg * p_cfg)
{
    // p_inst->adc_t_duration_ms = p_cfg->adc_t_duration_ms;
    // p_inst->adc_t_period_ms = p_cfg->adc_t_period_ms;
    init_io_ctrl_adc_device(p_inst);
    init_conversion_timer(p_inst);
}

static void config_instance_immediate(
        struct FKMG_IO_CTRL_ADC_Instance     * p_inst,
        struct FKMG_IO_CTRL_ADC_Instance_Cfg * p_cfg)
{
    config_instance_queues(p_inst, p_cfg);
}

static void init_instance_lists(struct FKMG_IO_CTRL_ADC_Instance * p_inst)
{
    for(enum FKMG_IO_CTRL_ADC_Evt_Sig sig = k_FKMG_IO_CTRL_ADC_Evt_Sig_Beg;
                         sig < k_FKMG_IO_CTRL_ADC_Evt_Sig_End;
                         sig++){
        sys_slist_init(&p_inst->list.listeners[sig]);
    }
}

static void clear_instance(struct FKMG_IO_CTRL_ADC_Instance * p_inst)
{
    memset(p_inst, 0, sizeof(*p_inst));
}

static void init_instance(struct FKMG_IO_CTRL_ADC_Instance * p_inst)
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
        struct FKMG_IO_CTRL_ADC_Evt      * p_evt,
        struct FKMG_IO_CTRL_ADC_Listener * p_lsnr)
{
    if(p_lsnr->cb)  p_lsnr->cb(p_evt);
}

static void broadcast_event_to_listeners(
        struct FKMG_IO_CTRL_ADC_Instance * p_inst,
        struct FKMG_IO_CTRL_ADC_Evt      * p_evt)
{
    enum FKMG_IO_CTRL_ADC_Evt_Sig sig = p_evt->sig;
    struct FKMG_IO_CTRL_ADC_Listener * p_lsnr = NULL;
    SYS_SLIST_FOR_EACH_CONTAINER(&p_inst->list.listeners[sig], p_lsnr, node.listener){
         broadcast(p_evt, p_lsnr);
    }
}

static void broadcast_instance_initialized(
        struct FKMG_IO_CTRL_ADC_Instance * p_inst,
        FKMG_IO_CTRL_ADC_Listener_Cb       cb)
{
    struct FKMG_IO_CTRL_ADC_Evt evt = {
            .sig = k_FKMG_IO_CTRL_ADC_Evt_Sig_Instance_Initialized,
            .data.initd.p_inst = p_inst
    };

    struct FKMG_IO_CTRL_ADC_Listener lsnr = {
        .p_inst = p_inst,
        .cb = cb
    };

    broadcast(&evt, &lsnr);
}

static void broadcast_adc_converted(
        struct FKMG_IO_CTRL_ADC_Instance * p_inst,
        enum FKMG_IO_CTRL_ADC_Id id,
        uint16_t val)
{
    struct FKMG_IO_CTRL_ADC_Evt evt = {
            .sig = k_FKMG_IO_CTRL_ADC_Evt_Sig_Converted,
            .data.converted.id = id,
            .data.converted.val = val
    };
    broadcast_event_to_listeners(p_inst, &evt);
}


/* **************
 * Listener Utils
 * **************/

static void add_listener_for_signal_to_listener_list(
    struct FKMG_IO_CTRL_ADC_Listener_Cfg * p_cfg)
{
    struct FKMG_IO_CTRL_ADC_Instance * p_inst = p_cfg->p_inst;

    struct FKMG_IO_CTRL_ADC_Listener * p_lsnr = p_cfg->p_lsnr;

    enum FKMG_IO_CTRL_ADC_Evt_Sig sig = p_cfg->sig;

    sys_slist_t * p_list = &p_inst->list.listeners[sig];
    sys_snode_t * p_node = &p_lsnr->node.listener;
    sys_slist_append(p_list, p_node);
}


/* **************
 * Event Queueing
 * **************/

static void q_sm_event(struct FKMG_IO_CTRL_ADC_Instance * p_inst, struct FKMG_IO_CTRL_ADC_SM_Evt * p_evt)
{
    bool queued = k_msgq_put(p_inst->msgq.p_sm_evts, p_evt, K_NO_WAIT) == 0;
    if(!queued) assert(false);
}

static void q_init_instance_event(struct FKMG_IO_CTRL_ADC_Instance_Cfg * p_cfg)
{
    struct FKMG_IO_CTRL_ADC_SM_Evt evt = {
            .sig = k_FKMG_IO_CTRL_ADC_SM_Evt_Sig_Init_Instance,
            .data.init_inst.cfg = *p_cfg
    };
    struct FKMG_IO_CTRL_ADC_Instance * p_inst = p_cfg->p_inst;
    q_sm_event(p_inst, &evt);
}

static void q_convert(struct FKMG_IO_CTRL_ADC_Instance * p_inst, enum FKMG_IO_CTRL_ADC_Id id)
{
    struct FKMG_IO_CTRL_ADC_SM_Evt evt = {
            .sig = k_FKMG_IO_CTRL_ADC_SM_Evt_Sig_Convert,
            .data.convert.id = id
    };
    q_sm_event(p_inst, &evt);
}

static void on_conversion_timer_expiry(struct k_timer * p_timer)
{
    struct FKMG_IO_CTRL_ADC_Instance * p_inst =
        CONTAINER_OF(p_timer, struct FKMG_IO_CTRL_ADC_Instance, timer.conversion);

    enum FKMG_IO_CTRL_ADC_Id nextADCId = next_adc_channel(p_inst->id);
    p_inst->id = nextADCId;
    q_convert(p_inst, nextADCId);
}

/* Cause conversion event to occur immediately and then regularly after that. */
static void start_conversion_timer(struct FKMG_IO_CTRL_ADC_Instance * p_inst)
{
    #define CONVERSION_INITIAL_DURATION     K_USEC(p_inst->adc_t_duration_us)
    #define CONVERSION_AUTO_RELOAD_PERIOD   K_USEC(p_inst->adc_t_period_us)
    k_timer_start(&p_inst->timer.conversion, CONVERSION_INITIAL_DURATION,
            CONVERSION_AUTO_RELOAD_PERIOD);
}

static int init_io_ctrl_adc_device(struct FKMG_IO_CTRL_ADC_Instance * p_inst)
{
    int err;
	uint32_t count = 0;
    int16_t * buf = p_inst->adc_buffer[ARRAY_SIZE(adc_channels)];

    /* Configure the sequence. */
    p_inst->sequence = (struct adc_sequence){
        .buffer = buf,
        .buffer_size = sizeof(buf),
    };

	/* Configure channels individually prior to sampling. */
	for (size_t i = 0U; i < ARRAY_SIZE(adc_channels); i++) {
		if (!adc_is_ready_dt(&adc_channels[i])) {
			LOG_ERR("ADC controller device %s not ready\n", adc_channels[i].dev->name);
			return FAIL;
		}

		err = adc_channel_setup_dt(&adc_channels[i]);
		if (err < 0) {
			LOG_ERR("Could not setup channel #%d (%d)\n", i, err);
			return err;
		}

        (void)adc_sequence_init_dt(&adc_channels[i], &p_inst->sequence);
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
    struct FKMG_IO_CTRL_ADC_Instance * p_inst = sm_ctx_to_instance(p_sm);

    /* Get the event. */
    struct FKMG_IO_CTRL_ADC_SM_Evt * p_evt = &p_inst->sm_evt;

    assert(p_evt->sig == k_FKMG_IO_CTRL_ADC_SM_Evt_Sig_Init_Instance);

    struct FKMG_IO_CTRL_ADC_SM_Evt_Sig_Init_Instance * p_ii = &p_evt->data.init_inst;
    
    config_instance_deferred(p_inst, &p_ii->cfg);
    broadcast_instance_initialized(p_inst, p_ii->cfg.cb);

    smf_set_state(SMF_CTX(p_sm), &states[run]);
}

static void state_run_entry(void * o)
{
    struct smf_ctx * p_sm = o;
    struct FKMG_IO_CTRL_ADC_Instance * p_inst = sm_ctx_to_instance(p_sm);
    start_conversion_timer(p_inst);
}

static void state_run_run(void * o)
{
    struct smf_ctx * p_sm = o;
    struct FKMG_IO_CTRL_ADC_Instance * p_inst = sm_ctx_to_instance(p_sm);

    /* Get the event. */
    struct FKMG_IO_CTRL_ADC_SM_Evt * p_evt = &p_inst->sm_evt;

    switch(p_evt->sig){
        default: break;
        case k_FKMG_IO_CTRL_ADC_SM_Evt_Sig_Init_Instance:
            /* Should never occur. */
            assert(false);
            break;
        case k_FKMG_IO_CTRL_ADC_SM_Evt_Sig_Convert:
            switch(p_evt->sig){
                default: break;
                case k_FKMG_IO_CTRL_ADC_SM_Evt_Sig_Init_Instance:
                    /* Should never occur. */
                    assert(false);
                    break;

                case k_FKMG_IO_CTRL_ADC_SM_Evt_Sig_Convert:
                    
                    struct FKMG_IO_CTRL_ADC_SM_Evt_Sig_Convert * p_convert = &p_evt->data.convert;

                    (void)adc_sequence_init_dt(&adc_channels[p_convert->id], &p_inst->sequence);
                    int err = adc_read_dt(&adc_channels[p_convert->id], &p_inst->sequence);
                    if (err < 0) {
                        LOG_ERR("Could not read (%d)\n", err);
                    }
                    break;
        }
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

static void thread(void * p_1, /* struct ADC Instance* */
        void * p_2_unused, void * p_3_unused)
{
    struct FKMG_IO_CTRL_ADC_Instance * p_inst = p_1;

    struct smf_ctx * p_sm = &p_inst->sm;
    smf_set_initial(SMF_CTX(p_sm), &states[init]);

    struct k_msgq * p_msgq = p_inst->msgq.p_sm_evts;
    struct FKMG_IO_CTRL_ADC_SM_Evt * p_evt = &p_inst->sm_evt;

    bool run = true;

    while(run){
        k_msgq_get(p_msgq, p_evt, K_FOREVER);
        run = smf_run_state(SMF_CTX(p_sm)) == 0;
    }
}

static void start_thread(
        struct FKMG_IO_CTRL_ADC_Instance     * p_inst,
        struct FKMG_IO_CTRL_ADC_Instance_Cfg * p_cfg)
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
    
void FKMG_IO_CTRL_ADC_Init_Instance(struct FKMG_IO_CTRL_ADC_Instance_Cfg * p_cfg)
{
    /* Get pointer to instance to configure. */
    struct FKMG_IO_CTRL_ADC_Instance * p_inst = p_cfg->p_inst;

    init_module();
    init_instance(p_inst);
    config_instance_immediate(p_inst, p_cfg);
    add_instance_to_instances(p_inst);
    start_thread(p_inst, p_cfg);
    q_init_instance_event(p_cfg);
}

void FKMG_IO_CTRL_ADC_Add_Listener(struct FKMG_IO_CTRL_ADC_Listener_Cfg * p_cfg)
{
    struct FKMG_IO_CTRL_ADC_Listener * p_lsnr = p_cfg->p_lsnr;
    init_listener(p_lsnr);
    config_listener(p_lsnr, p_cfg);
    add_listener_for_signal_to_listener_list(p_cfg);
}

uint16_t FKMG_IO_CTRL_ADC_Get_Value(struct FKMG_IO_CTRL_ADC_Instance * p_inst, 
                                    enum   FKMG_IO_CTRL_ADC_Id id)
{
   return (uint16_t)p_inst->adc_buffer[id];
}

#if CONFIG_FKMG_IO_CTRL_NO_OPTIMIZATIONS
#pragma GCC pop_options
#endif
