#include "drv_uart.h"
#include "drv_timer.h"

#include "queue_buffer.h"

#include "stdbool.h"

#ifdef DEBUG
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#endif

#define DEBUG_LOG_TAG                   "UART"

static struct drv_interface insTIMER_3;
static struct drv_interface insTIMER_4;

static struct Queue_Buffer queue_receive;
static struct Queue_Buffer queue_transmit;

static void (*uarte_event_handler)(enum UARTE_EVENTS, void*);

static void open_UARTE(void*,uint8_t in_action);
static uint32_t write_UARTE(void* in_instance, void* in_data, uint16_t in_length);
static ret_code_t read_UARTE(void* in_instance, void* out_data, uint32_t* out_length);
static ret_code_t ioctrl_UARTE(void* in_instance, uint8_t in_option, uint8_t* out_result);
static bool isBusy_UARTE(void* in_instance);
static void close_UARTE(void*);

static void timeout_event_handler(NRF_TIMER_Type* in_timer)
{
    if(in_timer == NRF_TIMER1)
    {
        #ifdef DEBUG
        NRF_LOG_INFO("[%s] %s", DEBUG_LOG_TAG, "Timer1 occur");
        #endif
    }

    if(in_timer == NRF_TIMER2)
    {
        #ifdef DEBUG
        NRF_LOG_INFO("[%s] %s", DEBUG_LOG_TAG, "Timer2 occur");
        #endif
    }
}

void init_UARTE(struct drv_interface* out_instance, void (*in_uarte_event_handler)(enum UARTE_EVENTS in_evnet, void* in_context))
{
    if(out_instance == NULL)
    {
        #ifdef DEBUG
        NRF_LOG_INFO("[%s] %s", DEBUG_LOG_TAG, "init_TIMER() invalid parameter : out_instance is NULL");
        #endif

        return;
    }

    out_instance->busy = false;

    out_instance->instance = NRF_UARTE0;

    out_instance->open = open_UARTE;
    out_instance->write = write_UARTE;
    out_instance->read = read_UARTE;
    out_instance->ioctrl = ioctrl_UARTE;
    out_instance->isBusy = isBusy_UARTE;
    out_instance->close = close_UARTE;

    init_Queue_Buffer(&queue_receive);
    init_Queue_Buffer(&queue_transmit);

    uarte_event_handler = in_uarte_event_handler;

    NRF_UARTE_Type *instance = (NRF_UARTE_Type *)out_instance->instance;

    instance->TASKS_STOPRX = true;
    instance->TASKS_STOPTX = true;
    instance->ENABLE = false;

    instance->PSEL.TXD = SERIAL_TX;
    instance->PSEL.RXD = SERIAL_RX;

    instance->RXD.PTR =  queue_receive.get_point(&queue_receive);
    instance->RXD.MAXCNT = SIZE_BUFFER;

    instance->CONFIG = UARTE_CONFIG_PARITIY_DISABLE | UARTE_CONFIG_FLOW_CONTROL_DISABLE;

    instance->BAUDRATE = UARTE_BAUDRATE_BAUDRATE_Baud921600;
    
    instance->INTENCLR = 0xFFFFFFFF;

    instance->INTENSET =   (UARTE_INTENSET_TXSTARTED_Set << UARTE_INTENSET_TXSTARTED_Pos) | 
                           (UARTE_INTENSET_ENDTX_Set << UARTE_INTENSET_ENDTX_Pos) |
                           (UARTE_INTENSET_RXSTARTED_Set << UARTE_INTENSET_RXSTARTED_Pos) |
                           (UARTE_INTENSET_ENDRX_Set << UARTE_INTENSET_ENDRX_Pos) |
                           (UARTE_INTENSET_RXTO_Set << UARTE_INTENSET_RXTO_Pos) |
                           (UARTE_INTENSET_ERROR_Set << UARTE_INTENSET_ERROR_Pos);

    /* Timer 설정 */
    init_TIMER(&insTIMER_3, NRF_TIMER3, TIMER_CONFIG_MODE_TIMER_1US, timeout_event_handler);  
    init_TIMER(&insTIMER_4, NRF_TIMER4, TIMER_CONFIG_MODE_COUNTER, NULL);

    NRF_PPI->CH[PPI_CH_UARTE_RX_WATCHDOG].EEP = instance + offsetof(NRF_UARTE_Type, EVENTS_RXDRDY);
    NRF_PPI->CH[PPI_CH_UARTE_RX_WATCHDOG].TEP = insTIMER_3.instance + offsetof(NRF_TIMER_Type, TASKS_START);
    NRF_PPI->FORK[PPI_CH_UARTE_RX_WATCHDOG].TEP = insTIMER_3.instance + offsetof(NRF_TIMER_Type, TASKS_CLEAR);

    NRF_PPI->CH[PPI_CH_UARTE_RX_TIMEOUT].EEP = insTIMER_3.instance + offsetof(NRF_TIMER_Type, EVENTS_COMPARE[0]);
    NRF_PPI->CH[PPI_CH_UARTE_RX_TIMEOUT].TEP = insTIMER_3.instance + offsetof(NRF_TIMER_Type, TASKS_STOP);
    NRF_PPI->FORK[PPI_CH_UARTE_RX_TIMEOUT].TEP = insTIMER_4.instance + offsetof(NRF_TIMER_Type, TASKS_CAPTURE[0]);

    NRF_PPI->CH[PPI_CH_UARTE_RX_COUNT].EEP = instance + offsetof(NRF_UARTE_Type, EVENTS_RXDRDY);
    NRF_PPI->CH[PPI_CH_UARTE_RX_COUNT].TEP = insTIMER_4.instance + offsetof(NRF_TIMER_Type, TASKS_COUNT);

    NRF_PPI->CH[PPI_CH_UARTE_RX_END].EEP = instance + offsetof(NRF_UARTE_Type, EVENTS_ENDRX);
    NRF_PPI->CH[PPI_CH_UARTE_RX_END].TEP = insTIMER_4.instance + offsetof(NRF_TIMER_Type, TASKS_CAPTURE[0]);

    NRF_PPI->CH[PPI_CH_UARTE_TX_END_START].EEP = instance + offsetof(NRF_UARTE_Type, EVENTS_ENDTX);
    NRF_PPI->CH[PPI_CH_UARTE_TX_END_START].TEP = instance + offsetof(NRF_UARTE_Type, TASKS_STARTTX);
}

