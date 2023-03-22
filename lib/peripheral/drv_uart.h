#ifndef __DRV_UART_H__
#define __DRV_UART_H__

#include "main.h"

void open_UARTE(void);
uint32_t write_UARTE(uint8_t* in_data, uint16_t in_length);
ret_code_t read_UARTE(uint8_t* out_data, uint32_t* out_length);
ret_code_t ioctrl_UARTE(uint8_t in_option);
bool isBusy_UARTE(void);

#endif
