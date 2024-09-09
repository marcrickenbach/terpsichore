/** ****************************************************************************
 * @brief FKMG GPIO Control listener configuration interface.
 */

#ifndef FKMG_IO_CTRL_GPIO_LISTENER_CB_CFG_H
#define FKMG_IO_CTRL_GPIO_LISTENER_CB_CFG_H

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
struct FKMG_IO_CTRL_GPIO_Instance;

struct FKMG_IO_CTRL_GPIO_Listener;

struct FKMG_IO_CTRL_GPIO_Listener_Cfg{
    /* Required: pointer to initialized opaque instace we'll add listener to. */
    struct FKMG_IO_CTRL_GPIO_Instance * p_inst;

    /* Required: pointer to uninitialized/unused opaque listener to config. */
    struct FKMG_IO_CTRL_GPIO_Listener * p_lsnr;

    /* Required: event signal to listen for. */
    enum FKMG_IO_CTRL_GPIO_Evt_Sig sig;

    /* Required: function to call back and send event to when signal occurs. */
    FKMG_IO_CTRL_GPIO_Listener_Cb cb;
};

#ifdef __cplusplus
}
#endif

#endif /* FKMG_IO_CTRL_GPIO_LISTENER_CB_CFG_H */
