#include "drv_uart.h"

#include "queue_buffer.h"

#include "stdbool.h"

#ifdef DEBUG
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#endif

#define DEBUG_LOG_TAG                   "UART"

struct Queue_Buffer queue_receive;
struct Queue_Buffer queue_transmit;

void init_UARTE(struct drv_interface* out_instance)
{
    out_instance->open = open_UARTE;
    out_instance->write = write_UARTE;
    out_instance->read = read_UARTE;
    out_instance->ioctrl = ioctrl_UARTE;
    out_instance->isBusy = isBusy_UARTE;

    init_Queue_Buffer(&queue_receive);
    init_Queue_Buffer(&queue_transmit);

    out_instance->open();
}

void open_UARTE(void)
{
    uint8_t* buffer = NULL;

    NRF_UARTE0->TASKS_STOPRX = true;
    NRF_UARTE0->TASKS_STOPTX = true;
    NRF_UARTE0->ENABLE = false;

    NRF_UARTE0->PSEL.TXD = SERIAL_TX;
    NRF_UARTE0->PSEL.RXD = SERIAL_RX;

    buffer = queue_receive.get_point();

    NRF_UARTE0->RXD.PTR = buffer;
    NRF_UARTE0->RXD.MAXCNT = SIZE_BUFFER;
    NRF_UARTE0->SHORTS = UARTE_SHORTS_ENDRX_STARTRX_Enabled << UARTE_SHORTS_ENDRX_STARTRX_Pos;

    NRF_UARTE0->CONFIG = UARTE_CONFIG_PARITIY_DISABLE | UARTE_CONFIG_FLOW_CONTROL_DISABLE;

    NRF_UARTE0->BAUDRATE = UARTE_BAUDRATE_BAUDRATE_Baud921600;
    
    NRF_UARTE0->INTENCLR = 0xFFFFFFFF;

    NRF_UARTE0->INTENSET = (UARTE_INTENSET_TXSTARTED_Set << UARTE_INTENSET_TXSTARTED_Pos) | 
                           (UARTE_INTENSET_ENDTX_Set << UARTE_INTENSET_ENDTX_Pos) |
                           (UARTE_INTENSET_RXSTARTED_Set << UARTE_INTENSET_RXSTARTED_Pos) |
                           (UARTE_INTENSET_ENDRX_Set << UARTE_INTENSET_ENDRX_Pos) |
                           (UARTE_INTENSET_RXTO_Set << UARTE_INTENSET_RXTO_Pos) |
                           (UARTE_INTENSET_ERROR_Set << UARTE_INTENSET_ERROR_Pos);

    NVIC_SetPriority(UARTE0_UART0_IRQn, 2);
	NVIC_EnableIRQ(UARTE0_UART0_IRQn);

    NRF_UARTE0->ENABLE = UARTE_ENABLE_ENABLE_Enabled << UARTE_ENABLE_ENABLE_Pos;
	NRF_UARTE0->TASKS_STARTRX = 1;

    
}

uint32_t write_UARTE(uint32_t in_fd, uint8_t* in_data, uint16_t in_length)
{
    
}

ret_code_t read_UARTE(uint32_t in_fd, uint8_t* out_data, uint32_t* out_length)
{

}

ret_code_t ioctrl_UARTE(uint32_t in_fd, uint8_t in_option, uint8_t* out_result)
{
    ret_code_t result = NRF_SUCCESS;

    if(in_fd != UARTE0) return NRF_ERROR_INVALID_PARAM;

    switch(in_option)
    {
        case UARTE_IOCTRL_HAS_DATA:
            *out_result = true;
        break;
        default:
            result = NRF_ERROR_INVALID_PARAM;
    }

    return result;
}

bool isBusy_UARTE(void)
{

}

void UARTE0_UART0_IRQHandler(void)
{

    if(NRF_UARTE0->EVENTS_ERROR)
    {
        NRF_UARTE0->EVENTS_ERROR = 0;

        uint32_t error = NRF_UARTE0->ERRORSRC;

        if(error & UART_ERRORSRC_OVERRUN_Msk)
        {
            #ifdef DEBUG
            NRF_LOG_INFO("[%s] %s", DEBUG_LOG_TAG, "UART ERROR : Overrun");
            #endif
        }

        if(error & UART_ERRORSRC_PARITY_Msk)
        {
            #ifdef DEBUG
            NRF_LOG_INFO("[%s] %s", DEBUG_LOG_TAG, "UART ERROR : Parity");
            #endif
        }

        if(error & UART_ERRORSRC_FRAMING_Msk)
        {
            #ifdef DEBUG
            NRF_LOG_INFO("[%s] %s", DEBUG_LOG_TAG, "UART ERROR : Framing");
            #endif
        }

        if(error & UART_ERRORSRC_BREAK_Msk)
        {
            #ifdef DEBUG
            NRF_LOG_INFO("[%s] %s", DEBUG_LOG_TAG, "UART ERROR : Break");
            #endif
        }
    }

    if(NRF_UARTE0->EVENTS_RXSTARTED)
    {
        NRF_UARTE0->EVENTS_RXSTARTED = 0;
    
    }
    
    if(NRF_UARTE0->EVENTS_ENDRX)
    {
        NRF_UARTE0->EVENTS_ENDRX = 0;
    }

    if(NRF_UARTE0->EVENTS_RXTO)
    {

    }

    if(NRF_UARTE0->EVENTS_TXSTARTED)
    {

    }

    if(NRF_UARTE0->EVENTS_ENDTX)
    {

    }
}
