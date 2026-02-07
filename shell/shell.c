#include "buildin_cmd.h"
#include "../lib/debug.h"
#include "../fs/dir.h"
#include "../lib/string.h"
#include "../fs/fs.h"
#include "../lib/user/syscall.h"
#include "shell.h"
#include "../lib/stdio.h"
#include "../lib/user/syscall.h"

void my_shell(void)
{
    cwd_cache[0] = '/';
    while (1)
    {
        print_prompt();
        memset(final_path, 0, MAX_PATH_LEN);
        memset(cmd_line, 0, MAX_PATH_LEN);
        readline(cmd_line, MAX_PATH_LEN);
        if (cmd_line[0] == 0)
        { // 若只键入了一个回车
            continue;
        }

        argc = -1;
        argc = cmd_parse(cmd_line, argv, ' ');
        if (argc == -1)
        {
            printf("num of arguments exceed %d\n", MAX_ARG_NR);
            continue;
        }
        if (!strcmp("ls", argv[0]))
        {
            buildin_ls(argc, argv);
        }
        else if (!strcmp("cd", argv[0]))
        {
            if (buildin_cd(argc, argv) != NULL)
            {
                memset(cwd_cache, 0, MAX_PATH_LEN);
                strcpy(cwd_cache, final_path);
            }
        }
        else if (!strcmp("pwd", argv[0]))
        {
            buildin_pwd(argc, argv);
        }
        else if (!strcmp("ps", argv[0]))
        {
            buildin_ps(argc, argv);
        }
        else if (!strcmp("clear", argv[0]))
        {
            buildin_clear(argc, argv);
        }
        else if (!strcmp("mkdir", argv[0]))
        {
            buildin_mkdir(argc, argv);
        }
        else if (!strcmp("rmdir", argv[0]))
        {
            buildin_rmdir(argc, argv);
        }
        else if (!strcmp("rm", argv[0]))
        {
            buildin_rm(argc, argv);
        }
        else
        { // 如果是外部命令,需要从磁盘上加载
            int32_t pid = fork();
            if (pid)
            { // 父进程
                int32_t status;
                int32_t child_pid = wait(&status); // 此时子进程若没有执行exit,my_shell会被阻塞,不再响应键入的命令
                if (child_pid == -1)
                { // 按理说程序正确的话不会执行到这句,fork出的进程便是shell子进程
                    panic("my_shell: no child\n");
                }
                printf("child_pid %d, it's status: %d\n", child_pid, status);
            }
            else
            { // 子进程
                make_clear_abs_path(argv[0], final_path);
                argv[0] = final_path;
                /* 先判断下文件是否存在 */
                struct stat file_stat;
                memset(&file_stat, 0, sizeof(struct stat));
                if (stat(argv[0], &file_stat) == -1)
                {
                    printf("my_shell: cannot access %s: No such file or directory\n", argv[0]);
                }
                else
                {
                    execv(argv[0], argv);
                }
            }
        }
        int32_t arg_idx = 0;
        while (arg_idx < MAX_ARG_NR)
        {
            argv[arg_idx] = NULL;
            arg_idx++;
        }
    }
    PANIC("my_shell: should not be here");
}
