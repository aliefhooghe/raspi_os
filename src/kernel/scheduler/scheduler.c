#include <stddef.h>
#include <stdint.h>

#include "hardware/cpu.h"
#include "hardware/mini_uart.h"
#include "hardware/mmu.h"

#include "kernel.h"
#include "kernel_types.h"
#include "lib/str.h"

#include "memory/bitfield.h"
#include "memory/memory_allocator.h"
#include "memory/section_allocator.h"
#include "memory/translation_table_allocator.h"

#include "scheduler.h"
#include "task_context.h"

#include "vfs/vfs.h"
#include "vfs/inode.h"

///
//  Process memory mapping
// 
#define PROCESS_SECTION_VIRTUAL_ADDRRESS  0x00800000u  // section 0x00800000u -> 0x00900000u = 1Mb
#define PROCESS_STACK_VIRTUAL_ADDRRESS    0x00880000u  // stack at the midle of section (= a 512 kb stack)

//
//  Process structure
// 
#define MAX_FILE_DESCRIPTOR_COUNT (8u)
#define FD_BITFIELD_COUNT (MAX_FILE_DESCRIPTOR_COUNT / 8)

_Static_assert(
    MAX_FILE_DESCRIPTOR_COUNT % 8 == 0,
    "max fd must be a multiple of 8"
);

#define SCHEDULER_MAX_TASK_COUNT  (32u)
#define MAX_EXITED_CHILD_TASK     SCHEDULER_MAX_TASK_COUNT 

// Unix Standard Constants
#define INIT_PROC_ID 1u
#define FD_STDIN  0u
#define FD_STDOUT 1u
#define FD_STDERR 2u

