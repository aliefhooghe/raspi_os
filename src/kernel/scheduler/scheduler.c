
#include <stddef.h>
#include <stdint.h>

#include "kernel.h"

#include "scheduler.h"
#include "task_context.h"

#include "hardware/cpu.h"
#include "hardware/mmu.h"

#include "lib/str.h"

#include "memory/translation_table_allocator.h"

#include "vfs/vfs.h"

/**
 *  task structure
 */
typedef struct {

    //
    //  Process management
    //
    task_context_t context;     // saved task context (register and processor status)
    int32_t id;                 // process id: pid

    //
    //  Memory management
    //
    uint32_t *translation_table;

    //
    //  Virtual file system interfaces
    //

    // on voudrais ici un array des fichier ouverts
    // modèle de polymorphisme sans doute à revoir.
    // est ce que certaines choses ne sont pas stocké dans le vfs ?
    file_descriptor_t file_descriptors[2];  // stdin, stdout
    uint32_t fd_count;

    // comment gérer un appel open ? Notion de path coté vfs. Stocker le fd: coté process ?
    /**

        fd = vfs.open(path)
        task.fds.push(fd)

     */

     // qui initialize stdin/stdout pour un process ?

} task_t; // a renommer => process


/**
 *  scheduler structure
 */
typedef struct {
    task_t tasks[SCHEDULER_MAX_TASK_COUNT]; // process table.
    uint32_t current_task;
    uint32_t task_count;
    uint32_t id_gen;
    uint32_t stop_current_task;
} scheduler_t;

/**
 *  set the current task context into the cpu
 */
extern void __set_task_context(task_context_t *current_context);

/**
 *  global scheduler state
 */
static scheduler_t _scheduler;


void scheduler_init(void)
{
    _memset(&_scheduler, 0, sizeof(scheduler_t));
}

void scheduler_start(void)
{
    if (_scheduler.task_count > 0u) {
        __set_task_context(&_scheduler.tasks[0].context);
    }
    else {
        kernel_fatal_error("no task to be started");
    }
}

static void _cleanup_task(task_t* task)
{
    translation_table_allocator_free(task->translation_table);
}

const task_context_t *scheduler_switch_task(const task_context_t *current_context)
{
    if (_scheduler.stop_current_task)
    {
        _scheduler.stop_current_task = 0u;

        // free task related resources
        _cleanup_task(&_scheduler.tasks[_scheduler.current_task]);

        // remove the current task. No state to save
        _memmove(
            &_scheduler.tasks[_scheduler.current_task],
            &_scheduler.tasks[_scheduler.current_task + 1],
            sizeof(task_t) * (_scheduler.task_count - (_scheduler.current_task + 1)));
        _scheduler.task_count--;

        // index stay the same
    }
    else {
        // save the current task context context
        _memcpy(
            &_scheduler.tasks[_scheduler.current_task].context,
            current_context,
            sizeof(task_context_t));

        // compute the next task index
        _scheduler.current_task = _scheduler.current_task + 1;
    }

    // round robin
    if (_scheduler.task_count == 0u)
    {
        kernel_fatal_error("the last running task was stopped");
    }
    if (_scheduler.current_task == _scheduler.task_count)
    {
        _scheduler.current_task = 0u;
    }

    // switch to next task
    return &_scheduler.tasks[_scheduler.current_task].context;
}

//
//  Task initialization
//

static void scheduler_task_context_init(
    task_context_t *context,
    void *stack_address, uintptr_t proc_address,
    uint32_t param)
{
    context->r0 = param;
    context->sp = (uint32_t)stack_address;
    context->lr = proc_address;
    context->spsr =
        CPU_CPSR_MODE_USER |
        CPU_CPSR_DISABLE_IRQ |
        CPU_CPSR_DISABLE_FIQ;
}

static void scheduler_task_init(
    task_t *new_task, uint32_t task_id,
    void *stack_address, uintptr_t proc_address,
    uint32_t param)
{
    _memset(new_task, 0, sizeof(task_t));

    // set pid
    new_task->id = task_id;

    // set initial task context
    scheduler_task_context_init(
        &new_task->context,
        stack_address, proc_address, param);

    // setup a virtual memory region.
    new_task->translation_table = translation_table_allocator_alloc();

    // setup stdin/stdout
    const file_descriptor_t tty_fd = vfs_get_tty_file_descriptor();
    new_task->fd_count = 2u;
    new_task->file_descriptors[0] = tty_fd;
    new_task->file_descriptors[1] = tty_fd;
}

int32_t scheduler_add_task(
    uintptr_t proc_address,
    void* stack_address,
    uint32_t param)
{
    if (_scheduler.task_count >= SCHEDULER_MAX_TASK_COUNT)
    {
        return -1;
    }

    // initialize the new task context
    const int32_t new_task_id = _scheduler.id_gen++;
    const uint32_t new_index = _scheduler.task_count++;

    task_t *new_task = &_scheduler.tasks[new_index];
    scheduler_task_init(
        new_task, new_task_id,
        stack_address, proc_address, param);

    return new_task_id;
}

//
//  Current process management
//

void scheduler_cur_proc_exit(void)
{
    _scheduler.stop_current_task = 1u;
}

int32_t scheduler_cur_proc_get_id(void)
{
    if (_scheduler.current_task < _scheduler.task_count)
    {
        return _scheduler.tasks[_scheduler.current_task].id;
    }
    else
    {
        return -1;
    }
}

file_descriptor_t *scheduler_cur_proc_get_fd(int32_t fd)
{
    task_t *current_task = &_scheduler.tasks[_scheduler.current_task];
    if ((uint32_t)fd >= current_task->fd_count)
        return NULL;
    return &current_task->file_descriptors[fd];
}
