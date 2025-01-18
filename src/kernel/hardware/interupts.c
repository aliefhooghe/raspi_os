
#include "interupts.h"

void software_interupt_handler(
    uint32_t syscall_num,
    uint32_t program_status,
    uint32_t arg0, uint32_t arg1)
{
    (void)syscall_num;
    (void)program_status;
    (void)arg0;
    (void)arg1;
}
