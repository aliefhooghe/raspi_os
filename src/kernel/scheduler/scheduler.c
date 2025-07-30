#include <stddef.h>
#include <stdint.h>

#include "hardware/cpu.h"
#include "hardware/mini_uart.h"
#include "hardware/mmu.h"

#include "kernel.h"
#include "lib/str.h"

#include "memory/section_allocator.h"
#include "memory/translation_table_allocator.h"

#include "scheduler.h"
#include "task_context.h"
#include "vfs/vfs.h"


///
//  Process memory mapping
// 
#define PROCESS_SECTION_VIRTUAL_ADDRRESS  0x00800000u  // section 0x00800000u -> 0x00900000u = 1Mb
#define PROCESS_STACK_VIRTUAL_ADDRRESS    0x00880000u  // stack at the midle of section (= a 512 kb stack)

//
//  Process structure
// 
#define MAX_FILE_DESCRIPTOR_COUNT (4u)
#define SCHEDULER_MAX_TASK_COUNT  (32u)

#define INIT_TASK_PID   1u

// worst case: all childs belong to init
#define MAX_EXITED_CHILD_TASK     SCHEDULER_MAX_TASK_COUNT 

typedef struct {
    enum {
        TASK_SCHEDULED,
        TASK_SUSPENDED,
        TASK_EXITED
    } status;
    union {

        struct {
            int32_t wait_pid;
            uint32_t *wstatus_ref;
        } suspended;

        struct {
            uint32_t wstatus;
        } exited;        
    };
} schedule_state_t;

typedef struct {

    //
    // Process management
    //
    task_context_t context;         // saved task context (register and processor status)
    int32_t id;                     // process id: pid
    int32_t parent_id;              // parent process id: ppid, 0 for init process (whose pid is 1)

    //
    // Scheduling 
    // 
    schedule_state_t schedule_state;

    //
    // Memory management
    //
    uint32_t *translation_table;    // process translation table.
    void *memory_section;           // for now 1 memory section per process.

    //
    // Virtual file system interfaces
    //
    // TODO: handle removal here. For now descriptor is set to null
    file_descriptor_t file_descriptors[MAX_FILE_DESCRIPTOR_COUNT];
    uint32_t fd_count;

} task_t; // a renommer => process

//
//  scheduler structure
//
typedef struct {
    // process table
    task_t taskss[SCHEDULER_MAX_TASK_COUNT];
    size_t taskss_count;
    
    // current task index: 0 -> takss_count
    uint32_t current_task;

    // pid generator for new processes   
    uint32_t id_gen;
} scheduler_t;

//
//  set the current task context into the cpu
//
extern void __set_task_context(task_context_t *current_context);

//
//  global scheduler state
//
static scheduler_t _scheduler;

///////////////////////////////////////////////////////////
//                                                       //
//              Internal Scheduler Functions             //
//                                                       //
///////////////////////////////////////////////////////////

//
//  Utils: TODO: should it be moved outside
//
static void _remove_task(task_t *task)
{
    const size_t index = task - _scheduler.taskss;
    if (index >= _scheduler.taskss_count)
        kernel_fatal_error(
            "trying to remove an entry outside of process table");
  
    // delete task
    _memmove(
        &_scheduler.taskss[index],
        &_scheduler.taskss[index + 1],
        sizeof(task_t) * (_scheduler.taskss_count - (index + 1)));

    _scheduler.taskss_count--;

    // if we remove current task, update the current index
    if (index == _scheduler.current_task) {
        if (index == 0u)
            _scheduler.current_task = _scheduler.taskss_count - 1u;
        else
            _scheduler.current_task--;   
    }
}

static task_t *_scheduler_get_current_task(void)
{
    if (_scheduler.current_task >= _scheduler.taskss_count)
        kernel_fatal_error(
            "scheduluer: get current task: index is beyond task count");
    return &_scheduler.taskss[_scheduler.current_task];
}

static task_t *_scheduler_find_task_by_pid(int32_t pid)
{
    for (size_t i = 0u; i < _scheduler.taskss_count; i++) {
        task_t *task = &_scheduler.taskss[i];
        if (task->id == pid)
            return task;
    }
    return NULL;
}

