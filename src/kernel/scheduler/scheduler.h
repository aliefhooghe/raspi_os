#ifndef SATAN_SCHEDULER_H_
#define SATAN_SCHEDULER_H_

#include <stdint.h>

#include "task_context.h"
#include "vfs/vfs.h"

#define SCHEDULER_MAX_TASK_COUNT 0x80u



void scheduler_init(void);
void scheduler_start(void);

// called from asm code in interupt.S
void scheduler_save_current_context(const task_context_t *current_context);
const task_context_t *scheduler_switch_task(void);

//

int32_t scheduler_add_task(void *proc_address, uint32_t param);

// current process getters
void scheduler_cur_proc_set_syscall_status(int32_t status);

void scheduler_cur_proc_exit(void);
void* scheduler_cur_proc_get_kernel_address(uintptr_t process_virtual_address);
int32_t scheduler_cur_proc_fork(void);

int32_t scheduler_cur_proc_get_id(void);
file_descriptor_t *scheduler_cur_proc_get_fd(int32_t fd);

#endif
