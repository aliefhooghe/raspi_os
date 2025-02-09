#ifndef SATAN_SCHEDULER_H_
#define SATAN_SCHEDULER_H_

#include <stdint.h>

#include "task_context.h"
#include "vfs/vfs.h"

#define SCHEDULER_MAX_TASK_COUNT 0x80u


typedef struct {
    task_context_t context;     // saved task context (register and processor status)
    int32_t id;                 // process id: pid

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


void scheduler_init(void);
void scheduler_start(void);

// called from asm code in interupt.S
const task_context_t *scheduler_switch_task(const task_context_t *current_context);

int32_t scheduler_add_task(uintptr_t proc_address, void* stack_address, uint32_t param);

// current process getters
void scheduler_cur_proc_exit(void);
int32_t scheduler_cur_proc_get_id(void);
file_descriptor_t *scheduler_cur_proc_get_fd(int32_t fd);

#endif