static void open_UARTE(void* in_instance, uint8_t in_action)
{
    NRF_UARTE_Type* instance = (NRF_UARTE_Type *)in_instance;

    instance->EVENTS_ENDRX = 0;
    instance->EVENTS_RXSTARTED = 0;

    instance->SHORTS = UARTE_SHORTS_ENDRX_STARTRX_Enabled << UARTE_SHORTS_ENDRX_STARTRX_Pos;

    instance->ENABLE = UARTE_ENABLE_ENABLE_Enabled << UARTE_ENABLE_ENABLE_Pos;

    NRF_PPI->CHENSET = PPI_CHENSET_CH0_Set << PPI_CH_UARTE_RX_WATCHDOG;
    NRF_PPI->CHENSET = PPI_CHENSET_CH0_Set << PPI_CH_UARTE_RX_TIMEOUT;

    insTIMER_4.open(&insTIMER_4, TIMER_PREPARE);

    NVIC_SetPriority(UARTE0_UART0_IRQn, _PRIO_APP_HIGH);
	NVIC_EnableIRQ(UARTE0_UART0_IRQn);

	instance->TASKS_STARTRX = 1;
}

static uint32_t write_UARTE(void* in_instance, void* in_data, uint16_t in_length)
{
    if(in_instance == NULL) return NRF_ERROR_INVALID_PARAM;

    NRF_UARTE_Type* instance = (NRF_UARTE_Type *)in_instance;
}

static ret_code_t read_UARTE(void* in_instance, void* out_data, uint32_t* out_length)
{
    if(in_instance == NULL) return NRF_ERROR_INVALID_PARAM;
    
    NRF_UARTE_Type* instance = (NRF_UARTE_Type *)in_instance;
}

static ret_code_t ioctrl_UARTE(void* in_instance, uint8_t in_option, uint8_t* out_result)
{
    ret_code_t result = NRF_SUCCESS;

    if(in_instance == NULL) return NRF_ERROR_INVALID_PARAM;

    NRF_UARTE_Type* instance = (NRF_UARTE_Type *)in_instance;

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

static bool isBusy_UARTE(void* in_instance)
{
    if(in_instance == NULL) return NRF_ERROR_INVALID_PARAM;

    NRF_UARTE_Type* instance = (NRF_UARTE_Type *)in_instance;
}

static void close_UARTE(void* in_instance)
{
    if(in_instance == NULL) return NRF_ERROR_INVALID_PARAM;

    NRF_UARTE_Type* instance = (NRF_UARTE_Type *)in_instance;
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
    
        NRF_UARTE0->RXD.PTR = queue_receive.get_point(&queue_receive);
    }
    
    if(NRF_UARTE0->EVENTS_ENDRX)
    {
        NRF_UARTE0->EVENTS_ENDRX = 0;
        
        ret_code_t result = queue_receive.push(&queue_receive, NRF_UARTE0->RXD.PTR);
        if(result != NRF_SUCCESS)
        {
            #ifdef DEBUG
            NRF_LOG_INFO("[%s] %s", DEBUG_LOG_TAG, "EVENTS_ENDRX : push() is error");
            #endif
        }

        uint8_t cnt_uarte_rx = 0;
        uint32_t length = 0;
        insTIMER_4.read(&insTIMER_4, &cnt_uarte_rx, &length );

        #ifdef DEBUG
        NRF_LOG_INFO("[%s] %s", DEBUG_LOG_TAG, "EVENTS_ENDRX : %d", cnt_uarte_rx );
        #endif
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
