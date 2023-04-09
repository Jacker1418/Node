#ifndef __DRV_GPIO_H__
#define __DRV_GPIO_H__

#include "main.h"

/* Define functions [Form] */
/**@brief  GPIO 설정 및 Event Hanlder 등록
 *
 * @param None 
 *
 * @retval None    
 * 
 * @warning
 * 
 * @note
 */
void init_GPIO(struct drv_interface *out_instace);
void open_GPIO(void);
uint32_t write_GPIO(uint32_t in_pos, uint8_t* in_data, uint32_t in_length);
ret_code_t read_GPIO(uint32_t in_pos, uint8_t* out_data, uint32_t* out_length);
ret_code_t ioctrl_GPIO(uint32_t in_pos, uint8_t in_option, uint8_t* out_result);

#endif 