static void _scheduler_task_context_init(
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

static void _scheduler_task_init(
    task_t *new_task,
    uint32_t task_id,
    uint32_t parent_task_id)
{
    _memset(new_task, 0, sizeof(task_t));

    /// Task metadata:
    new_task->id = task_id;
    new_task->parent_id = parent_task_id;
    new_task->schedule_state.status = TASK_SCHEDULED;

    /// Resource allocation:

    // 1 - allocate a memory section for the process
    new_task->memory_section = section_allocator_alloc();
    if (new_task->memory_section == NULL)
        kernel_fatal_error("failed to allocate a process memory section");
    _memset(new_task->memory_section, 0u, MMU_SECTION_SIZE);

    // 2 - allocate the process address translation table.
    new_task->translation_table = translation_table_allocator_alloc();
    if (new_task->translation_table == NULL)
        kernel_fatal_error("failed to allocate a process transaction table");
}

static task_t *_sheduler_select_next_scheduled_task(void)
{
    for (size_t i = 0; i < _scheduler.taskss_count; i++) {
        _scheduler.current_task++;
        if (_scheduler.current_task == _scheduler.taskss_count)
            _scheduler.current_task = 0u;
        task_t *current_task = _scheduler_get_current_task();
        if (current_task->schedule_state.status == TASK_SCHEDULED)
            return current_task;
    }
    
    kernel_fatal_error("no more scheduled tasks");
    return NULL;  // unreachable
}

static void _cleanup_task_resources(task_t* task)
{
    translation_table_allocator_free(task->translation_table);
    task->translation_table = NULL;
    section_allocator_free(task->memory_section);
    task->memory_section = NULL;
}

///////////////////////////////////////////////////////////
//                                                       //
//                    Public APIS                        //
//                                                       //
///////////////////////////////////////////////////////////

//
//  Scheduler initialization and startup: Called from kernel.c entrypoint
//
void scheduler_init(void)
{
    _memset(&_scheduler, 0, sizeof(scheduler_t));
}

void scheduler_start(void *init_proc)
{
    //
    // setup the init process
    // 
    _scheduler.current_task = 0u;
    _scheduler.taskss_count = 1u;
    
    // init task: pid, memory section alloc
    mini_uart_kernel_log("scheduler: initialize init process");
    const int32_t init_process_pid = ++_scheduler.id_gen;
    if (init_process_pid != INIT_TASK_PID)
        kernel_fatal_error("scheduler: unexpected init process pid");

    task_t *init_task = &_scheduler.taskss[0u];
    _scheduler_task_init(
        init_task, init_process_pid, 0u /* no parent id */);

    // setup the process translation table: 
    // - map the kernel code and data: read-only for user
    // - map the process section: read write
    translation_table_add_identity_mapping(
        init_task->translation_table,
        0x00000000u, 0x00800000u,
        MMU_L1_SECTION_AP_KERNEL_RW_USER_RO);
    translation_table_add_single_section(
        init_task->translation_table,
        init_task->memory_section,
        PROCESS_SECTION_VIRTUAL_ADDRRESS,
        MMU_L1_SECTION_AP_KERNEL_RW_USER_RW);

    // initialize task context: registers
    _scheduler_task_context_init(
        &init_task->context,
        PROCESS_STACK_VIRTUAL_ADDRRESS,
        init_proc, 0u);

    // setup stdin/stdout
    const file_descriptor_t tty_fd = vfs_file_descriptor_open("/tty", 0u, 0u);
    init_task->fd_count = 2u;
    init_task->file_descriptors[0] = tty_fd;
    init_task->file_descriptors[1] = tty_fd;

    //
    // jump to the init process
    //
    mini_uart_kernel_log("call init process !");
    mmu_set_translation_table(init_task->translation_table);
    __set_task_context(&init_task->context);
}

//
//  Scheduler context switching: Called from asm code in interupt.S
//
void scheduler_save_current_context(const task_context_t *current_context)
{
    // save the current task context context
    task_t *current_task = _scheduler_get_current_task();
    _memcpy(
        &current_task->context,
        current_context,
        sizeof(task_context_t));
}

const task_context_t *scheduler_switch_task(void)
{
    const int32_t old_pid = scheduler_cur_proc_get_id();
    task_t *next_task = _sheduler_select_next_scheduled_task();

    mini_uart_kernel_log(
        "scheduler: switch task %u => %u",
        old_pid, next_task->id);
    
    // 1 - select the process translation table
    mmu_set_translation_table(next_task->translation_table);

    // 2 - return the task context to be restored
    return &next_task->context;
}

//
//  Scheduler current task control: Called from syscall handlers.
//  current_task is scheduled when entering theses functions
//

void scheduler_cur_proc_set_syscall_status(int32_t status)
{
    _scheduler_get_current_task()->context.r0 = status;
}

void* scheduler_cur_proc_get_kernel_address(uintptr_t process_virtual_address)
{
    return mmu_translate_virtual_address(
        _scheduler_get_current_task()->translation_table,
        process_virtual_address);
}

int32_t scheduler_cur_proc_fork(void)
{
    if (_scheduler.taskss_count >= SCHEDULER_MAX_TASK_COUNT)
    {
        mini_uart_kernel_log("fork: too many tasks");
        return -1;
    }

    const task_t *current_task = _scheduler_get_current_task();
    mini_uart_kernel_log(
        "forking from task: index=%u pid=%u",
        _scheduler.current_task, current_task->id);

    // compute the new task id
    const int32_t new_task_id = ++_scheduler.id_gen;
    const uint32_t new_index = _scheduler.taskss_count++;
    mini_uart_kernel_log(
        "fork: create new task: index=%u pid=%u",
        new_index, new_task_id);

    // init task: allocate its own section and translation table
    task_t *new_task = &_scheduler.taskss[new_index];
    _scheduler_task_init(new_task, new_task_id, current_task->id);

    // fork the current task

    // fork: copy the context (cpu registers)
    _memcpy(
        &new_task->context,
        &current_task->context,
        sizeof(task_context_t));
    new_task->context.r0 = 0u; // fork should return 0 to the child process

    // fork: copy the memory section content (physically)
    _memcpy(
        new_task->memory_section,
        current_task->memory_section,
        MMU_SECTION_SIZE);

    // fork: copy oppened file descriptors
    new_task->fd_count = current_task->fd_count;
    _memcpy(
        new_task->file_descriptors,
        current_task->file_descriptors,
        sizeof(file_descriptor_t) * current_task->fd_count);

    // fork: copy the translation table, patch the virtual process section
    // todo: copy on write. Handle several sections.
    _memcpy(
        new_task->translation_table,
        current_task->translation_table,
        sizeof(uint32_t) * MMU_L1_ENTRY_COUNT);
    translation_table_add_single_section(
        new_task->translation_table,
        new_task->memory_section,
        PROCESS_SECTION_VIRTUAL_ADDRRESS,
        MMU_L1_SECTION_AP_KERNEL_RW_USER_RW);

    return new_task_id; // will be returned to the parent process
}

int32_t scheduler_cur_proc_wait_id(int32_t pid, uint32_t *wstatus)
{
    // note: only a direct child can be awaited
    task_t *current_task = _scheduler_get_current_task();

    // TODO: handle case were pid=-1: meaning any child
    if (pid == -1) {
        mini_uart_kernel_log("scheduler: wait: pid=-1 is not handled yet");
        return -1;
    }

    task_t *child_task = _scheduler_find_task_by_pid(pid);

    // the child task have already been cleaned up
    if (child_task == NULL)
        return -1;

    if (child_task->schedule_state.status == TASK_EXITED) {
        // 1st case: the child is exited
        *wstatus = child_task->schedule_state.exited.wstatus;
        _remove_task(child_task);
        return pid;
    }
    else {
        // 2nd case: the child is not exited yet
        schedule_state_t *current_state = &current_task->schedule_state;
        current_state->status = TASK_SUSPENDED;
        current_state->suspended.wait_pid = pid;
        current_state->suspended.wstatus_ref = wstatus;
        return -1;
    }

    // 3rd case: the pid is not a child process ?
    // how do we detect it ?

    // ignored return value because process was descheduled
    // return -1;
}

void scheduler_cur_proc_exit(int32_t status)
{
    task_t *current_task = _scheduler_get_current_task();

    // init must not be stopped
    if (current_task->id == 1)
        kernel_fatal_error("scheduler: init task was exited");

    mini_uart_kernel_log(
        "scheduler: exiting scheduled task %u (pid=%u)",
        _scheduler.current_task, current_task->id);

    // deschedule task
    current_task->schedule_state.status = TASK_EXITED;
    current_task->schedule_state.exited.wstatus = status;

    // clean resources
    _cleanup_task_resources(current_task);

    // each child process now belong to init
    for (size_t i = 0u; i < _scheduler.taskss_count; i++) {
        task_t *task = &_scheduler.taskss[i];
        if (task->parent_id == current_task->id)
            task->parent_id = INIT_TASK_PID;
    }

    // reshedule parent if relevant
    // task is not init: parent should exist
    task_t *parent = _scheduler_find_task_by_pid(current_task->parent_id);
    if (parent == NULL)
        kernel_fatal_error("found a task without a parent");

    if (parent->schedule_state.status == TASK_SUSPENDED &&
        parent->schedule_state.suspended.wait_pid == current_task->id) {

        // write wstatus
        *parent->schedule_state.suspended.wstatus_ref = status;

        // TODO: function reschedule_with_syscall_status()
        parent->schedule_state.status = TASK_SCHEDULED;
        parent->context.r0 = current_task->id;

        // remove task
        _remove_task(current_task);
    }
}

int32_t scheduler_cur_proc_get_id(void)
{
    return _scheduler_get_current_task()->id;
}

int32_t scheduler_cur_proc_get_parent_id(void)
{
    return _scheduler_get_current_task()->parent_id;
}

int32_t scheduler_cur_proc_add_fd(file_descriptor_t descriptor)
{
    task_t *current_task = _scheduler_get_current_task();
    if (current_task->fd_count == MAX_FILE_DESCRIPTOR_COUNT)
        kernel_fatal_error("maximum fd count was reached");
    int32_t fd = current_task->fd_count++;
    current_task->file_descriptors[fd] = descriptor;
    return fd;
}

file_descriptor_t *scheduler_cur_proc_get_fd(int32_t fd)
{
    task_t *current_task = _scheduler_get_current_task();
    if ((uint32_t)fd >= current_task->fd_count)
        return NULL;
    return &current_task->file_descriptors[fd];
}

