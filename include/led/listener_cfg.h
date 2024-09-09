/** ****************************************************************************
 * @brief LED listener configuration interface.
 */

#ifndef FKMG_LED_LISTENER_CB_CFG_H
#define FKMG_LED_LISTENER_CB_CFG_H

#ifdef __cplusplus
extern "C" {
#endif

/* *****************************************************************************
 * Includes
 */

#include <zephyr/kernel.h>
#include "listener_cb.h"
#include "evt.h"

/* *****************************************************************************
 * Listener Callback Typedef
 */

/* Forward references to prevent include interdependent items getting declared
 * out-of-order. */
struct LED_Instance;
struct LED_Listener;

struct LED_Listener_Cfg{
    /* Required: pointer to initialized opaque instace we'll add listener to. */
    struct LED_Instance * p_inst;

    /* Required: pointer to uninitialized/unused opaque listener to config. */
    struct LED_Listener * p_lsnr;

    /* Required: event signal to listen for. */
    enum LED_Evt_Sig sig;

    /* Required: function to call back and send event to when signal occurs. */
    LED_Listener_Cb cb;
};

#ifdef __cplusplus
}
#endif

#endif /* FKMG_LED_LISTENER_CB_CFG_H */
