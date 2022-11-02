/*
 * @Author: HoGC
 * @Date: 2022-03-16 19:59:04
 * @Last Modified time: 2022-03-16 19:59:04
 */

#include "rbuffer.h"

#include <string.h>

#define MIN(a, b) ((a)<(b)?(a):(b))

typedef struct {
    uint32_t init_size;
    uint32_t curr_size;
    uint32_t read_pos;
    uint32_t write_pos;
    void *buffer;
    bool suspend;
    bool need_free;
}rbuffer_t;

rbuffer_handle_t rbuffer_create(uint32_t size){
    rbuffer_t *rbuffer = NULL;

    if(size == 0){
        return NULL;
    }

    rbuffer = (rbuffer_t *)RBUFFER_MALLOC(sizeof(rbuffer_t));
    if(!rbuffer){
        return NULL;
    }

    rbuffer->buffer = RBUFFER_MALLOC(size);

    if(!rbuffer->buffer){
        RBUFFER_FREE(rbuffer);
        return NULL;
    }

    memset(rbuffer->buffer, 0, size);

    rbuffer->init_size = size;
    rbuffer->curr_size = 0;
    rbuffer->read_pos = 0;
    rbuffer->write_pos = 0;
    rbuffer->suspend = false;
    rbuffer->need_free = true;

    return rbuffer;
}

rbuffer_handle_t rbuffer_static_create(void *buffer, uint32_t size){
    rbuffer_t *rbuffer = NULL;

    if(buffer == NULL || size == 0){
        return NULL;
    }

    rbuffer = (rbuffer_t *)RBUFFER_MALLOC(sizeof(rbuffer_t));
    if(!rbuffer){
        return NULL;
    }

    rbuffer->buffer = buffer;

    rbuffer->init_size = size;
    rbuffer->curr_size = 0;
    rbuffer->read_pos = 0;
    rbuffer->write_pos = 0;
    rbuffer->suspend = false;
    rbuffer->need_free = false;

    return rbuffer;
}

bool rbuffer_delete(rbuffer_handle_t handle){
    rbuffer_t *rbuffer = NULL;

    rbuffer = (rbuffer_t *)handle;
    
    if(!rbuffer){
        return false;
    }

    if(rbuffer->buffer){
        RBUFFER_FREE(rbuffer->buffer);
    }

    RBUFFER_FREE(rbuffer);

    return true;
}

uint32_t rbuffer_push(rbuffer_handle_t handle, void *buffer, uint32_t size, bool cover){
    uint32_t move_size = 0;
    rbuffer_t *rbuffer = NULL;

    rbuffer = (rbuffer_t *)handle;
    
    if(!rbuffer){
        return 0;
    }

    if(rbuffer->suspend || buffer == NULL || size == 0){
        return 0;
    }

    if(!cover){
        size = MIN(size, rbuffer->init_size - rbuffer->curr_size);
    }

    move_size = MIN(size, rbuffer->init_size - rbuffer->write_pos);

    memcpy((uint8_t *)rbuffer->buffer + rbuffer->write_pos, buffer, move_size);

    if(size - move_size > 0){
        rbuffer->write_pos = 0;
        memcpy(rbuffer->buffer, (uint8_t *)buffer + move_size, size - move_size);
        move_size = size - move_size;
    }
    
    rbuffer->write_pos += move_size;
    if(rbuffer->curr_size + size > rbuffer->init_size){
        rbuffer->curr_size = rbuffer->init_size;
        rbuffer->read_pos = rbuffer->write_pos;
    }else{
        rbuffer->curr_size += size;
    }

    return size;
}

uint32_t rbuffer_pop(rbuffer_handle_t handle, void *buffer, uint32_t size){
    uint32_t move_size;
    rbuffer_t *rbuffer = NULL;

    rbuffer = (rbuffer_t *)handle;

    if(!rbuffer){
        return 0;
    }

    if(rbuffer->suspend || buffer == NULL || size == 0){
        return 0;
    }

    size = MIN(size, rbuffer->curr_size);

    move_size = MIN(size, rbuffer->init_size - rbuffer->read_pos);

    memcpy(buffer, (uint8_t *)rbuffer->buffer + rbuffer->read_pos, move_size);

    if(size - move_size > 0){
        rbuffer->read_pos = 0;
        memcpy((uint8_t *)buffer + move_size, rbuffer->buffer, size - move_size);
        move_size = size - move_size;
    }

    rbuffer->read_pos += move_size;
    rbuffer->curr_size -= size;

    return size;
}

bool rbuffer_reset(rbuffer_handle_t handle){
    rbuffer_t *rbuffer = NULL;

    rbuffer = (rbuffer_t *)handle;

    if(!rbuffer){
        return false;
    }

    rbuffer->read_pos = 0;
    rbuffer->write_pos = 0;
    rbuffer->curr_size = 0;
    // memset(rbuffer->buffer, 0, rbuffer->init_size);

    return true;
}

uint32_t rbuffer_discard(rbuffer_handle_t handle, uint32_t size){
    rbuffer_t *rbuffer = NULL;

    rbuffer = (rbuffer_t *)handle;

    if(!rbuffer){
        return false;
    }

    size = MIN(size, rbuffer->curr_size);
    rbuffer->read_pos = (rbuffer->init_size + rbuffer->read_pos + size)%rbuffer->init_size;
    rbuffer->curr_size -= size;

    return size;
}

