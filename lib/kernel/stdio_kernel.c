#include "stdio_kernel.h"
#include "../lib/../kernel/stdint.h"
#include "../stdio.h"
#include "../lib/../device/console.h"

void printk(const char* format, ...)
{
    va_list args;
    va_start(args,format);
    char buf[1024] = {0};
    vsprintf(buf,format,args);
    va_end(args);
    console_put_str(buf);
}
