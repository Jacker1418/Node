#ifndef __DRV_UART_H__
#define __DRV_UART_H__

#include "main.h"

#define UARTE0                              1

#define UARTE_CONFIG_PARITIY_DISABLE        UART_CONFIG_PARITY_Excluded << UART_CONFIG_PARITY_Pos
#define UARTE_CONFIG_PARITIY_ENABLE         UART_CONFIG_PARITY_Included << UART_CONFIG_PARITY_Pos
#define UARTE_CONFIG_FLOW_CONTROL_DISABLE   UART_CONFIG_HWFC_Disabled << UART_CONFIG_HWFC_Pos
#define UARTE_CONFIG_FLOW_CONTROL_ENABLE    UART_CONFIG_HWFC_Enabled << UART_CONFIG_HWFC_Pos

#define UARTE_IOCTRL_HAS_DATA               0x01

enum UARTE_EVENTS
{
    UARTE_RX_TIMEOUT,
    UARTE_RX_ENDRX
};

/**@brief UARTE Configuration

 * @retval  None             
 * 
 * @warning
 * 
 * @note UART Configuration 및 Timer, PPI, Queue에 대한 설정을 시작
 */
void init_UARTE(struct drv_interface* out_instance, void (*uarte_event_handler)(enum UARTE_EVENTS in_evnet, void* in_context));


#endif
