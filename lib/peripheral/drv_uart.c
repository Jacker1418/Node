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

static void (*uarte_event_handler)(enum UARTE_EVENTS, void*);

static void open_UARTE(void*,uint8_t in_action);
static uint32_t write_UARTE(void* in_instance, void* in_data, uint16_t in_length);
static ret_code_t read_UARTE(void* in_instance, void* out_data, uint32_t* out_length);
static ret_code_t ioctrl_UARTE(void* in_instance, uint8_t in_option, uint8_t* out_result);
static bool isBusy_UARTE(void* in_instance);
static void close_UARTE(void*);

static void uarte_timeout_event_handler(NRF_TIMER_Type* in_timer);

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

    uarte_event_handler = in_uarte_event_handler;

    NRF_UARTE_Type *instance = (NRF_UARTE_Type *)out_instance->instance;

    instance->TASKS_STOPRX = true;
    instance->TASKS_STOPTX = true;
    instance->ENABLE = false;

    instance->PSEL.TXD = SERIAL_TX;
    instance->PSEL.RXD = SERIAL_RX;

    instance->RXD.PTR =  queue_receive.get_point(&queue_receive);
    instance->RXD.MAXCNT = SIZE_ROW_BUFFER;

    instance->CONFIG = UARTE_CONFIG_PARITIY_DISABLE | UARTE_CONFIG_FLOW_CONTROL_DISABLE;

    instance->BAUDRATE = UARTE_BAUDRATE_BAUDRATE_Baud115200;
    
    instance->INTENCLR = 0xFFFFFFFF;

    instance->INTENSET =   (UARTE_INTENSET_TXSTARTED_Set << UARTE_INTENSET_TXSTARTED_Pos) | 
                           (UARTE_INTENSET_ENDTX_Set << UARTE_INTENSET_ENDTX_Pos) |
                           (UARTE_INTENSET_RXSTARTED_Set << UARTE_INTENSET_RXSTARTED_Pos) |
                           (UARTE_INTENSET_ENDRX_Set << UARTE_INTENSET_ENDRX_Pos) |
                           (UARTE_INTENSET_RXTO_Set << UARTE_INTENSET_RXTO_Pos) |
                           (UARTE_INTENSET_ERROR_Set << UARTE_INTENSET_ERROR_Pos);

    /* Timer 설정 */
    init_TIMER(&insTIMER_3, NRF_TIMER3, TIMER_CONFIG_MODE_TIMER_1US, uarte_timeout_event_handler);  
    init_TIMER(&insTIMER_4, NRF_TIMER4, TIMER_CONFIG_MODE_COUNTER, NULL);

    // PPI 설정 의심
    // [fix] : 포인터 계산이므로 캐스팅 작업을 해야함
    NRF_PPI->CH[PPI_CH_UARTE_RX_WATCHDOG].EEP = (uint32_t)instance + (uint32_t)offsetof(NRF_UARTE_Type, EVENTS_RXDRDY);
    NRF_PPI->CH[PPI_CH_UARTE_RX_WATCHDOG].TEP = (uint32_t)insTIMER_3.instance + (uint32_t)offsetof(NRF_TIMER_Type, TASKS_START);
    NRF_PPI->FORK[PPI_CH_UARTE_RX_WATCHDOG].TEP = (uint32_t)insTIMER_3.instance + (uint32_t)offsetof(NRF_TIMER_Type, TASKS_CLEAR);

    NRF_PPI->CH[PPI_CH_UARTE_RX_TIMEOUT].EEP = (uint32_t)insTIMER_3.instance + (uint32_t)offsetof(NRF_TIMER_Type, EVENTS_COMPARE[0]);
    NRF_PPI->CH[PPI_CH_UARTE_RX_TIMEOUT].TEP = (uint32_t)insTIMER_3.instance + (uint32_t)offsetof(NRF_TIMER_Type, TASKS_STOP);
    NRF_PPI->FORK[PPI_CH_UARTE_RX_TIMEOUT].TEP = (uint32_t)insTIMER_4.instance + (uint32_t)offsetof(NRF_TIMER_Type, TASKS_CAPTURE[0]);

    NRF_PPI->CH[PPI_CH_UARTE_RX_COUNT].EEP = (uint32_t)instance + (uint32_t)offsetof(NRF_UARTE_Type, EVENTS_RXDRDY);
    NRF_PPI->CH[PPI_CH_UARTE_RX_COUNT].TEP = (uint32_t)insTIMER_4.instance + (uint32_t)offsetof(NRF_TIMER_Type, TASKS_COUNT);

    NRF_PPI->CH[PPI_CH_UARTE_RX_END].EEP = (uint32_t)instance + (uint32_t)offsetof(NRF_UARTE_Type, EVENTS_ENDRX);
    NRF_PPI->CH[PPI_CH_UARTE_RX_END].TEP = (uint32_t)insTIMER_4.instance + (uint32_t)offsetof(NRF_TIMER_Type, TASKS_CAPTURE[0]);

    NRF_PPI->CH[PPI_CH_UARTE_TX_END_START].EEP = (uint32_t)instance + (uint32_t)offsetof(NRF_UARTE_Type, EVENTS_ENDTX);
    NRF_PPI->CH[PPI_CH_UARTE_TX_END_START].TEP = (uint32_t)instance + (uint32_t)offsetof(NRF_UARTE_Type, TASKS_STARTTX);

    instance->ENABLE = UARTE_ENABLE_ENABLE_Enabled << UARTE_ENABLE_ENABLE_Pos;
}

