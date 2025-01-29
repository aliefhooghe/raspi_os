#ifndef SATAN_USERMODE_USER_INIT_H_
#define SATAN_USERMODE_USER_INIT_H_

#include <stdint.h>

void user_function1(void);
void user_function2(void);

void start_usermode(
    uintptr_t function,
    uintptr_t user_stack);

#endif
