#ifndef __MAIN_H__
#define __MAIN_H__

#define DEBUG
#define NRF52_DK

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "nordic_common.h"
#include "nrf.h"
#include "nrf52.h"

#include "sdk_errors.h"

#ifdef NRF52_DK
enum PIN_MAP
{
    SERIAL_RX       = 8, 
    SERIAL_TX       = 6,

    BTN1            = 13,
    BTN2            = 14,
    BTN3            = 15,
    BTN4            = 16,

    LED1            = 17,
    LED2            = 18,
    LED3            = 19,
    LED4            = 20
};
#elif MBN52_NODE
enum PIN_MAP
{
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
};
#endif

typedef struct drv_interface
{
    void (*open)(void);
    uint32_t (*write)(uint32_t, uint8_t*, uint32_t);
    ret_code_t (*read)(uint32_t, uint8_t*, uint32_t*);
    ret_code_t (*ioctrl)(uint32_t, uint8_t, uint8_t*);
    bool (*isBusy)(void);
};

#endif
