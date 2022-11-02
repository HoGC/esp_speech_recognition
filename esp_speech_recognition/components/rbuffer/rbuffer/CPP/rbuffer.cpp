/*
 * @Author: HoGC
 * @Date: 2022-03-16 19:59:12
 * @Last Modified time: 2022-03-16 19:59:12
 */

#include "rbuffer.hpp"

#include <string.h>

#define MIN(a, b) ((a)<(b)?(a):(b))

Rbuffer::Rbuffer(uint32_t size): m_buffer(nullptr), m_suspend(false), m_need_free(true), m_init_size(size), m_curr_size(0), m_read_pos(0), m_write_pos(0){

    m_buffer = RBUFFER_MALLOC(size);

    if(!m_buffer){
        // throw bad_alloc
    }

    memset(m_buffer, 0, size);
}

Rbuffer::Rbuffer(void *buffer, uint32_t size): m_buffer(buffer), m_suspend(false), m_need_free(false), m_init_size(size), m_curr_size(0), m_read_pos(0), m_write_pos(0){

}

Rbuffer::~Rbuffer(){
    
    if(m_buffer){
        RBUFFER_FREE(m_buffer);
    }
}

uint32_t Rbuffer::push(void *buffer, uint32_t size, bool cover){
    uint32_t move_size = 0;

    if(m_suspend || buffer == NULL || size == 0){
        return 0;
    }

    if(!cover){
        size = MIN(size, m_init_size - m_curr_size);
    }

    move_size = MIN(size, m_init_size - m_write_pos);

    memcpy((uint8_t *)m_buffer + m_write_pos, buffer, move_size);

    if(size - move_size > 0){
        m_write_pos = 0;
        memcpy(m_buffer, (uint8_t *)buffer + move_size, size - move_size);
        move_size = size - move_size;
    }
    
    m_write_pos += move_size;
    if(m_curr_size + size > m_init_size){
        m_curr_size = m_init_size;
        m_read_pos = m_write_pos;
    }else{
        m_curr_size += size;
    }

    return size;
}

uint32_t Rbuffer::pop(void *buffer, uint32_t size){
    uint32_t move_size;

    if(m_suspend || buffer == NULL || size == 0){
        return 0;
    }

    size = MIN(size, m_curr_size);

    move_size = MIN(size, m_init_size - m_read_pos);

    memcpy(buffer, (uint8_t *)m_buffer + m_read_pos, move_size);

    if(size - move_size > 0){
        m_read_pos = 0;
        memcpy((uint8_t *)buffer + move_size, m_buffer, size - move_size);
        move_size = size - move_size;
    }

    m_read_pos += move_size;
    m_curr_size -= size;

    return size;
}

bool Rbuffer::reset(){

    m_read_pos = 0;
    m_write_pos = 0;
    m_curr_size = 0;
    // memset(m_buffer, 0, m_init_size);

    return true;
}

uint32_t Rbuffer::discard(uint32_t size){

    size = MIN(size, m_curr_size);
    m_read_pos = (m_init_size + m_read_pos + size)%m_init_size;
    m_curr_size -= size;

    return size;
}

bool Rbuffer::suspend(){

    m_suspend = true;

    return true;
}

bool Rbuffer::resume(){

    m_suspend = false;

    return true;
}

uint32_t Rbuffer::total_size(){

    return m_init_size;
}

uint32_t Rbuffer::used_size(){

    return m_curr_size;
}

uint32_t Rbuffer::available_size(){
    
    return m_init_size - m_curr_size;
}

bool Rbuffer::is_empty(){
    
    return ((m_curr_size == 0)?true:false);
}

bool Rbuffer::is_full(){
    
    return ((m_curr_size == m_init_size)?true:false);
}

bool Rbuffer::get_head_index(uint32_t *index){

    *index = m_read_pos;

    return true;
}

bool Rbuffer::get_end_index(uint32_t *index){

    *index = m_write_pos;

    return true;
}

bool Rbuffer::get_head_forward_index(uint32_t forward, uint32_t *index){
    
    if(forward > m_curr_size){
        return false;
    }

    *index = (m_init_size + m_read_pos + forward)%m_init_size;

    return true;
}

bool Rbuffer::get_end_backward_index(uint32_t backward, uint32_t *index){

    if(backward > m_curr_size){
        return false;
    }

    *index = (m_init_size + m_write_pos - backward)%m_init_size;

    return true;
}

uint32_t Rbuffer::get_buffer(uint32_t index, void *buffer, uint32_t size){
    uint32_t move_size;

    index = index%m_init_size;

    size = MIN(size, m_init_size);

    move_size = MIN(size, m_init_size - index);

    memcpy(buffer, (uint8_t *)m_buffer + index, move_size);

    if(size - move_size > 0){
        memcpy((uint8_t *)buffer + move_size, m_buffer, size - move_size);
        move_size = size - move_size;
    }

    return size;
}


void Rbuffer::dump(uint32_t size){
    uint32_t i = 0;

    size = MIN(size, m_curr_size);

    RBUFFER_PRINTF("rbuffer dump:");
    for (i = 0; i < m_curr_size; i++)
    {
        if(i%16==0){
            RBUFFER_PRINTF("\n[%d - %d]: ", i, ((m_curr_size-i)>16)?(i+15):(i+m_curr_size-i-1));
        }
        RBUFFER_PRINTF("%d ", *((uint8_t *)m_buffer + (m_init_size + m_read_pos + i)%m_init_size)); 
    }
    RBUFFER_PRINTF("\n");
}
