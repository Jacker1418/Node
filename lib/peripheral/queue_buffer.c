#include "queue_buffer.h"

#ifdef DEBUG
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#endif

#define DEBUG_LOG_TAG                   "QUEUE"

static ret_code_t push_queue(struct Queue_Buffer* in_out_instance, uint8_t* in_data, uint32_t in_length);
static ret_code_t pop_queue(struct Queue_Buffer* in_out_instance, uint8_t* out_data, uint32_t* out_length);
static bool is_full_queue(struct Queue_Buffer* in_instance);
static bool is_empty_queue(struct Queue_Buffer* in_instance);
static uint8_t* next_point_buffer(struct Queue_Buffer* in_out_instance);
static uint8_t* get_point_buffer(struct Queue_Buffer* in_out_instance);

void init_Queue_Buffer(struct Queue_Buffer* out_instance)
{
    
    if(out_instance == NULL)
    {
        #ifdef DEBUG
        NRF_LOG_INFO("[%s] %s", DEBUG_LOG_TAG, "init_Queue_Buffer() : out_instance is NULL");
        #endif

        return;
    }

    out_instance->idxBuffer = 0;
    out_instance->idxLastReceive = 0;

    out_instance->idxFront = 0;
    out_instance->idxTail = 0;

    out_instance->push = push_queue;
    out_instance->pop = pop_queue;
    out_instance->is_full = is_full_queue;
    out_instance->is_empty = is_empty_queue;
    out_instance->next_point = next_point_buffer;
    out_instance->get_point = get_point_buffer;
}

static ret_code_t push_queue(struct Queue_Buffer* in_out_instance, uint8_t* in_data, uint32_t in_length)
{
    ret_code_t result = NRF_SUCCESS;

    if(in_out_instance == NULL)
    {
        #ifdef DEBUG
        NRF_LOG_INFO("[%s] %s", DEBUG_LOG_TAG, "push_queue() : in_out_instance is NULL");
        #endif

        return NRF_ERROR_INVALID_PARAM;
    }

    if(in_data == NULL)
    {
        #ifdef DEBUG
        NRF_LOG_INFO("[%s] %s", DEBUG_LOG_TAG, "push_queue() : in_data is NULL");
        #endif

        return NRF_ERROR_INVALID_PARAM;
    }

    uint8_t idxTail = in_out_instance->idxTail;
    uint8_t idxLastReceive = in_out_instance->idxLastReceive;

    if(!in_out_instance->is_full(in_out_instance))
    {
        in_out_instance->queue[idxTail].data = in_data[idxLastReceive];
        in_out_instance->queue[idxTail].length = in_length;

        idxLastReceive = (idxLastReceive + in_length) % SIZE_ROW_BUFFER;
        idxTail = (idxTail + 1) % SIZE_QUEUE;

        in_out_instance->idxTail = idxTail;
        in_out_instance->idxLastReceive = idxLastReceive;
    }
    else
    {
        #ifdef DEBUG
        NRF_LOG_INFO("[%s] %s", DEBUG_LOG_TAG, "push_queue() : Queue is full");
        #endif

        result = NRF_ERROR_RESOURCES;
    }

    return result;
}

static ret_code_t pop_queue(struct Queue_Buffer* in_out_instance, uint8_t* out_data, uint32_t* out_length)
{
    ret_code_t result = NRF_SUCCESS;

    if(in_out_instance == NULL)
    {
        #ifdef DEBUG
        NRF_LOG_INFO("[%s] %s", DEBUG_LOG_TAG, "pop_queue() : in_out_instance is NULL");
        #endif

        return NRF_ERROR_INVALID_PARAM;
    }

    if(out_data == NULL)
    {
        #ifdef DEBUG
        NRF_LOG_INFO("[%s] %s", DEBUG_LOG_TAG, "pop_queue() : out_data is NULL");
        #endif

        return NRF_ERROR_INVALID_PARAM;
    }

    if(out_length == NULL)
    {
        #ifdef DEBUG
        NRF_LOG_INFO("[%s] %s", DEBUG_LOG_TAG, "pop_queue() : out_length is NULL");
        #endif

        return NRF_ERROR_INVALID_PARAM;
    }

    uint8_t idxFront = in_out_instance->idxFront;

    if(!in_out_instance->is_empty(in_out_instance))
    {
        struct Buffer* temp = &in_out_instance->queue[idxFront];
        
        memcpy(out_data, temp->data, temp->length);
        *out_length = temp->length;

        idxFront = (idxFront + 1) % SIZE_QUEUE;

        in_out_instance->idxFront = idxFront;
    }
    else
    {
        #ifdef DEBUG
        NRF_LOG_INFO("[%s] %s", DEBUG_LOG_TAG, "pop_queue() : Queue is Empty");
        #endif

        result = NRF_ERROR_RESOURCES;
    }

    return result;
}

static bool is_full_queue(struct Queue_Buffer* in_instance)
{
    uint8_t front = in_instance->idxFront;
    uint8_t tail = in_instance->idxTail;

    return front == (tail + 1) % SIZE_QUEUE;
}

static bool is_empty_queue(struct Queue_Buffer* in_instance)
{
    uint8_t front = in_instance->idxFront;
    uint8_t tail = in_instance->idxTail;

    #ifdef DEBUG
    NRF_LOG_INFO("[%s] %d, %d", DEBUG_LOG_TAG, front, tail);
    #endif

    return front == tail;
}

static uint8_t* next_point_buffer(struct Queue_Buffer* in_out_instance)
{
    in_out_instance->idxBuffer = (in_out_instance->idxBuffer + 1) % SIZE_COL_BUFFER;

    return in_out_instance->buffer_pool[in_out_instance->idxBuffer];
} 

static uint8_t* get_point_buffer(struct Queue_Buffer* in_out_instance)
{
    return in_out_instance->buffer_pool[in_out_instance->idxBuffer];
} 