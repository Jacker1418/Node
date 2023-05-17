#ifndef __DRV_TIMER_H__
#define __DRV_TIMER_H__

#include "main.h"

enum TIMER_CONFIG_MODE
{
    TIMER_CONFIG_MODE_COUNTER,
    TIMER_CONFIG_MODE_TIMER_1US,
    TIMER_CONFIG_MODE_TIMER_1MS,
    TIMER_CONFIG_MODE_TIMER_1S
};

enum TIMER_INDEX
{
    TIMER1,
    TIMER2,
    TIMER3,
    TIMER4
};

enum TIMER_ACTION
{
    TIMER_ON,
    TIMER_PREPARE
};

void init_TIMER(struct drv_interface* out_instance, NRF_TIMER_Type* in_timer, enum TIMER_CONFIG_MODE in_mode, void (*in_event_handler)(NRF_TIMER_Type* in_timer));

#endif
