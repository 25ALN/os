#include "thread.h"   //函数声明 各种结构体
#include "../kernel/stdint.h"   //前缀
#include "../lib/string.h"   //memset
#include "../kernel/global.h"
#include "../kernel/memory.h"   //分配页需要
#include "../kernel/interrupt.h"
#include "../kernel/print.h"
#include "../lib/debug.h"
#include "../userprog/process.h"
#include "sync.h"

#define PG_SIZE 4096

struct task_struct* main_thread;                        //主线程main_thread的pcb
struct list thread_ready_list;			  //就绪队列
struct list thread_all_list;				  //总线程队列
static struct list_elem* thread_tag;      //保存队列中的线程节点 
struct lock pid_lock;
struct task_struct* idle_thread;

extern void switch_to(struct task_struct* cur,struct task_struct* next);
extern void init(void);


pid_t allocate_pid(void)
{
    static pid_t next_pid = 0;			  //约等于全局变量 全局性+可修改性
    lock_acquire(&pid_lock);
    ++next_pid;
    lock_release(&pid_lock);
    return next_pid;
}

// 获取 pcb 指针
// 线程所在的esp 是在 get得到的那一页内存 pcb页上下浮动 但是我们的pcb的最起始位置是整数的 除去后面的12位
//对前面的取 & 则可以得到地址所在地
struct task_struct* running_thread(void)
{
    uint32_t esp;
    asm ("mov %%esp,%0" : "=g"(esp));
    return (struct task_struct*)(esp & 0xfffff000);
}

void kernel_thread(thread_func* function,void* func_arg)
{
    intr_enable();                                      //开启中断防止后面无法切换线程
    function(func_arg);
}

void thread_create(struct task_struct* pthread,thread_func function,void* func_arg)
{
    pthread->self_kstack -= sizeof(struct intr_stack);  //减去中断栈的空间
    pthread->self_kstack -= sizeof(struct thread_stack);
    struct thread_stack* kthread_stack = (struct thread_stack*)pthread->self_kstack; 
    kthread_stack->eip = kernel_thread;                 //地址为kernel_thread 由kernel_thread 执行function
    kthread_stack->function = function;
    kthread_stack->func_arg = func_arg;
    kthread_stack->ebp = kthread_stack->ebx = kthread_stack->ebx = kthread_stack->esi = 0; //初始化一下
    return;
}

void init_thread(struct task_struct* pthread,char* name,int prio)
{
    memset(pthread,0,sizeof(*pthread)); //pcb位置清0
    pthread->pid=allocate_pid();
    strcpy(pthread->name,name);
    
    if(pthread == main_thread)
    	pthread->status = TASK_RUNNING;                              //我们的主线程肯定是在运行的
    else
    	pthread->status = TASK_READY;					//放到就绪队列里面
    	
    pthread->self_kstack = (uint32_t*)((uint32_t)pthread + PG_SIZE); //刚开始的位置是最低位置 栈顶位置+一页得最顶部
                                                                     //后面还要对这个值进行修改                                    
    pthread->priority = prio;                                        
    pthread->ticks = prio;                                           //和特权级 相同的时间片
    pthread->elapsed_ticks = 0;
    pthread->pgdir = NULL;                                           //线程没有单独的地址
    pthread->fd_table[0] = 0;
    pthread->fd_table[1] = 1;
    pthread->fd_table[2] = 2;                                       
    uint8_t fd_idx = 3;
    while(fd_idx < MAX_FILES_OPEN_PER_PROC){
        pthread->fd_table[fd_idx] = -1;
        fd_idx++;
    }
    pthread->cwd_inode_nr = 0;					//默认工作路径在根目录
    pthread->stack_magic = 0x19870916;                               //设置的魔数 检测是否越界限
}
struct task_struct* thread_start(char* name,int prio,thread_func function,void* func_arg)
{
    struct task_struct* thread = get_kernel_pages(1);
    init_thread(thread,name,prio);
    thread_create(thread,function,func_arg);
    //通过ret指令进入，原因：1、函数地址与参数可以放入栈中统一管理；2、ret指令可以直接从栈顶取地址跳入执行
    ASSERT(!elem_find(&thread_ready_list,&thread->general_tag));     //之前不应该在就绪队列里面
    list_append(&thread_ready_list,&thread->general_tag);
    
    ASSERT(!elem_find(&thread_all_list,&thread->all_list_tag));
    list_append(&thread_all_list,&thread->all_list_tag);
     
    return thread;
}

void make_main_thread(void)
{		
    main_thread = running_thread();					//得到main_thread 的pcb指针
    init_thread(main_thread,"main",31);				
    
    ASSERT(!elem_find(&thread_all_list,&main_thread->all_list_tag));
    list_append(&thread_all_list,&main_thread->all_list_tag);
    
}

void schedule(void)
{
    ASSERT(intr_get_status() == INTR_OFF);
    struct task_struct* cur = running_thread(); 
    if (cur->status == TASK_RUNNING) { // 若此线程只是cpu时间片到了,将其加入到就绪队列尾
        ASSERT(!elem_find(&thread_ready_list, &cur->general_tag));
        list_append(&thread_ready_list, &cur->general_tag);
        cur->ticks = cur->priority;     // 重新将当前线程的ticks再重置为其priority;
        cur->status = TASK_READY;
    } 
    else { 
        /* 若此线程需要某事件发生后才能继续上cpu运行, 
        不需要将其加入队列,因为当前线程不在就绪队列中。*/
    }

    //就绪队列中没有任务就唤醒idle，不然会出现list为空的错误
    if (list_empty(&thread_ready_list)) {
      thread_unblock(idle_thread);
    }

    ASSERT(!list_empty(&thread_ready_list));
    thread_tag = NULL;	  // thread_tag清空
    /* 将thread_ready_list队列中的第一个就绪线程弹出,准备将其调度上cpu. */
    thread_tag = list_pop(&thread_ready_list);   
    struct task_struct* next = elem2entry(struct task_struct, general_tag, thread_tag);
    next->status = TASK_RUNNING;
    process_activate(next);
    switch_to(cur, next);
}

