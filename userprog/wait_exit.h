#ifndef __USERPROG_WAITEXIT_H
#define __USERPROG_WAITEXIT_H

#include "../kernel/stdint.h"
static bool find_child(struct list_elem *pelem, int32_t ppid);
static bool find_hanging_child(struct list_elem* pelem, int32_t ppid);
static bool init_adopt_a_child(struct list_elem *pelem, int32_t pid);
pid_t sys_wait(int32_t *status);


#endif
