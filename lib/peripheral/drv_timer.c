#include "drv_timer.h"
#include "nrf_timer.h"

#ifdef DEBUG
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#endif

#define DEBUG_LOG_TAG                   "TIMER"

static void (*timer_event_handler[4])(NRF_TIMER_Type* in_timer) = {0,}; 

static void open_TIMER(void *in_instance,uint8_t in_action);
static ret_code_t read_TIMER(void* , void*, uint32_t*); 
static void close_TIMER(void *in_instance);

void init_TIMER(struct drv_interface* out_instance, NRF_TIMER_Type* in_timer, enum TIMER_CONFIG_MODE in_mode, void (*in_event_handler)(NRF_TIMER_Type* in_timer))
{
    if(out_instance == NULL)
    {
        #ifdef DEBUG
        NRF_LOG_INFO("[%s] %s", DEBUG_LOG_TAG, "init_TIMER() invalid parameter : out_instance is NULL");
        #endif

        return;
    }

    out_instance->busy = false;

    out_instance->open = open_TIMER;
    out_instance->read = read_TIMER;
    out_instance->write = NULL;
    out_instance->ioctrl = NULL;
    out_instance->close = close_TIMER;

    if(in_timer == NULL || in_timer == NRF_TIMER0)
    {
        #ifdef DEBUG
        NRF_LOG_INFO("[%s] %s", DEBUG_LOG_TAG, "init_TIMER() invalid parameter : in_timer");
        #endif

        return;
    }

    uint8_t index = 0;

    if(in_timer == NRF_TIMER1) index = TIMER1;
    else if(in_timer == NRF_TIMER2) index = TIMER2;
    else if(in_timer == NRF_TIMER3) index = TIMER3;
    else if(in_timer == NRF_TIMER4) index = TIMER4;
    else
    {
        #ifdef DEBUG
        NRF_LOG_INFO("[%s] %s", DEBUG_LOG_TAG, "init_TIMER() invalid parameter : in_timer");
        #endif

        return;
    }

    timer_event_handler[index] = in_event_handler;

    out_instance->instance = in_timer;
    NRF_TIMER_Type* instance = out_instance->instance; 

    instance->TASKS_STOP = 1;
    instance->TASKS_CLEAR = 1;

    switch(in_mode)
    {
        case TIMER_CONFIG_MODE_COUNTER:
            
            instance->MODE =  TIMER_MODE_MODE_Counter;
            instance->BITMODE =  NRF_TIMER_BIT_WIDTH_32;
            instance->PRESCALER = NRF_TIMER_FREQ_16MHz;
        break;

        case TIMER_CONFIG_MODE_TIMER_1US:

            instance->MODE =  TIMER_MODE_MODE_Timer;
            instance->BITMODE =  NRF_TIMER_BIT_WIDTH_32;
            instance->PRESCALER = NRF_TIMER_FREQ_1MHz;

            // 100us Timer
            instance->CC[0] = 100;
        break;

        case TIMER_CONFIG_MODE_TIMER_1MS:

            instance->MODE =  TIMER_MODE_MODE_Timer;
            instance->BITMODE =  NRF_TIMER_BIT_WIDTH_32;
            instance->PRESCALER = NRF_TIMER_FREQ_31250Hz;

            // 100ms Timer
            instance->CC[0] = (float)31250 * ((float)100 / 1000.0f);
        break;

        case TIMER_CONFIG_MODE_TIMER_1S:

            instance->MODE =  TIMER_MODE_MODE_Timer;
            instance->BITMODE =  NRF_TIMER_BIT_WIDTH_32;
            instance->PRESCALER = NRF_TIMER_FREQ_31250Hz;

            // 1s Timer
            instance->CC[0] = 31250 * 1;
        break;

        default:
            #ifdef DEBUG
            NRF_LOG_INFO("[%s] %s", DEBUG_LOG_TAG, "init_TIMER() invalid parameter : in_mode");
            #endif
        break;
    }
}

