#include <stddef.h>
#include <stdint.h>

#include "elf_loader/elf_loader.h"

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
// Process memory mapping
//
// For now there is one 1Mb section mapped from virtual address 0x00800000u
// -         | <start>     | <end>       | <size>            | 
// - stack   | 0x00800000u | 0x00840000u |  0x40000 (256 Kb) |
// - program | 0x00840000u | 0x00880000u |  0x40000 (256 Kb) |
// - heap    | 0x00880000u | 0x00900000u |  0x80000 (512 Kb  |
// ----------------------------------------------------------
//   section | 0x00800000u | 0x00900000u | 0x100000 (1Mb)
// 
#define PROCESS_SECTION_VIRTUAL_ADDRRESS  0x00800000u  // section 0x00800000u -> 0x00900000u = 1Mb
#define PROCESS_STACK_VIRTUAL_ADDRRESS    0x0083FFFCu  // stack is going descending. 4 bytes aligned
#define PROCESS_PROGRAM_START_VADRR       0x00840000u
#define PROCESS_PROGRAM_END_VADRR         0x00880000u

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
    
    // current process index: 0 -> proc_count
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
//

static void _dump_task_context(task_context_t *context)
{
    mini_uart_kernel_log(
        "\n"
        "  r0     = %x\n"  
        "  lr_usr = %x\n"
        "  sp     = %x\n"
        "  spsr   = %x\n"
        "  r1     = %x\n"
        "  r2     = %x\n"
        "  r3     = %x\n"
        "  r4     = %x\n"
        "  r5     = %x\n"
        "  r6     = %x\n"
        "  r7     = %x\n"
        "  r8     = %x\n"
        "  r9     = %x\n"
        "  r10    = %x\n"
        "  r11    = %x\n"
        "  r12    = %x\n"
        "  lr_svc = %x",
        context->r0,
        context->lr_usr,  
        context->sp,
        context->spsr,
        context->r1,
        context->r2,
        context->r3,
        context->r4,
        context->r5,
        context->r6,
        context->r7,
        context->r8,
        context->r9,
        context->r10,
        context->r11,
        context->r12,
        context->lr_svc
    );
}

static void _remove_process(process_t *proc)
{
    const size_t index = proc - _scheduler.processes;
    if (index >= _scheduler.proc_count)
        kernel_fatal_error(
            "trying to remove a process outside of process table");

    mini_uart_kernel_log(
        "scheduler: remove proc index=%u, pid=%u",
        index, proc->id);
    
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
    uint32_t entry,
    uint32_t param)
{
    context->r0 = param;
    context->sp = stack_address;
    context->lr_svc = entry;
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
        (uint32_t)init_code_entry, 0u);

    // setup stdin/stdout
    mini_uart_kernel_log("scheduler: init: setup IOs");
    file_t *tty = vfs_file_open("/dev/tty", 0u, 0u);
    if (tty == NULL)
        kernel_fatal_error(
            "scheduler: init: failed to open tty device at /dev/tty");

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
    mini_uart_kernel_log(
        "save current context: pid=%u",
        current_proc->id);
    _dump_task_context(&current_proc->context);
}

const task_context_t *scheduler_switch_task(void)
{
    const int32_t old_pid = _scheduler.processes[_scheduler.current_proc].id;
    process_t *next_proc = _select_next_scheduled_proc();

    mini_uart_kernel_log(
        "scheduler: switch proc %u => %u (section=%x). Restore context:",
        old_pid, next_proc->id, next_proc->memory_section);
    _dump_task_context(&next_proc->context);

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
       mini_uart_kernel_log("fork: too many processes");
        return -1;
    }

    const process_t *current_proc = _get_current_proc();
    mini_uart_kernel_log(
        "forking from proc: index=%u pid=%u",
        _scheduler.current_proc, current_proc->id);

    // compute the new process id
    const int32_t new_proc_id = ++_scheduler.id_gen;
    const uint32_t new_index = _scheduler.proc_count++;
    mini_uart_kernel_log(
        "fork: create new proc: index=%u pid=%u",
        new_index, new_proc_id);

    // init process: allocate its own section and translation table
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

