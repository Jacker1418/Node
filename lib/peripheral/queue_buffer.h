#ifndef __QUEUE_BUFFER_H__
#define __QUEUE_BUFFER_H__

#include "main.h"

#define SIZE_BUFFER               255
#define SIZE_QUEUE                5

struct Queue_Buffer
{
    uint8_t buffer[SIZE_QUEUE][SIZE_BUFFER];
    uint8_t* queue[SIZE_QUEUE]; 
    uint8_t idx_front;
    uint8_t idx_tail;

    ret_code_t (*push)(struct Queue_Buffer*, uint8_t*);
    ret_code_t (*pop)(struct Queue_Buffer*, uint8_t*);
    bool (*is_full)(struct Queue_Buffer*);
    bool (*is_empty)(struct Queue_Buffer*);
    uint8_t* (*get_point)(struct Queue_Buffer*);
};

#define SIZE_QUEUE  2048

void init_Queue_Buffer(struct Queue_Buffer* out_instance);

#endif
