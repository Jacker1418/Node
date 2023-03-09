#ifndef __COMMON_INIT_H__
#define __COMMON_INIT_H__

#include "nordic_common.h"
#include "nrf.h"

#include "sdk_errors.h"

struct interface_peripheral
{

};

/**@brief  The initialize peripheral such as UART, Timer, GPIO, etc...
 *
 * @param[in]   None
 * @param[out]  None
 *
 * @retval  NRF_SUCCESS    
 * 
 */
ret_code_t init_Peripheral(void);

#endif

/* Define functions [Form] */
/**@brief  The description of function 
 *
 * @param[in]   This is the description about input of parameter
 * @param[out]  output through parameter using pointer
 *
 * @retval  NRF_SUCCESS    
 * 
 * @warning
 * 
 * @note
 */