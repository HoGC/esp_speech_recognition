/*
 * @Author: HoGC
 * @Date: 2022-03-16 19:59:18
 * @Last Modified time: 2022-03-16 19:59:18
 */

#include <stdio.h>
#include <string.h>

extern "C" {
    #include "rbuffer.h"
}

#include "rbuffer.hpp"

extern "C" void app_main(void)
{
    printf("C example!\n");
    
    uint32_t size = 0;
    uint32_t index = 0;
    uint32_t pop_size = 0;
    uint32_t push_size = 0;
    uint8_t buf[10] = {0};
    uint8_t buf1[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    uint8_t buf2[10] = {10, 11, 12, 13, 14, 15, 16, 17, 18, 19};

    memset(buf, 0, sizeof(buf));

    rbuffer_handle_t rbuffer = rbuffer_create(10);

    if(rbuffer_is_empty(rbuffer)){
        printf("rbuffer is empty\n");
    }

    push_size = 8;
    rbuffer_push(rbuffer, buf1, push_size, false);
    rbuffer_dump(rbuffer, rbuffer_used_size(rbuffer));

    pop_size = 2;
    rbuffer_pop(rbuffer, buf, pop_size);
    printf("pop: ");
    for (size_t i = 0; i < pop_size; i++){
        printf("%d ", buf[i]);
    }
    printf("\n");
    rbuffer_dump(rbuffer, rbuffer_used_size(rbuffer));

    rbuffer_discard(rbuffer, 3);
    rbuffer_dump(rbuffer, rbuffer_used_size(rbuffer));

    size = rbuffer_used_size(rbuffer);
    rbuffer_get_head_index(rbuffer, &index);
    rbuffer_get_buffer(rbuffer, index, buf, size);
    
    push_size = 8;
    printf("push_size: %d\n", push_size);
    rbuffer_push(rbuffer, buf2, push_size, false);
    rbuffer_dump(rbuffer, rbuffer_used_size(rbuffer));

    if(rbuffer_is_full(rbuffer)){
        printf("rbuffer is full\n");
    }

    rbuffer_reset(rbuffer);
    rbuffer_push(rbuffer, buf, size, false);

    printf("cover_push_size: %d\n", push_size);
    rbuffer_push(rbuffer, buf2, push_size, true);
    rbuffer_dump(rbuffer, rbuffer_used_size(rbuffer));

    printf("CPP example!\n");

    memset(buf, 0, sizeof(buf));

    Rbuffer *rbuffer1 = new Rbuffer(10);

    if(rbuffer1->is_empty()){
        printf("rbuffer1 is empty\n");
    }

    push_size = 8;
    rbuffer1->push(buf1, push_size, false);
    rbuffer1->dump(rbuffer1->used_size());

    pop_size = 2;
    rbuffer1->pop(buf, pop_size);
    printf("pop: ");
    for (size_t i = 0; i < pop_size; i++){
        printf("%d ", buf[i]);
    }
    printf("\n");
    rbuffer1->dump(rbuffer1->used_size());

    rbuffer1->discard(3);
    rbuffer1->dump(rbuffer1->used_size());

    size = rbuffer1->used_size();
    rbuffer1->get_head_index(&index);
    rbuffer1->get_buffer(index, buf, size);
    
    push_size = 8;
    printf("push_size: %d\n", push_size);
    rbuffer1->push(buf2, push_size, false);
    rbuffer1->dump(rbuffer1->used_size());

    if(rbuffer1->is_full()){
        printf("rbuffer1 is full\n");
    }

    rbuffer1->reset();
    rbuffer1->push(buf, size, false);

    printf("cover_push_size: %d\n", push_size);
    rbuffer1->push(buf2, push_size, true);
    rbuffer1->dump(rbuffer1->used_size());
}
