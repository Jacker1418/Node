#include "queue_buffer.h"

#ifdef DEBUG
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#endif

#define DEBUG_LOG_TAG                   "QUEUE"

static ret_code_t push_queue(struct Queue_Buffer* in_out_instance, struct Buffer* in_buffer);
static ret_code_t pop_queue(struct Queue_Buffer* in_out_instance, struct Buffer* out_buffer);
static bool is_full_queue(struct Queue_Buffer* in_instance);
static bool is_empty_queue(struct Queue_Buffer* in_instance);
static struct Buffer* get_point_buffer(struct Queue_Buffer* in_instance);

void init_Queue_Buffer(struct Queue_Buffer* out_instance)
{
    if(out_instance == NULL)
    {
        #ifdef DEBUG
        NRF_LOG_INFO("[%s] %s", DEBUG_LOG_TAG, "init_Queue_Buffer() : out_instance is NULL");
        #endif
    }

    for(uint8_t index = 0; index < SIZE_QUEUE; index++)
    {
        out_instance->queue[index] = NULL;
    }
    
    out_instance->currentBuffer = NULL;

    out_instance->idx_front = 0;
    out_instance->idx_tail = 0;

    out_instance->push = push_queue;
    out_instance->pop = pop_queue;
    out_instance->is_full = is_full_queue;
    out_instance->is_empty = is_empty_queue;
    out_instance->get_point = get_point_buffer;
}

static ret_code_t push_queue(struct Queue_Buffer* in_out_instance, struct Buffer* in_buffer)
{
    ret_code_t result = NRF_SUCCESS;

    uint8_t* idx_tail = &(in_out_instance->idx_tail);

    if(!in_out_instance->is_full(in_out_instance))
    {
        in_out_instance->queue[*idx_tail] = in_buffer;
        *idx_tail = (*idx_tail + 1) % SIZE_QUEUE;

        in_out_instance->currentBuffer = in_out_instance->queue[*idx_tail];
    }
    else
    {
        result = NRF_ERROR_RESOURCES;

        #ifdef DEBUG
        NRF_LOG_INFO("[%s] %s", DEBUG_LOG_TAG, "push_queue() : Queue is full");
        #endif
    }

    return result;
}

static ret_code_t pop_queue(struct Queue_Buffer* in_out_instance, struct Buffer* out_buffer)
{
    ret_code_t result = NRF_SUCCESS;

    uint8_t* idx_front = &(in_out_instance->idx_front);

    if(!in_out_instance->is_empty(in_out_instance))
    {
        out_buffer = in_out_instance->queue[*idx_front];
        *idx_front = (*idx_front + 1) % SIZE_QUEUE;

        in_out_instance->currentBuffer = in_out_instance->queue[*idx_front];
    }
    else
    {
        result = NRF_ERROR_RESOURCES;

        #ifdef DEBUG
        NRF_LOG_INFO("[%s] %s", DEBUG_LOG_TAG, "push_queue() : Queue is full");
        #endif
    }

    return result;
}

static bool is_full_queue(struct Queue_Buffer* in_instance)
{
    uint8_t front = in_instance->idx_front;
    uint8_t tail = in_instance->idx_tail;

    return tail == (front + 1) % SIZE_QUEUE;
}

static bool is_empty_queue(struct Queue_Buffer* in_instance)
{
    uint8_t front = in_instance->idx_front;
    uint8_t tail = in_instance->idx_tail;

    return front == tail;
}

static struct Buffer* get_point_buffer(struct Queue_Buffer* in_instance)
{
    return in_instance->currentBuffer;
} 