static void open_UARTE(void* in_instance, uint8_t in_action)
{
    if(in_instance == NULL)
    {
        #ifdef DEBUG
        NRF_LOG_INFO("[%s] %s", DEBUG_LOG_TAG, "init_TIMER() invalid parameter : out_instance is NULL");
        #endif

        return;
    }

    struct drv_interface *insUARTE = (struct drv_interface *)in_instance;
    NRF_UARTE_Type* instance = (NRF_UARTE_Type *)insUARTE->instance;

    instance->EVENTS_ENDRX = 0;
    instance->EVENTS_RXSTARTED = 0;

    NRF_PPI->CHENSET =  PPI_CHENSET_CH0_Set << PPI_CH_UARTE_RX_WATCHDOG | 
                        PPI_CHENSET_CH0_Set << PPI_CH_UARTE_RX_TIMEOUT |
                        PPI_CHENSET_CH0_Set << PPI_CH_UARTE_RX_COUNT |
                        PPI_CHENSET_CH0_Set << PPI_CH_UARTE_RX_END;

    insTIMER_3.open(&insTIMER_3, TIMER_PREPARE);
    insTIMER_4.open(&insTIMER_4, TIMER_PREPARE);

    instance->SHORTS = UARTE_SHORTS_ENDRX_STARTRX_Enabled << UARTE_SHORTS_ENDRX_STARTRX_Pos;

    NVIC_SetPriority(UARTE0_UART0_IRQn, _PRIO_APP_HIGH);
    NVIC_ClearPendingIRQ(UARTE0_UART0_IRQn);
	NVIC_EnableIRQ(UARTE0_UART0_IRQn);

	instance->TASKS_STARTRX = 1;
}

static uint32_t write_UARTE(void* in_instance, void* in_data, uint16_t in_length)
{
    if(in_instance == NULL)
    {
        #ifdef DEBUG
        NRF_LOG_INFO("[%s] %s", DEBUG_LOG_TAG, "init_TIMER() invalid parameter : out_instance is NULL");
        #endif

        return;
    }

    struct drv_interface *insUARTE = (struct drv_interface *)in_instance;
    NRF_UARTE_Type* instance = insUARTE->instance;

    return 0;
}

