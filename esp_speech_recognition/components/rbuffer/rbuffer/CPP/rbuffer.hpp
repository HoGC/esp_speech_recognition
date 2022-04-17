/*
 * @Author: HoGC
 * @Date: 2022-03-16 19:59:15
 * @Last Modified time: 2022-03-16 19:59:15
 */

#ifndef __RBUFFER_HPP__
#define __RBUFFER_HPP__

#include <stdint.h>
#include <stdbool.h>

#include <stdio.h>


#define RBUFFER_MALLOC(size)  malloc(size)
#define RBUFFER_FREE(block)   free(block)

#define RBUFFER_PRINTF(format, ...) printf(format, ##__VA_ARGS__) 


class Rbuffer
{
private:
    void *m_buffer;
    bool m_suspend;
    bool m_need_free;
    uint32_t m_init_size;
    uint32_t m_curr_size;
    uint32_t m_read_pos, m_write_pos;
public:
    Rbuffer(uint32_t size);
    Rbuffer(void *buffer, uint32_t size);
    ~Rbuffer();

    uint32_t push(void *buffer, uint32_t size, bool cover);
    uint32_t pop(void *buffer, uint32_t size);

    bool reset();
    uint32_t discard(uint32_t size);

    bool suspend();
    bool resume();

    uint32_t total_size();
    uint32_t used_size();
    uint32_t available_size();

    bool is_empty();
    bool is_full();

    bool get_head_index(uint32_t *index);
    bool get_end_index(uint32_t *index);
    bool get_head_forward_index(uint32_t forward, uint32_t *index);
    bool get_end_backward_index(uint32_t backward, uint32_t *index);
    uint32_t get_buffer(uint32_t index, void *buffer, uint32_t size);

    void dump(uint32_t size);
};

#endif // __RBUFFER_HPP__