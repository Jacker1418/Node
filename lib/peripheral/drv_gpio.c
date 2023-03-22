#include "drv_gpio.h"

#include "nrf_gpio.h"
#include "nrf_drv_gpiote.h"

#ifdef DEBUG
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#endif

#define DEBUG_LOG_TAG "GPIO"

void event_gpio_handler(nrf_drv_gpiote_pin_t in_pin, nrf_gpiote_polarity_t in_action)
{	
#ifdef NRF52_DK

    if(in_pin == BTN1 && in_action == NRF_GPIOTE_POLARITY_HITOLO)
    {
        #ifdef DEBUG
        NRF_LOG_INFO("[%s] %d %d", DEBUG_LOG_TAG, in_pin, in_action);
        #endif
    }
#elif MBN52_NODE
    if(in_pin == PHOLD_SWITCH && in_action == NRF_GPIOTE_POLARITY_HITOLO)
    {
        #ifdef DEBUG
        NRF_LOG_INFO("[%s] %d %d", DEBUG_LOG_TAG, in_pin, in_action);
        #endif
    }
#endif
}

void open_GPIO(void)
{
    ret_code_t result = nrf_drv_gpiote_init();
    if(result != NRF_SUCCESS)
    {
        #ifdef DEBUG
        NRF_LOG_INFO("[%s] %s(%d)",DEBUG_LOG_TAG, "initGPIO()->nrf_drv_gpiote_init()", result);
        #endif
        return;
    }

#ifdef NRF52_DK

    nrf_drv_gpiote_in_config_t input = NRFX_GPIOTE_CONFIG_IN_SENSE_HITOLO(true);
    input.pull = NRF_GPIO_PIN_PULLUP;

    result = nrf_drv_gpiote_in_init(BTN1, &input, event_gpio_handler);
    if(result != NRF_SUCCESS)
    {
        #ifdef DEBUG
        NRF_LOG_INFO("[%s] %s(%d)",DEBUG_LOG_TAG, "initGPIO()->nrf_drv_gpiote_in_init()", result);
        #endif
        return;
    }
    nrf_drv_gpiote_in_event_enable(BTN1, true); 

    nrf_gpio_cfg_output(LED1);
    nrf_gpio_cfg_output(LED2);

    nrf_gpio_pin_clear(LED1);
    nrf_gpio_pin_set(LED2);
#elif MBN52_NODE

    #ifdef DEBUG
    NRF_LOG_INFO("[%s] %s(%d)",DEBUG_LOG_TAG, "initGPIO() for NRF52_DK", result);
    #endif

#endif

}

uint32_t write_GPIO(uint8_t* in_data, uint16_t in_length)
{

}

ret_code_t read_GPIO(uint8_t* out_data, uint32_t* out_length)
{

}

ret_code_t ioctrl_GPIO(uint8_t in_option)
{

}