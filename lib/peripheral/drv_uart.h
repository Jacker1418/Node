#ifndef __DRV_UART_H__
#define __DRV_UART_H__

#include "main.h"

#define UARTE0                              1

#define UARTE_CONFIG_PARITIY_DISABLE        UART_CONFIG_PARITY_Excluded << UART_CONFIG_PARITY_Pos
#define UARTE_CONFIG_PARITIY_ENABLE         UART_CONFIG_PARITY_Included << UART_CONFIG_PARITY_Pos
#define UARTE_CONFIG_FLOW_CONTROL_DISABLE   UART_CONFIG_HWFC_Disabled << UART_CONFIG_HWFC_Pos
#define UARTE_CONFIG_FLOW_CONTROL_ENABLE    UART_CONFIG_HWFC_Enabled << UART_CONFIG_HWFC_Pos

#define UARTE_IOCTRL_HAS_DATA               0x01

void init_UARTE(struct drv_interface* out_instance);

void open_UARTE(void);
uint32_t write_UARTE(uint32_t in_fd, uint8_t* in_data, uint16_t in_length);
ret_code_t read_UARTE(uint32_t in_fd, uint8_t* out_data, uint32_t* out_length);
ret_code_t ioctrl_UARTE(uint32_t in_fd, uint8_t in_option, uint8_t* out_result);
bool isBusy_UARTE(void);

#endif
