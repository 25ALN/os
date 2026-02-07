#include "console.h"
#include "../kernel/print.h"
#include "../thread/sync.h"

struct lock console_lock;

//初始化终端
void console_init()
{
    lock_init(&console_lock);
}

//获取终端
void console_acquire()
{
    lock_acquire(&console_lock);
}

//释放终端
void console_release()
{
    lock_release(&console_lock);
}

//在终端中输出字符串
void console_put_str(char* str)
{
    console_acquire();
    put_str(str);
    console_release();
}

//在终端中输出16进制整数
void console_put_int(uint32_t num)
{
    console_acquire();
    put_int(num);
    console_release();
}

//在终端中输出字符
void console_put_char(uint8_t chr)
{
    console_acquire();
    put_char(chr);
    console_release();
}
