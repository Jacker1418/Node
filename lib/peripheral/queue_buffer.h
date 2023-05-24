#ifndef __QUEUE_BUFFER_H__
#define __QUEUE_BUFFER_H__

#include "main.h"

#define SIZE_ROW_BUFFER                 255
#define SIZE_COL_BUFFER                 5
#define SIZE_QUEUE                      10
#define IDX_QUEUE_NONE                  11

struct Buffer 
{
    uint8_t *data;
    uint8_t length;
};

struct Queue_Buffer
{
    uint8_t buffer_pool[SIZE_COL_BUFFER][SIZE_ROW_BUFFER];
    struct Buffer queue[SIZE_QUEUE]; 

    uint8_t idxBuffer;
    uint8_t idxLastReceive;

    uint8_t idxFront;
    uint8_t idxTail;

    ret_code_t (*push)(struct Queue_Buffer*, uint8_t*, uint32_t);
    ret_code_t (*pop)(struct Queue_Buffer*, uint8_t*, uint32_t*);
    bool (*is_full)(struct Queue_Buffer*);
    bool (*is_empty)(struct Queue_Buffer*);
    uint8_t* (*next_point)(struct Queue_Buffer*);
    uint8_t* (*get_point)(struct Queue_Buffer*);
};

void init_Queue_Buffer(struct Queue_Buffer* out_instance);

#endif
