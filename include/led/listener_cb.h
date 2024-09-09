/** ****************************************************************************
 * @brief LED listener definition.
 */

#ifndef FKMG_LED_LISTENER_CB_H
#define FKMG_LED_LISTENER_CB_H

#ifdef __cplusplus
extern "C" {
#endif

/* *****************************************************************************
 * Includes
 */

#include <zephyr/kernel.h>

/* *****************************************************************************
 * Listener Callback Typedef
 */

/* Forward references to prevent include interdependent items getting declared
 * out-of-order. */
struct LED_Evt;

typedef void (*LED_Listener_Cb)(struct LED_Evt *p_evt);

#ifdef __cplusplus
}
#endif

#endif /* FKMG_LED_LISTENER_CB_H */
