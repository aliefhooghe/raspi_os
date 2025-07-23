
#include <stddef.h>
#include <stdint.h>

#include "hardware/cpu.h"
#include "hardware/mmu.h"

#include "kernel.h"
#include "lib/str.h"

#include "memory/section_allocator.h"
#include "memory/translation_table_allocator.h"

#include "scheduler.h"
#include "task_context.h"
#include "vfs/vfs.h"

#define PROCESS_SECTION_VIRTUAL_ADDRRESS  0x00800000u  // section 0x00800000u -> 0x00900000u = 1Mb
#define PROCESS_STACK_VIRTUAL_ADDRRESS    0x00880000u  // stack at the midle of section (= a 512 kb stack)

/**
 *  task structure
 */
typedef struct {

    //
    //  Process management
    //
    task_context_t context;         // saved task context (register and processor status)
    int32_t id;                     // process id: pid

    //
    //  Memory management
    //
    uint32_t *translation_table;    // process translation table.
    void *memory_section;           // for now 1 memory section per process.

    //
    //  Virtual file system interfaces
    //
    file_descriptor_t file_descriptors[2];  // stdin, stdout
    uint32_t fd_count;
} task_t; // a renommer => process


/**
 *  scheduler structure
 */
typedef struct {
    task_t tasks[SCHEDULER_MAX_TASK_COUNT]; // process table.
    uint32_t current_task;
    uint32_t task_count;
    uint32_t id_gen;
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
    // HORRIBLE
    if (_scheduler.task_count > 0u) {
        task_t *task = &_scheduler.tasks[0];
        mmu_set_translation_table(task->translation_table);
        __set_task_context(&task->context);
    }
    else {
        kernel_fatal_error("no task to be started");
    }
}

static void _cleanup_task(task_t* task)
{
    translation_table_allocator_free(task->translation_table);
    section_allocator_free(task->memory_section);
}

void scheduler_save_current_context(const task_context_t *current_context)
{
    // save the current task context context
    _memcpy(
        &_scheduler.tasks[_scheduler.current_task].context,
        current_context,
        sizeof(task_context_t));
}

const task_context_t *scheduler_switch_task(void)
{
    // compute the next task index: round robin
    _scheduler.current_task = _scheduler.current_task + 1;
    if (_scheduler.current_task == _scheduler.task_count)
    {
        _scheduler.current_task = 0u;
    }

    // switch to next task:
    const task_t *current_task = &_scheduler.tasks[_scheduler.current_task];

    // 1 - select the process translation table
    mmu_set_translation_table(current_task->translation_table);

    // 2 - return the task context to be restored
    return &current_task->context;
}

//
//  Task initialization
//

static void scheduler_task_context_init(
    task_context_t *context,
    uintptr_t stack_address,
    void *proc_address,
    uint32_t param)
{
    context->r0 = param;
    context->sp = stack_address;
    context->lr = (uint32_t)proc_address;
    context->spsr =
        CPU_CPSR_MODE_USER |
        CPU_CPSR_DISABLE_IRQ |
        CPU_CPSR_DISABLE_FIQ;
}

static void scheduler_task_init(
    task_t *new_task,
    uint32_t task_id)
{
    _memset(new_task, 0, sizeof(task_t));

    // set pid
    new_task->id = task_id;

    // 1 - allocate a memory section for the process
    new_task->memory_section = section_allocator_alloc();
    if (new_task->memory_section == NULL)
    {
        kernel_fatal_error("failed to allocate a process memory section");
    }

    // 2 - allocate the process address translation table.
    new_task->translation_table = translation_table_allocator_alloc();
    if (new_task->translation_table == NULL)
    {
        kernel_fatal_error("failed to allocate a process transaction table");
    }
}

