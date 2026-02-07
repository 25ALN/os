#include "init.h"
#include "print.h"
#include "interrupt.h"
#include "../device/timer.h"
#include "memory.h"
#include "../thread/thread.h"
#include "../device/console.h"
#include "../device/keyboard.h"
#include "../userprog/tss.h"
#include "../userprog/syscall_init.h"
#include "../device/ide.h"
#include "../fs/fs.h"

//初始化各个模块

void init_all(){
    put_str("init all things\n");
    idt_init();                     //初始化中断
    timer_init();                   //时钟中断初始化
    mem_init();
    thread_init();
    console_init();
    keyboard_init();                //键盘中断初始化
    tss_init();                     
    syscall_init();
    ide_init();
    filesys_init();
}