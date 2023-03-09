#include "initGPIO.h"

static uint8_t gpiote_channel[8] = {0,};

__STATIC_INLINE void config_gpio_output(uint32_t in_pin, nrf_gpio_pin_pull_t in_pushpull, nrf_gpio_pin_drive_t in_drive);
__STATIC_INLINE void config_gpio_input(uint32_t in_pin, nrf_gpio_pin_pull_t in_pushpull, nrf_gpio_pin_drive_t in_drive, nrf_gpio_pin_sense_t in_sense);

__STATIC_INLINE void clear_gpiote_event(uint32_t in_event);
__STATIC_INLINE void enable_gpiote_event(uint32_t in_interrupt);
__STATIC_INLINE void disable_gpiote_event(uint32_t in_interrupt);

__STATIC_INLINE uint8_t check_gpiote_channel(uint8_t in_channel);

ret_code_t openGPIO(void)
{
    ret_code_t result = NRF_SUCCESS;

    #ifdef NRF52_DK

    #elif

    #endif

    return result;
}

__INLINE uint32_t readGPIO(uint32_t in_pin)
{
    NRF_GPIO_Type * reg = NRF_P0;

    return (reg->IN >> in_pin) & 1UL;
}

__INLINE void writeGPIO(uint32_t in_pin, bool in_value)
{
    NRF_GPIO_Type * reg = NRF_P0;

    if(in_value)
    {
        reg->OUTCLR = 1UL << in_pin;
    }
    else
    {
        reg->OUTSET = 1UL << in_pin;
    }
}

ret_code_t closeGPIO(void)
{
    ret_code_t result = NRF_SUCCESS;

    return result;
}

__STATIC_INLINE void config_gpio_output(uint32_t in_pin, nrf_gpio_pin_pull_t in_pushpull, nrf_gpio_pin_drive_t in_drive)
{
    NRF_GPIO_Type* reg_gpio = NRF_P0;

    reg_gpio->PIN_CNF[in_pin] = ((uint32_t)in_pushpull << GPIO_PIN_CNF_PULL_Pos)
                                | ((uint32_t)NRF_GPIO_PIN_DIR_OUTPUT << GPIO_PIN_CNF_DIR_Pos)
                                | ((uint32_t)in_drive << GPIO_PIN_CNF_DRIVE_Pos)
                                | ((uint32_t)NRF_GPIO_PIN_INPUT_DISCONNECT << GPIO_PIN_CNF_INPUT_Pos)
                                | ((uint32_t)NRF_GPIO_PIN_NOSENSE << GPIO_PIN_CNF_SENSE_Pos);
    
}

__STATIC_INLINE void config_gpio_input(uint32_t in_pin, nrf_gpio_pin_pull_t in_pushpull, nrf_gpio_pin_drive_t in_drive, nrf_gpio_pin_sense_t in_sense)
{
    NRF_GPIO_Type* reg_gpio = NRF_P0;

    reg_gpio->PIN_CNF[in_pin] = ((uint32_t)in_pushpull << GPIO_PIN_CNF_PULL_Pos)
                                | ((uint32_t)NRF_GPIO_PIN_DIR_INPUT << GPIO_PIN_CNF_DIR_Pos)
                                | ((uint32_t)in_drive << GPIO_PIN_CNF_DRIVE_Pos)
                                | ((uint32_t)NRF_GPIO_PIN_INPUT_DISCONNECT << GPIO_PIN_CNF_INPUT_Pos)
                                | ((uint32_t)NRF_GPIO_PIN_NOSENSE << GPIO_PIN_CNF_SENSE_Pos);

    if(in_sense != GPIOTE_CONFIG_POLARITY_None)
    {
        NVIC_SetPriority(GPIOTE_IRQn, NRFX_GPIOTE_CONFIG_IRQ_PRIORITY);
        NVIC_EnableIRQ(GPIOTE_IRQn);

        clear_gpiote_event(NRF_GPIOTE_EVENTS_PORT);
        enable_gpiote_event(GPIOTE_INTENSET_PORT_Msk);

        NRF_GPIOTE->CONFIG[0] &= ~(GPIOTE_CONFIG_PORT_PIN_Msk | GPIOTE_CONFIG_POLARITY_Msk);
        NRF_GPIOTE->CONFIG[0] |=    ((in_pin << GPIOTE_CONFIG_PSEL_Pos) & GPIOTE_CONFIG_PORT_PIN_Msk) |
                                    ((in_sense << GPIOTE_CONFIG_POLARITY_Pos) & GPIOTE_CONFIG_POLARITY_Msk);
    }
    else
    {

    }
}

__STATIC_INLINE void clear_gpiote_event(uint32_t in_event)
{
    *(uint32_t *)(NRF_GPIOTE + in_event) = 0;
}

__STATIC_INLINE void enable_gpiote_event(uint32_t in_interrupt)
{
    NRF_GPIOTE->INTENSET = in_interrupt;
}

__STATIC_INLINE void disable_gpiote_event(uint32_t in_interrupt)
{
    NRF_GPIOTE->INTENCLR = in_interrupt;
}

__STATIC_INLINE uint8_t check_gpiote_channel(uint8_t in_channel)
{
    uint8_t result = 0;

    for(uint8_t index = 0; index < 8; index++)
    {
        if(gpiote_channel[index] == 0)
        {
            result = index;
        }
    }
}

void GPIOTE_IRQHandler(void)
{

}