#ifndef SATAN_SCHEDULER_H_
#define SATAN_SCHEDULER_H_

#include <stdint.h>

#include "task_context.h"
#include "vfs/vfs.h"


// called from kernel.c entrypoint
void scheduler_init(void);
void scheduler_start(void *init_proc);

// called from asm code in interupt.S
void scheduler_save_current_context(const task_context_t *current_context);
const task_context_t *scheduler_switch_task(void);

// syscall handler api: Current process management
void scheduler_cur_proc_set_syscall_status(int32_t status);

void scheduler_cur_proc_exit(int32_t status);
void* scheduler_cur_proc_get_kernel_address(uintptr_t process_virtual_address);

int32_t scheduler_cur_proc_fork(void);
int32_t scheduler_cur_proc_wait_id(int32_t pid, uint32_t *wstatus);
int32_t scheduler_cur_proc_get_id(void);
int32_t scheduler_cur_proc_get_parent_id(void);

// 
int32_t scheduler_cur_proc_add_fd(file_descriptor_t descriptor);
file_descriptor_t *scheduler_cur_proc_get_fd(int32_t fd);


#endif
