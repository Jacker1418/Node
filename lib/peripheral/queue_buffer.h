#ifndef __QUEUE_BUFFER_H__
#define __QUEUE_BUFFER_H__

#include "main.h"

#define SIZE_BUFFER               255
#define SIZE_QUEUE                10

struct Buffer 
{
    uint8_t* p_data;
    uint8_t length;
};

struct Queue_Buffer
{
    struct Buffer buffer_pool[SIZE_QUEUE][SIZE_BUFFER];
    struct Buffer* currentBuffer;
    struct Buffer* queue[SIZE_QUEUE]; 

    uint8_t idx_front;
    uint8_t idx_tail;

    ret_code_t (*push)(struct Queue_Buffer*, struct Buffer*);
    ret_code_t (*pop)(struct Queue_Buffer*, struct Buffer*);
    bool (*is_full)(struct Queue_Buffer*);
    bool (*is_empty)(struct Queue_Buffer*);
    struct Buffer* (*get_point)(struct Queue_Buffer*);
};

void init_Queue_Buffer(struct Queue_Buffer* out_instance);

#endif
