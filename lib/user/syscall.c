#include "syscall.h"
#include "../thread/thread.h"

#define _syscall0(NUMBER) ({ \
    int retval;		\
    asm volatile ("int $0x80" : "=a"(retval) : "a"(NUMBER) : "memory"); retval; \
    })

#define _syscall1(NUMBER, ARG1) ({ \
    int retval;		\
    asm volatile ("int $0x80" : "=a"(retval) : "a"(NUMBER) , "b"(ARG1) : "memory"); retval; \
    })
    
#define _syscall2(NUMBER, ARG1, ARG2) ({ \
    int retval;		\
    asm volatile ("int $0x80" : "=a"(retval) : "a"(NUMBER) , "b"(ARG1) , "c"(ARG2): "memory"); retval; \
    })

#define _syscall3(NUMBER, ARG1, ARG2, ARG3) ({ \
    int retval;		\
    asm volatile ("int $0x80" : "=a"(retval) : "a"(NUMBER) , "b"(ARG1) , "c"(ARG2), "d"(ARG3): "memory"); retval; \
    })

uint32_t getpid(void)
{
    return _syscall0(SYS_GETPID);
}

void* malloc(uint32_t size){
    return  (void *)_syscall1(SYS_MALLOC,size);
}

void free(void *ptr){
    _syscall1(SYS_FREE,ptr);
}

uint32_t write(int fd,const void* buf,int count)
{
    return _syscall3(SYS_WRITE,fd,buf,count);
}


/* 派生子进程,返回子进程pid */
pid_t fork(void)
{
    return _syscall0(SYS_FORK);
}

/* 从文件描述符fd中读取count个字节到buf */
int32_t read(int32_t fd, void *buf, uint32_t count)
{
    return _syscall3(SYS_READ, fd, buf, count);
}

/* 输出一个字符 */
void putchar(char char_asci)
{
    _syscall1(SYS_PUTCHAR, char_asci);
}

/* 清空屏幕 */
void clear(void)
{
    _syscall0(SYS_CLEAR);
}

int execv(const char *pathname, char **argv)
{
    return _syscall2(SYS_EXECV, pathname, argv);
}

/* 以状态status退出 */
void exit(int32_t status)
{
    _syscall1(SYS_EXIT, status);
}

/* 等待子进程,子进程状态存储到status */
pid_t wait(int32_t *status)
{
    return _syscall1(SYS_WAIT, status);
}

/* 生成管道,pipefd[0]负责读入管道,pipefd[1]负责写入管道 */
int32_t pipe(int32_t pipefd[2])
{
    return _syscall1(SYS_PIPE, pipefd);
}

/* 将文件描述符old_local_fd重定向到new_local_fd */
void fd_redirect(uint32_t old_local_fd, uint32_t new_local_fd)
{
    _syscall2(SYS_FD_REDIRECT, old_local_fd, new_local_fd);
}

/* 显示系统支持的命令 */
void help(void)
{
    _syscall0(SYS_HELP);
}
