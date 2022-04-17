/*
 * @Author: HoGC
 * @Date: 2022-03-16 19:59:10
 * @Last Modified time: 2022-03-16 19:59:10
 */

#ifndef __RBUFFER_H__
#define __RBUFFER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#include <stdio.h>


#define RBUFFER_MALLOC(size)  malloc(size)
#define RBUFFER_FREE(block)   free(block)

#define RBUFFER_PRINTF(format, ...) printf(format, ##__VA_ARGS__) 


typedef void* rbuffer_handle_t;

rbuffer_handle_t rbuffer_create(uint32_t size);
rbuffer_handle_t rbuffer_static_create(void *buffer, uint32_t size);
bool rbuffer_delete(rbuffer_handle_t handle);

uint32_t rbuffer_push(rbuffer_handle_t handle, void *buffer, uint32_t size, bool cover);
uint32_t rbuffer_pop(rbuffer_handle_t handle, void *buffer, uint32_t size);

bool rbuffer_reset(rbuffer_handle_t handle);
uint32_t rbuffer_discard(rbuffer_handle_t handle, uint32_t size);

bool rbuffer_suspend(rbuffer_handle_t handle);
bool rbuffer_resume(rbuffer_handle_t handle);

uint32_t rbuffer_total_size(rbuffer_handle_t handle);
uint32_t rbuffer_used_size(rbuffer_handle_t handle);
uint32_t rbuffer_available_size(rbuffer_handle_t handle);

bool rbuffer_is_empty(rbuffer_handle_t handle);
bool rbuffer_is_full(rbuffer_handle_t handle);

bool rbuffer_get_head_index(rbuffer_handle_t handle, uint32_t *index);
bool rbuffer_get_end_index(rbuffer_handle_t handle, uint32_t *index);
bool rbuffer_get_head_forward_index(rbuffer_handle_t handle, uint32_t forward, uint32_t *index);
bool rbuffer_get_end_backward_index(rbuffer_handle_t handle, uint32_t backward, uint32_t *index);
uint32_t rbuffer_get_buffer(rbuffer_handle_t handle, uint32_t index, void *buffer, uint32_t size);

void rbuffer_dump(rbuffer_handle_t handle, uint32_t size);


#ifdef __cplusplus
}
#endif

#endif // __RBUFFER_H__