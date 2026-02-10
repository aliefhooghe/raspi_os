#ifndef SATAN_SCHEDULER_H_
#define SATAN_SCHEDULER_H_

#include <stdint.h>

#include "task_context.h"
#include "vfs/vfs.h"

#define PROCESS_MAX_ARG_COUNT  8u
#define PROCESS_MAX_ARG_SIZE   32u

//
// Not beautiful but we must copy args in kernel space before cleaning current
// process when passing arguments
typedef struct {
    int argc;
    char argv[PROCESS_MAX_ARG_COUNT][PROCESS_MAX_ARG_SIZE];
} process_args_t;

typedef struct process process_t;

// called from kernel.c entrypoint
void scheduler_init(void);
void scheduler_start(const char *init_path);

// called from asm code in interupt.S
void scheduler_save_current_context(const task_context_t *current_context);
const process_t *scheduler_switch_task(void);

// syscall handler api: Current process management
void scheduler_cur_proc_set_syscall_status(int32_t status);

void scheduler_cur_proc_exit(int32_t status);
int32_t scheduler_cur_proc_fork(void);
int32_t scheduler_cur_proc_exec(const char *path, const process_args_t *argv);
int32_t scheduler_cur_proc_wait_id(int32_t pid, uint32_t *wstatus);
int32_t scheduler_cur_proc_get_id(void);
int32_t scheduler_cur_proc_get_parent_id(void);

//
// Current process utils
//

//
int32_t scheduler_cur_proc_add_fd(file_t *descriptor);

//
void scheduler_cur_proc_rem_fd(int32_t fd);

// get a file by fd for currrent proc
file_t *scheduler_cur_proc_get_fd(int32_t fd);

// translate an address in current process virtual space to kernel address
void* scheduler_cur_proc_to_kernel_address(uintptr_t process_virtual_address);

#endif