bool rbuffer_suspend(rbuffer_handle_t handle){
    rbuffer_t *rbuffer = NULL;

    rbuffer = (rbuffer_t *)handle;

    if(!rbuffer){
        return false;
    }

    rbuffer->suspend = true;

    return true;
}

bool rbuffer_resume(rbuffer_handle_t handle){
    rbuffer_t *rbuffer = NULL;

    rbuffer = (rbuffer_t *)handle;

    if(!rbuffer){
        return false;
    }

    rbuffer->suspend = false;

    return true;
}

uint32_t rbuffer_total_size(rbuffer_handle_t handle){
    rbuffer_t *rbuffer = NULL;

    rbuffer = (rbuffer_t *)handle;

    if(!rbuffer){
        return 0;
    }

    return rbuffer->init_size;
}

uint32_t rbuffer_used_size(rbuffer_handle_t handle){
    rbuffer_t *rbuffer = NULL;

    rbuffer = (rbuffer_t *)handle;

    if(!rbuffer){
        return 0;
    }

    return rbuffer->curr_size;
}

uint32_t rbuffer_available_size(rbuffer_handle_t handle){
    rbuffer_t *rbuffer = NULL;

    rbuffer = (rbuffer_t *)handle;

    if(!rbuffer){
        return 0;
    }
    
    return rbuffer->init_size - rbuffer->curr_size;
}

bool rbuffer_is_empty(rbuffer_handle_t handle){
    rbuffer_t *rbuffer = NULL;

    rbuffer = (rbuffer_t *)handle;

    if(!rbuffer){
        return true;
    }
    
    return ((rbuffer->curr_size == 0)?true:false);
}

bool rbuffer_is_full(rbuffer_handle_t handle){
    rbuffer_t *rbuffer = NULL;

    rbuffer = (rbuffer_t *)handle;

    if(!rbuffer){
        return false;
    }
    
    return ((rbuffer->curr_size == rbuffer->init_size)?true:false);
}

bool rbuffer_get_head_index(rbuffer_handle_t handle, uint32_t *index){
    rbuffer_t *rbuffer = NULL;

    rbuffer = (rbuffer_t *)handle;

    if(!rbuffer){
        return false;
    }

    *index = rbuffer->read_pos;

    return true;
}

bool rbuffer_get_end_index(rbuffer_handle_t handle, uint32_t *index){
    rbuffer_t *rbuffer = NULL;

    rbuffer = (rbuffer_t *)handle;

    if(!rbuffer){
        return false;
    }

    *index = rbuffer->write_pos;

    return true;
}

bool rbuffer_get_head_forward_index(rbuffer_handle_t handle, uint32_t forward, uint32_t *index){
    rbuffer_t *rbuffer = NULL;

    rbuffer = (rbuffer_t *)handle;

    if(!rbuffer){
        return false;
    }
    
    if(forward > rbuffer->curr_size){
        return false;
    }

    *index = (rbuffer->init_size + rbuffer->read_pos + forward)%rbuffer->init_size;

    return true;
}

bool rbuffer_get_end_backward_index(rbuffer_handle_t handle, uint32_t backward, uint32_t *index){
    rbuffer_t *rbuffer = NULL;

    rbuffer = (rbuffer_t *)handle;

    if(!rbuffer){
        return false;
    }

    if(backward > rbuffer->curr_size){
        return false;
    }

    *index = (rbuffer->init_size + rbuffer->write_pos - backward)%rbuffer->init_size;

    return true;
}

uint32_t rbuffer_get_buffer(rbuffer_handle_t handle, uint32_t index, void *buffer, uint32_t size){
    uint32_t move_size;
    rbuffer_t *rbuffer = NULL;

    rbuffer = (rbuffer_t *)handle;

    if(!rbuffer){
        return 0;
    }

    index = index%rbuffer->init_size;

    size = MIN(size, rbuffer->init_size);

    move_size = MIN(size, rbuffer->init_size - index);

    memcpy(buffer, (uint8_t *)rbuffer->buffer + index, move_size);

    if(size - move_size > 0){
        memcpy((uint8_t *)buffer + move_size, rbuffer->buffer, size - move_size);
        move_size = size - move_size;
    }

    return size;
}


void rbuffer_dump(rbuffer_handle_t handle, uint32_t size){
    uint32_t i = 0;
    rbuffer_t *rbuffer = NULL;

    rbuffer = (rbuffer_t *)handle;

    if(!rbuffer){
        return;
    }

    size = MIN(size, rbuffer->curr_size);

    RBUFFER_PRINTF("rbuffer dump:");
    for (i = 0; i < rbuffer->curr_size; i++)
    {
        if(i%16==0){
            RBUFFER_PRINTF("\n[%d - %d]: ", i, ((rbuffer->curr_size-i)>16)?(i+15):(i+rbuffer->curr_size-i-1));
        }
        RBUFFER_PRINTF("%d ", *((uint8_t *)rbuffer->buffer + (rbuffer->init_size + rbuffer->read_pos + i)%rbuffer->init_size)); 
    }
    RBUFFER_PRINTF("\n");
}