void thread_block(enum task_status stat)
{
    //设置block状态的参数必须是下面三个以下的
    ASSERT(((stat == TASK_BLOCKED) || (stat == TASK_WAITING) || stat == TASK_HANGING));
    
    enum intr_status old_status = intr_disable();			 //关中断
    struct task_struct* cur_thread = running_thread();		 
    cur_thread->status = stat;					 //把状态重新设置
    
    //调度器切换其他进程了 而且由于status不是running 不会再被放到就绪队列中
    schedule();	
    				
    //被切换回来之后再进行的指令了
    intr_set_status(old_status);
}

//由锁拥有者来执行的 把原来自我阻塞的线程重新放到队列中
void thread_unblock(struct task_struct* pthread)
{
    enum intr_status old_status = intr_disable();
    ASSERT(((pthread->status == TASK_BLOCKED) || (pthread->status == TASK_WAITING) || (pthread->status == TASK_HANGING)));
    if(pthread->status != TASK_READY)
    {
    	//被阻塞线程 不应该存在于就绪队列中）
    	ASSERT(!elem_find(&thread_ready_list,&pthread->general_tag));
    	if(elem_find(&thread_ready_list,&pthread->general_tag))
    	    PANIC("thread_unblock: blocked thread in ready_list\n"); //debug.h中定义过
    	
    	//让阻塞了很久的任务放在就绪队列最前面
    	list_push(&thread_ready_list,&pthread->general_tag);
    	
    	//状态改为就绪态
    	pthread->status = TASK_READY;
    }
    intr_set_status(old_status);
}


/* 系统空闲时运行的线程 */
void idle() {
   while(1) {
      thread_block(TASK_BLOCKED);     
      //执行hlt时必须要保证目前处在开中断的情况下
      asm volatile ("sti; hlt" : : : "memory");
   }
}

/* 初始化线程环境 */
void thread_init(void) {
   put_str("thread_init start\n");
   list_init(&thread_ready_list);
   list_init(&thread_all_list);
   lock_init(&pid_lock);
   /* 先创建第一个用户进程:init */
   process_execute(init, "init");         // 放在第一个初始化,这是第一个进程,init进程的pid为1
/* 将当前main函数创建为线程 */
   make_main_thread();
      /* 创建idle线程 */
   idle_thread = thread_start("idle", 10, idle, NULL);
   put_str("thread_init done\n");
}


/* 主动让出cpu,换其它线程运行 */
void thread_yield(void) {
   struct task_struct* cur = running_thread();   
   enum intr_status old_status = intr_disable();
   ASSERT(!elem_find(&thread_ready_list, &cur->general_tag));
   list_append(&thread_ready_list, &cur->general_tag);
   cur->status = TASK_READY;
   schedule();
   intr_set_status(old_status);
}

/* fork进程时为其分配pid,因为allocate_pid已经是静态的,别的文件无法调用.
不想改变函数定义了,故定义fork_pid函数来封装一下。*/
pid_t fork_pid(void)
{
    return allocate_pid();
}

/* 回收thread_over的pcb和页表,并将其从调度队列中去除 */
void thread_exit(struct task_struct *thread_over, bool need_schedule)
{
    /* 要保证schedule在关中断情况下调用 */
    intr_disable();
    thread_over->status = TASK_DIED;

    /* 如果thread_over不是当前线程,就有可能还在就绪队列中,将其从中删除 */
    if (elem_find(&thread_ready_list, &thread_over->general_tag))
    {
        list_remove(&thread_over->general_tag);
    }
    if (thread_over->pgdir)
    { // 如是进程,回收进程的页表
        mfree_page(PF_KERNEL, thread_over->pgdir, 1);
    }

    /* 从all_thread_list中去掉此任务 */
    list_remove(&thread_over->all_list_tag);

    /* 回收pcb所在的页,主线程的pcb不在堆中,跨过 */
    if (thread_over != main_thread)
    {
        mfree_page(PF_KERNEL, thread_over, 1);
    }

    /* 归还pid */
    release_pid(thread_over->pid);

    /* 如果需要下一轮调度则主动调用schedule */
    if (need_schedule)
    {
        schedule();
        PANIC("thread_exit: should not be here\n");
    }
}

/* 比对任务的pid */
static bool pid_check(struct list_elem *pelem, int32_t pid)
{
    struct task_struct *pthread = elem2entry(struct task_struct, all_list_tag, pelem);
    if (pthread->pid == pid)
    {
        return true;
    }
    return false;
}

/* 根据pid找pcb,若找到则返回该pcb,否则返回NULL */
struct task_struct *pid2thread(int32_t pid)
{
    struct list_elem *pelem = list_traversal(&thread_all_list, pid_check, pid);
    if (pelem == NULL)
    {
        return NULL;
    }
    struct task_struct *thread = elem2entry(struct task_struct, all_list_tag, pelem);
    return thread;
}