static ret_code_t read_UARTE(void* in_instance, void* out_data, uint32_t* out_length)
{
    if(in_instance == NULL)
    {
        #ifdef DEBUG
        NRF_LOG_INFO("[%s] %s", DEBUG_LOG_TAG, "init_TIMER() invalid parameter : out_instance is NULL");
        #endif

        return NRF_ERROR_INVALID_PARAM;
    }

    struct drv_interface *insUARTE = (struct drv_interface *)in_instance;
    NRF_UARTE_Type* instance = insUARTE->instance;

    uint8_t *data = (uint8_t *)out_data;
    *out_length = 0;

    uint8_t tempData[SIZE_ROW_BUFFER];
    uint32_t tempLength = 0;

    while(queue_receive.pop(&queue_receive, tempData, &tempLength) == NRF_SUCCESS)
    {
        memcpy(&data[*out_length], tempData, tempLength);
        *out_length += tempLength;

        #ifdef DEBUG
        NRF_LOG_INFO("[%s] %08X, %d", DEBUG_LOG_TAG, tempData, tempLength);
        NRF_LOG_HEXDUMP_INFO(tempData, tempLength);
        #endif
    }

    return *out_length != 0 ? NRF_SUCCESS : NRF_ERROR_RESOURCES;
}

static ret_code_t ioctrl_UARTE(void* in_instance, uint8_t in_option, uint8_t* out_result)
{
    ret_code_t result = NRF_SUCCESS;

    if(in_instance == NULL)
    {
        #ifdef DEBUG
        NRF_LOG_INFO("[%s] %s", DEBUG_LOG_TAG, "init_TIMER() invalid parameter : out_instance is NULL");
        #endif

        return NRF_ERROR_INVALID_PARAM;
    }

    struct drv_interface *insUARTE = (struct drv_interface *)in_instance;
    NRF_UARTE_Type* instance = insUARTE->instance;

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
    bool result = false;

    if(in_instance == NULL)
    {
        #ifdef DEBUG
        NRF_LOG_INFO("[%s] %s", DEBUG_LOG_TAG, "init_TIMER() invalid parameter : out_instance is NULL");
        #endif

        return NRF_ERROR_INVALID_PARAM;
    }

    struct drv_interface *insUARTE = (struct drv_interface *)in_instance;
    NRF_UARTE_Type* instance = insUARTE->instance;

    return result;
}

static void close_UARTE(void* in_instance)
{
    if(in_instance == NULL)
    {
        #ifdef DEBUG
        NRF_LOG_INFO("[%s] %s", DEBUG_LOG_TAG, "init_TIMER() invalid parameter : out_instance is NULL");
        #endif

        return NRF_ERROR_INVALID_PARAM;
    }

    struct drv_interface *insUARTE = (struct drv_interface *)in_instance;
    NRF_UARTE_Type* instance = insUARTE->instance;
}

static void uarte_timeout_event_handler(NRF_TIMER_Type* in_timer)
{
    if(in_timer == NRF_TIMER3)
    {
        #ifdef DEBUG
        NRF_LOG_INFO("[%s] %s", DEBUG_LOG_TAG, "Timer3 occur");
        #endif

        uint8_t cntUARTE_RX = 0;
        uint32_t length = 0;

        insTIMER_4.read(&insTIMER_4, &cntUARTE_RX, &length);
        insTIMER_4.open(&insTIMER_4, TIMER_PREPARE);

        uint8_t* data = queue_receive.get_point(&queue_receive);

        ret_code_t result = queue_receive.push(&queue_receive, data, cntUARTE_RX);
        if(result != NRF_SUCCESS)
        {
            #ifdef DEBUG
            NRF_LOG_INFO("[%s] %s", DEBUG_LOG_TAG, "EVENTS_ENDRX : push() is error");
            #endif
        }
    }
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
    
        NRF_UARTE0->RXD.PTR = queue_receive.next_point(&queue_receive);
        NRF_UARTE0->RXD.MAXCNT = SIZE_ROW_BUFFER;
    }
    
    if(NRF_UARTE0->EVENTS_ENDRX)
    {
        NRF_UARTE0->EVENTS_ENDRX = 0;
        
        uint8_t cntUARTE_RX = 0;
        uint32_t length = 0;

        insTIMER_4.read(&insTIMER_4, &cntUARTE_RX, &length);

        insTIMER_4.open(&insTIMER_4, TIMER_PREPARE);

        uint8_t* data = queue_receive.get_point(&queue_receive);

        ret_code_t result = queue_receive.push(&queue_receive, data, cntUARTE_RX);
        if(result != NRF_SUCCESS)
        {
            #ifdef DEBUG
            NRF_LOG_INFO("[%s] %s", DEBUG_LOG_TAG, "EVENTS_ENDRX : push() is error");
            #endif
        }
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
