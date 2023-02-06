#ifndef __COMMON_INIT_H__
#define __COMMON_INIT_H__

#include "nordic_common.h"
#include "nrf.h"

#include "sdk_errors.h"

enum PIN_MAP
{
    #if NRF52_DK
        SERIAL_RX       = 8, 
        SERIAL_TX       = 6,
    #elif MBN52_NODE
        SERIAL_RX       = 7,
        SERIAL_TX       = 8,
        
        PHOLD_SWITCH    = 0,    // @note Low frequency XTAL를 비활성화해야 사용 가능하다.
        PHOLD           = 1,    // @note Low frequency XTAL를 비활성화해야 사용 가능하다.

        BLE_STAT        = 17,
        CHG_STAT        = 13,

        SCL             = 15,
        SDA             = 14,

        BAT_MON_EN      = 16,   // @brief Battery ADC Net을 Open/Close하는 MOSFET 제어
        BAT_CHECK       = 2,    // @brief ADC0와 맴핑

        USB_DETECT      = 18,   // @brief USB 전원 공급 시 High / USB 전원 공급 해제시 Low

        /* @note P0.09를 사용시에는 P0.10과 함께 사용되어야 한다. nRF52832 PS datasheet의 NFC 파트 참고 */
        RESET           = 9,    // @brief SX1509의 Reset 단자 제어

        MEDIUM          = 29,   // @brief ADC5와 맴핑
        SENSOR_IN       = 4,    // @brief ADC2와 맴핑
        
        DIAL            = 3,    // @brief ADC1와 맴핑  
        DIAL_EN         = 5     // @brief Dial ADC Net을 Open/Close하는 MOSFET 제어
    #endif
};

/**@brief  The initialize peripheral such as UART, Timer, GPIO, etc...
 *
 * @param[in]   None
 * @param[out]  None
 *
 * @retval  NRF_SUCCESS    
 * 
 */
ret_code_t peripheral_init(void);

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