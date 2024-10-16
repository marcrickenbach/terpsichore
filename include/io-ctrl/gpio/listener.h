/** ****************************************************************************
 * @brief FKMG GPIO Control listener instance interface.
 */

#ifndef FKMG_IO_CTRL_GPIO_LISTENER_H
#define FKMG_IO_CTRL_GPIO_LISTENER_H

#ifdef __cplusplus
extern "C" {
#endif

/* *****************************************************************************
 * Includes
 */

#include <zephyr/kernel.h>
#include "listener_cb.h"

/* *****************************************************************************
 * Listener
 */

/* Forward references to prevent include interdependent items getting declared
 * out-of-order. */
struct FKMG_IO_CTRL_GPIO_Instance;

struct FKMG_IO_CTRL_GPIO_Listener{
    /* Listener instances are kept track in a singly-linked list. */
    struct{
        sys_snode_t listener;
    }node;

    /* Pointer to instance this listener is listening to. */
    struct FKMG_IO_CTRL_GPIO_Instance * p_inst;

    /* Listener callback. */
    FKMG_IO_CTRL_GPIO_Listener_Cb cb;
};

#ifdef __cplusplus
}
#endif

#endif /* FKMG_IO_CTRL_GPIO_LISTENER_H */