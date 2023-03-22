#ifndef __DRV_INTERFACE_H__
#define __DRV_INTERFACE_H__

#include "main.h"

typedef struct drv_interface
{
    void (*open)(void);
    uint32_t (*write)(uint8_t* in_data, uint32_t in_length);
    ret_code_t (*read)(uint8_t* out_data, uint32_t* out_length);
    ret_code_t (*ioctrl)(uint8_t in_option);
    bool (*isBusy)(void);
};

#endif