static void open_TIMER(void *in_instance, uint8_t in_action)
{
    if(in_instance == NULL)
    {
        #ifdef DEBUG
        NRF_LOG_INFO("[%s] %s", DEBUG_LOG_TAG, "open_TIMER() invalid parameter : instance is NULL");
        #endif

        return;
    }

    struct drv_interface *insTimer = (struct drv_interface *)in_instance;
    
    NRF_TIMER_Type* instance = insTimer->instance;

    instance->TASKS_STOP = 1;
    instance->TASKS_CLEAR = 1;

    instance->SHORTS = 1;
    instance->INTENSET	= (1<<16);

    NVIC_SetPriority(nrfx_get_irq_number(instance), _PRIO_APP_LOW);
    NVIC_EnableIRQ(nrfx_get_irq_number(instance));

    if(in_action == TIMER_ON)
    {
        instance->TASKS_START = 1;
    }
    
}

static ret_code_t read_TIMER(void* in_instance , void* out_buffer, uint32_t* out_length)
{
    ret_code_t result = NRF_SUCCESS;

    struct drv_interface *insTimer = (struct drv_interface *)in_instance;
    
    NRF_TIMER_Type* instance = insTimer->instance;

    uint8_t *data = (uint8_t *)out_buffer;

    instance->TASKS_CAPTURE[0] = 1;

    *data = instance->CC[0];

    *out_length = 1;

    return result;
}

static void close_TIMER(void *in_instance)
{
    if(in_instance == NULL)
    {
        #ifdef DEBUG
        NRF_LOG_INFO("[%s] %s", DEBUG_LOG_TAG, "close_TIMER() invalid parameter : instance is NULL");
        #endif

        return;
    }

    NRF_TIMER_Type* instance = (NRF_TIMER_Type *) in_instance;

    instance->TASKS_STOP = 1;
    instance->TASKS_CLEAR = 1;

    instance->SHORTS = 0;
    instance->INTENSET	 = instance->INTENSET & ~(1<<16);

    NVIC_DisableIRQ(nrfx_get_irq_number(instance));

}

void TIMER1_IRQHandler(void)
{
    if(NRF_TIMER1->EVENTS_COMPARE[0] == 1)
    {
        NRF_TIMER1->EVENTS_COMPARE[0] = 0;
        
        if(timer_event_handler[TIMER1] != NULL)
        {
            timer_event_handler[TIMER1](NRF_TIMER1);
        }
    }

    // #ifdef DEBUG
    // NRF_LOG_INFO("[%s] %s", DEBUG_LOG_TAG, "TIMER1_IRQHandler()");
    // #endif
}

void TIMER2_IRQHandler(void)
{
    if(NRF_TIMER2->EVENTS_COMPARE[0] == 1)
    {
        NRF_TIMER2->EVENTS_COMPARE[0] = 0;

        if(timer_event_handler[TIMER2] != NULL)
        {
            timer_event_handler[TIMER2](NRF_TIMER2);
        }
    }

    // #ifdef DEBUG
    // NRF_LOG_INFO("[%s] %s", DEBUG_LOG_TAG, "TIMER2_IRQHandler()");
    // #endif
}

void TIMER3_IRQHandler(void)
{
    if(NRF_TIMER3->EVENTS_COMPARE[0] == 1)
    {
        NRF_TIMER3->EVENTS_COMPARE[0] = 0;

        if(timer_event_handler[TIMER3] != NULL)
        {
            timer_event_handler[TIMER3](NRF_TIMER3);
        }
    }

    // #ifdef DEBUG
    // NRF_LOG_INFO("[%s] %s", DEBUG_LOG_TAG, "TIMER3_IRQHandler()");
    // #endif
}

void TIMER4_IRQHandler(void)
{
    if(NRF_TIMER4->EVENTS_COMPARE[0] == 1)
    {
        NRF_TIMER4->EVENTS_COMPARE[0] = 0;

        if(timer_event_handler[TIMER4] != NULL)
        {
            timer_event_handler[TIMER4](NRF_TIMER4);
        }
    }

    // #ifdef DEBUG
    // NRF_LOG_INFO("[%s] %s", DEBUG_LOG_TAG, "TIMER4_IRQHandler()");
    // #endif
}
