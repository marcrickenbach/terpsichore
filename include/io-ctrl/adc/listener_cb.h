/** ****************************************************************************
 * @brief FKMG ADC Control listener definition.
 */

#ifndef FKMG_IO_CTRL_ADC_LISTENER_CB_H
#define FKMG_IO_CTRL_ADC_LISTENER_CB_H

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
struct FKMG_IO_CTRL_ADC_Evt;

typedef void (*FKMG_IO_CTRL_ADC_Listener_Cb)(struct FKMG_IO_CTRL_ADC_Evt *p_evt);

#ifdef __cplusplus
}
#endif

#endif /* FKMG_IO_CTRL_ADC_LISTENER_CB_H */
