# RingBuffer
使用C/C++编写的简单嵌入式RingBuffer, 不带锁

### C例程
```
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
rbuffer_dump(rbuffer, rbuffer_used_size(rbuffer));    //[0 - 7]: 0 1 2 3 4 5 6 7

pop_size = 2;
rbuffer_pop(rbuffer, buf, pop_size);
printf("pop: ");
for (size_t i = 0; i < pop_size; i++){
    printf("%d ", buf[i]);
}
printf("\n");
rbuffer_dump(rbuffer, rbuffer_used_size(rbuffer));    //[0 - 5]: 2 3 4 5 6 7

rbuffer_discard(rbuffer, 3);
rbuffer_dump(rbuffer, rbuffer_used_size(rbuffer));    //[0 - 2]: 5 6 7

size = rbuffer_used_size(rbuffer);
rbuffer_get_head_index(rbuffer, &index);
rbuffer_get_buffer(rbuffer, index, buf, size);

push_size = 8;
printf("push_size: %d\n", push_size);
rbuffer_push(rbuffer, buf2, push_size, false);        
rbuffer_dump(rbuffer, rbuffer_used_size(rbuffer));    //[0 - 9]: 5 6 7 10 11 12 13 14 15 16

if(rbuffer_is_full(rbuffer)){
    printf("rbuffer is full\n");
}

rbuffer_reset(rbuffer);
rbuffer_push(rbuffer, buf, size, false);             
rbuffer_dump(rbuffer, rbuffer_used_size(rbuffer));    //[0 - 2]: 5 6 7

printf("cover_push_size: %d\n", push_size);
rbuffer_push(rbuffer, buf2, push_size, true);
rbuffer_dump(rbuffer, rbuffer_used_size(rbuffer));    //[0 - 9]: 6 7 10 11 12 13 14 15 16 17
```