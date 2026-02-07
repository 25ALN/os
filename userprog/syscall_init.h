#ifndef __USERPROG_SYSCALL_INIT_H
#define __USERPROG_SYSCALL_INIT_H
#include "../kernel/stdint.h"

uint32_t sys_getpid(void);
void syscall_init(void);

#endif

