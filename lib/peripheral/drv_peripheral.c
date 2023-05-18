#include "drv_peripheral.h"

#include "drv_gpio.h"
#include "drv_timer.h"
#include "drv_uart.h"

#ifdef DEBUG
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#endif

#define DEBUG_LOG_TAG "peripheral"

struct drv_interface GPIO;

struct drv_interface insUARTE;

/**@brief PCB's peripheral initialize

 * @retval  NRF_SUCCESS                    
 * 
 * @warning
 * 
 * @note
 */
ret_code_t init_Peripheral(void)
{
    ret_code_t result = NRF_SUCCESS;

#ifdef DEBUG 
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
#endif

    init_GPIO(&GPIO);

    init_UARTE(&insUARTE, NULL);

    return result;
}