#ifndef __USERPROG_FORK_H
#define __USERPROG_FORK_H

#include "stdint.h"

pid_t sys_fork(void);
void intr_exit(void);
static int32_t copy_pcb_vaddrbitmap_stack0(struct task_struct *child_thread, struct task_struct *parent_thread);
static void copy_body_stack3(struct task_struct *child_thread, struct task_struct *parent_thread, void *buf_page);
static int32_t build_child_stack(struct task_struct *child_thread);
static int32_t copy_process(struct task_struct *child_thread, struct task_struct *parent_thread);
static void update_inode_open_cnts(struct task_struct *child_thread);

#endif