int32_t scheduler_add_task(
    void *proc_address,
    uint32_t param)
{
    if (_scheduler.task_count >= SCHEDULER_MAX_TASK_COUNT)
    {
        return -1;
    }

    // initialize the new task
    const int32_t new_task_id = _scheduler.id_gen++;
    const uint32_t new_index = _scheduler.task_count++;

    task_t *new_task = &_scheduler.tasks[new_index];
    scheduler_task_init(new_task, new_task_id);

    //
    // setup the translation table
    //

    // map the kernel code and data + heap
    translation_table_add_identity_mapping(
        new_task->translation_table,
        0x00000000u, 0x00800000u,
        MMU_L1_SECTION_AP_KERNEL_RW_USER_RW);

    // map the process section
    translation_table_add_single_section(
        new_task->translation_table,
        new_task->memory_section,
        PROCESS_SECTION_VIRTUAL_ADDRRESS,
        MMU_L1_SECTION_AP_KERNEL_RW_USER_RW);

    //
    // set initial task context
    //
    scheduler_task_context_init(
        &new_task->context,
        PROCESS_STACK_VIRTUAL_ADDRRESS,
        proc_address, param);

    // setup stdin/stdout
    const file_descriptor_t tty_fd = vfs_get_tty_file_descriptor();
    new_task->fd_count = 2u;
    new_task->file_descriptors[0] = tty_fd;
    new_task->file_descriptors[1] = tty_fd;
    return new_task_id;
}

//
//  Current process management
//

void scheduler_cur_proc_set_syscall_status(int32_t status)
{
    _scheduler
        .tasks[_scheduler.current_task]
        .context
        .r0 = status;
}

void* scheduler_cur_proc_get_kernel_address(uintptr_t process_virtual_address)
{
    return mmu_translate_virtual_address(
        _scheduler.tasks[_scheduler.current_task].translation_table,
        process_virtual_address);
}

int32_t scheduler_cur_proc_fork(void)
{
    if (_scheduler.task_count >= SCHEDULER_MAX_TASK_COUNT)
    {
        return -1;
    }

    // initialize the new task id
    const int32_t new_task_id = _scheduler.id_gen++;
    const uint32_t new_index = _scheduler.task_count++;

    // init task: allocate its own section and translation table
    task_t *new_task = &_scheduler.tasks[new_index];
    scheduler_task_init(new_task, new_task_id);

    // fork the current task
    const task_t *current_task = &_scheduler.tasks[_scheduler.current_task];

    // fork: copy the context
    _memcpy(
        &new_task->context,
        &current_task->context,
        sizeof(task_context_t));

    // fork: copy the memory section
    _memcpy(
        &new_task->memory_section,
        &current_task->memory_section,
        MMU_SECTION_SIZE);

    // fork: copy oppened file descriptors
    new_task->fd_count = current_task->fd_count;
    _memcpy(
        &new_task->file_descriptors,
        &current_task->file_descriptors,
        sizeof(file_descriptor_t) * current_task->fd_count);

    // fork: copy the translation table, patch the virtual process section
    // todo: copy and write. Handle several sections.
    _memcpy(
        &new_task->translation_table,
        &current_task->translation_table,
        sizeof(uint32_t) * MMU_L1_ENTRY_COUNT);
    translation_table_add_single_section(
        new_task->translation_table,
        new_task->memory_section,
        PROCESS_SECTION_VIRTUAL_ADDRRESS,
        MMU_L1_SECTION_AP_KERNEL_RW_USER_RW);

    return new_task_id;
}

void scheduler_cur_proc_exit(void)
{
    // free task related resources
    _cleanup_task(&_scheduler.tasks[_scheduler.current_task]);

    // delete task
    _memmove(
        &_scheduler.tasks[_scheduler.current_task],
        &_scheduler.tasks[_scheduler.current_task + 1],
        sizeof(task_t) * (_scheduler.task_count - (_scheduler.current_task + 1)));

    // update task count.
    _scheduler.task_count--;
    if (_scheduler.task_count == 0u)
    {
        kernel_fatal_error("the last running task was stopped");
    }

    // If last task was current, goto task 0
    // TODO: implement __aeabi_uidivmod in order to use % operator
    if (_scheduler.current_task == _scheduler.task_count)
    {
        _scheduler.current_task = 0u;
    }
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
