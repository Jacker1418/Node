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

volatile uint32_t cntTimer_1 = 0;
volatile uint32_t cntTimer_2 = 0;

// static void timeout_event_handler(NRF_TIMER_Type* in_timer)
// {
//     if(in_timer == NRF_TIMER1)
//     {
//         #ifdef DEBUG
//         NRF_LOG_INFO("[%s] %s(%d)", DEBUG_LOG_TAG, "Timer1 occur", cntTimer_1++);
//         #endif
//     }

//     if(in_timer == NRF_TIMER2)
//     {
//         #ifdef DEBUG
//         NRF_LOG_INFO("[%s] %s(%d)", DEBUG_LOG_TAG, "Timer2 occur", cntTimer_2++);
//         #endif
//     }
// }

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
