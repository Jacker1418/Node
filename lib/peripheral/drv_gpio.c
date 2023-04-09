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

void init_GPIO(struct drv_interface *out_instance)
{
    out_instance->open = open_GPIO;
    out_instance->write = write_GPIO;
    out_instance->read = read_GPIO;
    out_instance->ioctrl = ioctrl_GPIO;
    out_instance->isBusy = NULL;

    out_instance->open();

    uint8_t value = false;
    out_instance->write(LED1, &value, 1);
    value = true;
    out_instance->write(LED2, &value, 1);
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

#elif MBN52_NODE

    #ifdef DEBUG
    NRF_LOG_INFO("[%s] %s(%d)",DEBUG_LOG_TAG, "initGPIO() for NRF52_DK", result);
    #endif

#endif

}

uint32_t write_GPIO(uint32_t in_pos, uint8_t* in_data, uint32_t in_length)
{
    uint32_t result = in_length;

    nrf_gpio_pin_write(in_pos, *in_data);

    return result;
}

ret_code_t read_GPIO(uint32_t in_pos, uint8_t* out_data, uint32_t* out_length)
{
    ret_code_t result = NRF_SUCCESS;

    *out_data = nrf_gpio_pin_input_get(in_pos);
    *out_length = 1;

    return result;
}

ret_code_t ioctrl_GPIO(uint32_t in_pos, uint8_t in_option, uint8_t* out_result)
{
    ret_code_t result = NRF_SUCCESS;

    return result;
}