int32_t scheduler_cur_proc_exec(const char *path)
{
    process_t *current_proc = _get_current_proc();
    mini_uart_kernel_log(
        "scheduler: exec: path=%s, index=%u pid=%u, section=%x",
        path, _scheduler.current_proc, current_proc->id, current_proc->memory_section);
    elf32_file_t elf_file;
    const int32_t status = elf32_open(path, &elf_file);
    if (status < 0)
        return status;

    // 1 - replace memory image

    // zero initialize section for security
    _memset(
        current_proc->memory_section,
        0,
        MMU_SECTION_SIZE);

    elf32_program_header_iterator_t it =
        elf32_init_program_header_iterator(&elf_file);

    int32_t st;
    elf32_program_header_t phdr;
    while (1 == (st = elf32_program_header_iterator_read_next(&it, &phdr))) {
        // only handle PT_LOAD sections
        mini_uart_kernel_log(
            "scheduler: exec: read section: type=%x",
            phdr.type);
        if (phdr.type != PT_LOAD || phdr.vaddress == 0u) {
            // ignore non PT_LOAD: not to be loaded
            // ignore vaddr=0 => they contains elf headers
            continue;
        }

        // load section to process image memory
        uint8_t *const dst_kaddr = mmu_translate_virtual_address(
            current_proc->translation_table, phdr.vaddress);
        mini_uart_kernel_log(
            "scheduler: load PT_LOAD section to %x=>%x (paddr=%x) (file_sz=%u/mem_sz=%u)",
            phdr.vaddress, phdr.vaddress + phdr.mem_size, dst_kaddr,
            phdr.file_size, phdr.mem_size);

        const off_t seek_st = vfs_file_lseek(
            elf_file.file, phdr.offset, SEEK_SET);
        KERNEL_ASSERT(seek_st == (off_t)phdr.offset);

        const ssize_t read_sz = vfs_file_read(
            elf_file.file, dst_kaddr, phdr.file_size);
        KERNEL_ASSERT(read_sz == (ssize_t)phdr.file_size);

        // pad with zero if required
        // TODO: not usefull because we have zeroed the section before
        if (phdr.mem_size > phdr.file_size) {
            const size_t padd_sz = phdr.mem_size - phdr.file_size;
            mini_uart_kernel_log(
                "scheduler: exec: pad %u zeros",
                padd_sz);
            _memset(
                dst_kaddr + phdr.file_size, 0, padd_sz);
        }
    }

    // if (st < 0) // hard to handle
    //     return st;
    KERNEL_ASSERT(st == 0);

    // __invalidate_instruction_cache();
    // asm volatile ("" ::: "memory"); // memory barrier

    // 2 - reset cpu context
    const uint32_t entry = elf_file.header.entry;
    KERNEL_ASSERT(entry != 0u);
    mini_uart_kernel_log(
        "scheduler: exec: set entry=%x",
        entry);
    KERNEL_ASSERT(
        entry >= PROCESS_PROGRAM_START_VADRR &&
        entry <= PROCESS_PROGRAM_END_VADRR);

    _proc_context_init(
        &current_proc->context,
        PROCESS_STACK_VIRTUAL_ADDRRESS,
        entry, 0u);

    // current_proc->schedule_state.status = PROC_EXITED;

    const int32_t close_status = elf32_close(&elf_file);
    KERNEL_ASSERT(close_status == 0);

    return 0;
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

    // the child process have already been cleaned up
    if (child_proc == NULL)
        return -1;

    if (child_proc->schedule_state.status == PROC_EXITED) {
        mini_uart_kernel_log(
            "scheduler: waiting %u: child is already exited",
            pid);
        // 1st case: the child is exited
        *wstatus = child_proc->schedule_state.exited.wstatus;
        _remove_process(child_proc);
        return pid;
    }
    else {
        mini_uart_kernel_log(
            "scheduler: waiting %u: child is not exited yet",
            pid);
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
        "scheduler: exiting scheduled proc index=%u pid=%u",
        _scheduler.current_proc, current_proc->id);

    // deschedule proc
    current_proc->schedule_state.status = PROC_EXITED;
    current_proc->schedule_state.exited.wstatus = status;

    // clean resources
    _cleanup_proc_resources(current_proc);

    // each child process now belong to init
    for (size_t i = 0u; i < _scheduler.proc_count; i++) {
        process_t *proc = &_scheduler.processes[i];
        if (proc->parent_id == current_proc->id) {
            mini_uart_kernel_log(
                "scheduler: exit: child proc index=%u pid=%u become a zombie",
                i, proc->id);
            proc->parent_id = INIT_PROC_ID;
        }
    }

    // reshedule parent if relevant
    // proc is not init: parent should exist
    process_t *parent = _find_proc_by_pid(current_proc->parent_id);
    if (parent == NULL)
        kernel_fatal_error("found a non init proc without a parent");

    if (parent->schedule_state.status == PROC_SUSPENDED &&
        parent->schedule_state.suspended.wait_pid == current_proc->id) {
        mini_uart_kernel_log(
            "scheduler: exit: parent pid=%u was waiting proc. reschedule parent",
            parent->id);

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