typedef struct {
    enum {
        PROC_SCHEDULED,
        PROC_SUSPENDED,
        PROC_EXITED
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
    task_context_t context;         // saved proc context (register and processor status)
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
    file_t *files[MAX_FILE_DESCRIPTOR_COUNT];
    uint8_t files_bitfield[FD_BITFIELD_COUNT];

} process_t;

//
//  scheduler structure
//

typedef struct {
    // process table
    process_t processes[SCHEDULER_MAX_TASK_COUNT];
    size_t proc_count;
    
    // current process index: 0 -> takss_count
    uint32_t current_proc;

    // pid generator for new processes   
    uint32_t id_gen;
} scheduler_t;

//
//  set the current proc context into the cpu
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
static void _remove_process(process_t *proc)
{
    const size_t index = proc - _scheduler.processes;
    if (index >= _scheduler.proc_count)
        kernel_fatal_error(
            "trying to remove a process outside of process table");

    // delete process
    _memmove(
        &_scheduler.processes[index],
        &_scheduler.processes[index + 1],
        sizeof(process_t) * (_scheduler.proc_count - (index + 1)));

    _scheduler.proc_count--;

    // if we remove current process, update the current index
    if (index == _scheduler.current_proc) {
        if (index == 0u)
            _scheduler.current_proc = _scheduler.proc_count - 1u;
        else
            _scheduler.current_proc--;   
    }
}

static process_t *_get_current_proc(void)
{
    if (_scheduler.current_proc >= _scheduler.proc_count)
        kernel_fatal_error(
            "scheduluer: get current proc: index is beyond proc count");
    return &_scheduler.processes[_scheduler.current_proc];
}

static process_t *_find_proc_by_pid(int32_t pid)
{
    for (size_t i = 0u; i < _scheduler.proc_count; i++) {
        process_t *proc = &_scheduler.processes[i];
        if (proc->id == pid)
            return proc;
    }
    return NULL;
}

static void _proc_context_init(
    task_context_t *context,
    uintptr_t stack_address,
    void *code_entry,
    uint32_t param)
{
    context->r0 = param;
    context->sp = stack_address;
    context->lr = (uint32_t)code_entry;
    context->spsr =
        CPU_CPSR_MODE_USER |
        CPU_CPSR_DISABLE_IRQ |
        CPU_CPSR_DISABLE_FIQ;
}

static void _proc_init(
    process_t *new_proc,
    uint32_t proc_id,
    uint32_t parent_proc_id)
{
    _memset(new_proc, 0, sizeof(process_t));

    /// process metadata:
    new_proc->id = proc_id;
    new_proc->parent_id = parent_proc_id;
    new_proc->schedule_state.status = PROC_SCHEDULED;

    /// Dynamic Resource allocation:

    // 1 - allocate a memory section for the process
    new_proc->memory_section = section_allocator_alloc();
    if (new_proc->memory_section == NULL)
        kernel_fatal_error("failed to allocate a process memory section");
    _memset(new_proc->memory_section, 0u, MMU_SECTION_SIZE);

    // 2 - allocate the process address translation table.
    new_proc->translation_table = translation_table_allocator_alloc();
    if (new_proc->translation_table == NULL)
        kernel_fatal_error("failed to allocate a process transaction table");
}

int32_t _proc_add_fd(
    process_t *proc,
    file_t *file)
{
    const int32_t fd = bitfield_acquire_first(
        proc->files_bitfield, FD_BITFIELD_COUNT);
    if (fd >= 0)
        proc->files[fd] = file;
    return fd;
}

void _proc_rem_fd(
    process_t *proc,
    int32_t fd)
{
    bitfield_clear(
        proc->files_bitfield, FD_BITFIELD_COUNT, fd);
}

static file_t *_proc_get_fd(
    process_t *proc,
    int32_t fd)
{
    if (!bitfield_bit(
        proc->files_bitfield,
        FD_BITFIELD_COUNT, fd))
        return NULL;
    return proc->files[fd];
}

static process_t *_select_next_scheduled_proc(void)
{
    for (size_t i = 0; i < _scheduler.proc_count; i++) {
        _scheduler.current_proc++;
        if (_scheduler.current_proc == _scheduler.proc_count)
            _scheduler.current_proc = 0u;
        process_t *current_proc = _get_current_proc();
        if (current_proc->schedule_state.status == PROC_SCHEDULED)
            return current_proc;
    }
    
    kernel_fatal_error("no more scheduled processes");
    return NULL;  // unreachable
}

static void _cleanup_proc_resources(process_t* proc)
{
    translation_table_allocator_free(proc->translation_table);
    proc->translation_table = NULL;
    section_allocator_free(proc->memory_section);
    proc->memory_section = NULL;
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

void scheduler_start(void *init_code_entry)
{
    //
    // setup the init process
    // 
    _scheduler.current_proc = 0u;
    _scheduler.proc_count = 1u;
    
    // init process: pid, memory section alloc
    mini_uart_kernel_log("scheduler: initialize init process");
    const int32_t init_process_pid = ++_scheduler.id_gen;
    if (init_process_pid != INIT_PROC_ID)
        kernel_fatal_error("scheduler: unexpected init process pid");

    process_t *init_proc = &_scheduler.processes[0u];
    _proc_init(
        init_proc, init_process_pid, 0u /* no parent id */);

    // setup the process translation table: 
    // - map the kernel code and data: read-only for user
    // - map the process section: read write
    translation_table_add_identity_mapping(
        init_proc->translation_table,
        0x00000000u, 0x00800000u,
        MMU_L1_SECTION_AP_KERNEL_RW_USER_RO);
    translation_table_add_single_section(
        init_proc->translation_table,
        init_proc->memory_section,
        PROCESS_SECTION_VIRTUAL_ADDRRESS,
        MMU_L1_SECTION_AP_KERNEL_RW_USER_RW);

    // initialize proc context: registers
    _proc_context_init(
        &init_proc->context,
        PROCESS_STACK_VIRTUAL_ADDRRESS,
        init_code_entry, 0u);

    // create device files
    mini_uart_kernel_log("scheduler: init setup character device");
    KERNEL_ASSERT(0 == vfs_mkdir("/dev", S_IFDIR));
    KERNEL_ASSERT(0 == vfs_mknod("/dev/tty", S_IFCHR, 0u));

    // setup stdin/stdout
    mini_uart_kernel_log("scheduler: init: setup IOs");
    file_t *tty = vfs_file_open("/dev/tty", 0u, 0u);
    if (tty == NULL)
        kernel_fatal_error("failed to open tty device at /dev/tty");

    // initialize standard ios
    const int32_t stdin = _proc_add_fd(init_proc, tty);
    const int32_t stdout = _proc_add_fd(init_proc, tty);
    const int32_t stderr = _proc_add_fd(init_proc, tty);
    if (stdin != FD_STDIN ||
        stdout != FD_STDOUT ||
        stderr != FD_STDERR)
        kernel_fatal_error(
            "scheduler: init: unexpected fd num for standard IOs");

    //
    // jump to the init process
    //
    mini_uart_kernel_log("scheduler: call init !");
    mmu_set_translation_table(init_proc->translation_table);
    __set_task_context(&init_proc->context);
}

//
//  Scheduler context switching: Called from asm code in interupt.S
//
void scheduler_save_current_context(const task_context_t *current_context)
{
    // save the current task context context
    process_t *current_proc = _get_current_proc();
    _memcpy(
        &current_proc->context,
        current_context,
        sizeof(task_context_t));
}

const task_context_t *scheduler_switch_task(void)
{
    const int32_t old_pid = scheduler_cur_proc_get_id();
    process_t *next_proc = _select_next_scheduled_proc();

    mini_uart_kernel_log(
        "scheduler: switch proc %u => %u",
        old_pid, next_proc->id);
    
    // 1 - select the process translation table
    mmu_set_translation_table(next_proc->translation_table);

    // 2 - return the proc context to be restored
    return &next_proc->context;
}

//
//  Scheduler current process control: Called from syscall handlers.
//  current_proc is scheduled when entering theses functions
//

void scheduler_cur_proc_set_syscall_status(int32_t status)
{
    _get_current_proc()->context.r0 = status;
}

void* scheduler_cur_proc_get_kernel_address(uintptr_t process_virtual_address)
{
    return mmu_translate_virtual_address(
        _get_current_proc()->translation_table,
        process_virtual_address);
}

int32_t scheduler_cur_proc_fork(void)
{
    if (_scheduler.proc_count >= SCHEDULER_MAX_TASK_COUNT)
    {
       mini_uart_kernel_log("fork: too many tasks");
        return -1;
    }

    const process_t *current_proc = _get_current_proc();
    mini_uart_kernel_log(
        "forking from proc: index=%u pid=%u",
        _scheduler.current_proc, current_proc->id);

    // compute the new task id
    const int32_t new_proc_id = ++_scheduler.id_gen;
    const uint32_t new_index = _scheduler.proc_count++;
    mini_uart_kernel_log(
        "fork: create new proc: index=%u pid=%u",
        new_index, new_proc_id);

    // init task: allocate its own section and translation table
    process_t *new_proc = &_scheduler.processes[new_index];
    _proc_init(new_proc, new_proc_id, current_proc->id);

    // fork the current proc

    // copy the context (cpu registers)
    _memcpy(
        &new_proc->context,
        &current_proc->context,
        sizeof(task_context_t));
    new_proc->context.r0 = 0u; // fork should return 0 to the child process

    // copy the memory section content (physically)
    _memcpy(
        new_proc->memory_section,
        current_proc->memory_section,
        MMU_SECTION_SIZE);

    // copy opened files
    for (size_t index = 0u; index < MAX_FILE_DESCRIPTOR_COUNT; index++) {
        if (bitfield_bit(
            current_proc->files_bitfield, FD_BITFIELD_COUNT, index)) {
            // fd at index is allocated. Copy the file
            file_t *file = memory_calloc(sizeof(file_t));
            KERNEL_ASSERT(file != NULL); // horrible

            _memcpy(file, current_proc->files[index], sizeof(file_t));
            new_proc->files[index] = file;
        }
        else {
            new_proc->files[index] = NULL;
        }
    }
    _memcpy(
        new_proc->files_bitfield,
        current_proc->files_bitfield,
        FD_BITFIELD_COUNT);

    // fork: copy the translation table, patch the virtual process section
    // todo: copy on write. Handle several sections.
    _memcpy(
        new_proc->translation_table,
        current_proc->translation_table,
        sizeof(uint32_t) * MMU_L1_ENTRY_COUNT);
    translation_table_add_single_section(
        new_proc->translation_table,
        new_proc->memory_section,
        PROCESS_SECTION_VIRTUAL_ADDRRESS,
        MMU_L1_SECTION_AP_KERNEL_RW_USER_RW);

    return new_proc_id; // will be returned to the parent process
}

int32_t scheduler_cur_proc_wait_id(int32_t pid, uint32_t *wstatus)
{
    // note: only a direct child can be awaited
    process_t *current_proc = _get_current_proc();

    // TODO: handle case were pid=-1: meaning any child
    if (pid == -1) {
        mini_uart_kernel_log("scheduler: wait: pid=-1 is not handled yet");
        return -1;
    }

    process_t *child_proc = _find_proc_by_pid(pid);

    // the child task have already been cleaned up
    if (child_proc == NULL)
        return -1;

    if (child_proc->schedule_state.status == PROC_EXITED) {
        // 1st case: the child is exited
        *wstatus = child_proc->schedule_state.exited.wstatus;
        _remove_process(child_proc);
        return pid;
    }
    else {
        // 2nd case: the child is not exited yet
        schedule_state_t *current_state = &current_proc->schedule_state;
        current_state->status = PROC_SUSPENDED;
        current_state->suspended.wait_pid = pid;
        current_state->suspended.wstatus_ref = wstatus;
        return -1;
    }

    // TODO: determine what to do
    // 3rd case: the pid is not a child process ?
    // how do we detect it ?

    // ignored return value because process was descheduled
    // return -1;
}

void scheduler_cur_proc_exit(int32_t status)
{
    process_t *current_proc = _get_current_proc();

    // init must not be stopped
    if (current_proc->id == 1)
        kernel_fatal_error("scheduler: init proc was exited");

    mini_uart_kernel_log(
        "scheduler: exiting scheduled proc %u (pid=%u)",
        _scheduler.current_proc, current_proc->id);

    // deschedule proc
    current_proc->schedule_state.status = PROC_EXITED;
    current_proc->schedule_state.exited.wstatus = status;

    // clean resources
    _cleanup_proc_resources(current_proc);

    // each child process now belong to init
    for (size_t i = 0u; i < _scheduler.proc_count; i++) {
        process_t *task = &_scheduler.processes[i];
        if (task->parent_id == current_proc->id)
            task->parent_id = INIT_PROC_ID;
    }

    // reshedule parent if relevant
    // proc is not init: parent should exist
    process_t *parent = _find_proc_by_pid(current_proc->parent_id);
    if (parent == NULL)
        kernel_fatal_error("found a non init proc without a parent");

    if (parent->schedule_state.status == PROC_SUSPENDED &&
        parent->schedule_state.suspended.wait_pid == current_proc->id) {

        // write wstatus
        *parent->schedule_state.suspended.wstatus_ref = status;

        // TODO: function reschedule_with_syscall_status()
        parent->schedule_state.status = PROC_SCHEDULED;
        parent->context.r0 = current_proc->id;

        // remove process
        _remove_process(current_proc);
    }
}

int32_t scheduler_cur_proc_get_id(void)
{
    return _get_current_proc()->id;
}

int32_t scheduler_cur_proc_get_parent_id(void)
{
    return _get_current_proc()->parent_id;
}

int32_t scheduler_cur_proc_add_fd(file_t *file)
{
    process_t *current_proc = _get_current_proc();
    return _proc_add_fd(current_proc, file);
}

void scheduler_cur_proc_rem_fd(int32_t fd)
{
    process_t *current_proc = _get_current_proc();
    _proc_rem_fd(current_proc, fd);
}

file_t *scheduler_cur_proc_get_fd(int32_t fd)
{
    process_t *current_proc = _get_current_proc();
    return _proc_get_fd(current_proc, fd);
}

