#include <memlayout.h>
#include "process.h"
#include "lock.h"
#include "pagetable.h"
#include "elf.h"
#include <../user/stdlib.h>
extern const char binary_putc_start;
cpu_t cpus[NCPU];
process_t proc[NPROC];
thread_t *running[NCPU];
struct list_head sched_list[NCPU];
struct lock pidlock, tidlock, schedlock;
int _pid, _tid;
int nextpid = 1;
cpu_t* mycpu() {
    int id = cpuid();
    cpu_t *c = &cpus[id];
    return c;
}
void push_off(){
    int old = intr_get();

    intr_off();
    if(mycpu()->noff == 0)
        mycpu()->intena = old;
    mycpu()->noff += 1;
}

void pop_off(){
    cpu_t *c = mycpu();
    c->noff -= 1;
    if(c->noff == 0 && c->intena)
        intr_on();
}
thread_t* mythr(){
    return container_of(&(sched_list[cpuid()]), thread_t, sched_list_thread_node);
}
process_t* myproc() {
    push_off();
    cpu_t *c = mycpu();
    process_t *p = c->thr->proc;
    pop_off();
    return p;
}
// 将ELF文件映射到给定页表的地址空间，返回pc的数值
// 关于 ELF 文件，请参考：https://docs.oracle.com/cd/E19683-01/816-1386/chapter6-83432/index.html
static uint64 load_binary(pagetable_t *target_page_table, const char *bin){
	struct elf_file *elf;
    int i;
    uint64 seg_sz, p_vaddr, seg_map_sz;
	elf = elf_parse_file(bin);
	
	/* load each segment in the elf binary */
	for (i = 0; i < elf->header.e_phnum; ++i) {
		if (elf->p_headers[i].p_type == PT_LOAD) {
            // 根据 ELF 文件格式做段映射
            // 从ELF中获得这一段的段大小
            seg_sz = elf->p_headers[i].p_memsz;
            // 对应段的在内存中的虚拟地址
            p_vaddr = elf->p_headers[i].p_vaddr;
            // 对映射大小做页对齐
			seg_map_sz = ROUNDUP(seg_sz + p_vaddr, PGSIZE) - PGROUNDDOWN(p_vaddr);
            // 接下来代码的期望目的：将程序代码映射/复制到对应的内存空间
            // 一种可能的实现如下：
            /* 
             * 在 target_page_table 中分配一块大小
             * 通过 memcpy 将某一段复制进入这一块空间
             * 页表映射修改
             */
		}
	}
	/* PC: the entry point */
	return elf->header.e_entry;
}

/* 分配一个进程，需要至少完成以下目标：
 * 
 * 分配一个主线程
 * 创建一张进程页表
 * 分配pid、tid
 * 设置初始化线程上下文
 * 设置初始化线程返回地址寄存器ra，栈寄存器sp
 * 
 * 这个函数传入参数为一个二进制的代码和一个线程指针(具体传入规则可以自己修改)
 * 此外程序首次进入用户态之前，应该设置好trap处理向量为usertrap（或者你自定义的）
 *//*
void procinit(void){
    process_t *p;

    lock_init(&pidlock);
    lock_init(&schedlock);
    for(p = proc; p < &proc[NPROC]; p++) {
        lock_init(&p->lock);
        p->kstack = KSTACK((int) (p - proc));
    }
}
void proc_freepagetable(pagetable_t pagetable, uint64 sz){
    pt_unmap_addrs(pagetable, TRAMPOLINE);
    pt_unmap_addrs(pagetable, TRAPFRAME);
    for(uint64 va=0;  va < sz; va += PGSIZE)
        pt_unmap_addrs(pagetable, va);
}
static void freeproc(process_t *p)
{
    if(p->trapframe)
        mm_kfree((void*)p->trapframe);
    p->trapframe = 0;
    if(p->pagetable)
        proc_freepagetable(p->pagetable, p->sz);
    p->pagetable = 0;
    p->sz = 0;
    p->pid = 0;
    p->parent = 0;
    p->chan = 0;
    p->xstate = 0;
    p->state = UNUSED;
}
int allocpid() {
    int pid;
    acquire(&pidlock);
    pid = nextpid;
    nextpid = nextpid + 1;
    release(&pidlock);

    return pid;
}

process_t *alloc_proc(const char* bin, thread_t *thr){
    process_t *p;
    for(p = proc; p < &proc[NPROC]; p++) {
        acquire(&p->lock);
        if(p->state == UNUSED) {
            p->pid = allocpid();
            p->state = IDLE;
            // Allocate a trapframe page.
            if((p->trapframe = (struct trapframe *)mm_kalloc()) == 0){
                freeproc(p);
                release(&p->lock);
                return 0;
            }

            // An empty user page table.
            p->pagetable = proc_pagetable(p);
            if(p->pagetable == 0){
                freeproc(p);
                release(&p->lock);
                return 0;
            }

            // Set up new context to start executing at forkret,
            // which returns to user space.
            memset(&p->context, 0, sizeof(p->context));
            //p->context.ra = (uint64)forkret;
            p->context.sp = p->kstack + PGSIZE;

            return p;
        } else {
            release(&p->lock);
        }
    }
    return 0;
}
*/
bool load_thread(file_type_t type){
    if(type == PUTC){
        thread_t *t = NULL;
        //process_t *p = alloc_proc(&binary_putc_start, t);
        if(!t) return false;
        sched_enqueue(t);
    } else {
        BUG("Not supported");
    }

}

// sched_enqueue和sched_dequeue的主要任务是加入一个任务到队列中和删除一个任务
// 这两个函数的展示了如何使用list.h中可的函数（加入、删除、判断空、取元素）
// 具体可以参考：Stackoverflow上的回答
// https://stackoverflow.com/questions/15832301/understanding-container-of-macro-in-the-linux-kernel
void sched_enqueue(thread_t *target_thread){
    if(target_thread->thread_state == RUNNING) BUG("Running Thread cannot be scheduled.");
    list_add(&target_thread->sched_list_thread_node, &(sched_list[cpuid()]));
}

thread_t *sched_dequeue(){
    if(list_empty(&(sched_list[cpuid()]))) BUG("Scheduler List is empty");
    thread_t *head = container_of(&(sched_list[cpuid()]), thread_t, sched_list_thread_node);
    list_del(&head->sched_list_thread_node);
    return head;
}

bool sched_empty(){
    return list_empty(&(sched_list[cpuid()]));
}

// 开始运行某个特定的函数
void thread_run(thread_t *target){

}

// sched_start函数启动调度，按照调度的队列开始运行。
void sched_start(){
    while(1){
        if(sched_empty()) BUG("Scheduler list empty, no app loaded");
        thread_t *next = sched_dequeue();
        thread_run(next);
    }
}

void sched_init(){
    // 初始化调度队列锁
    lock_init(&schedlock);
    // 初始化队列头
    init_list_head(&(sched_list[cpuid()]));
}

void proc_init(){
    // 初始化pid、tid锁
    lock_init(&pidlock);
    lock_init(&tidlock);
    // 接下来代码期望的目的：映射第一个用户线程并且插入调度队列
    if(!load_thread(PUTC)) BUG("Load failed");
}
/*
void sched(){
    int intena;
    process_t *p = myproc();
    intena = mycpu()->intena;
    swtch(&p->context, &mycpu()->context);
    mycpu()->intena = intena;
}


void scheduler(){
    process_t *p;
    struct cpu *c = mycpu();

    c->proc = 0;
    for(;;){
        // Avoid deadlock by ensuring that devices can interrupt.
        intr_on();

        int found = 0;
        for(p = proc; p < &proc[NPROC]; p++) {
            acquire(&p->lock);
            if(p->state == RUNNABLE) {
                // Switch to chosen process.  It is the process's job
                // to release its lock and then reacquire it
                // before jumping back to us.
                p->state = RUNNING;
                c->proc = p;
                swtch(&c->context, &p->context);

                // Process is done running for now.
                // It should have changed its p->state before coming back.
                c->proc = 0;

                found = 1;
            }
            release(&p->lock);
        }
        if(found == 0) {
            intr_on();
            asm volatile("wfi");
        }
    }
}
void sleep(void *chan, struct spinlock *lk){
    process_t *p = myproc();
    if(lk != &p->lock){
        acquire(&p->lock);
        release(lk);
    }
    p->chan = chan;
    p->state = SLEEPING;
    sched();
    p->chan = 0;
    if(lk != &p->lock){
        release(&p->lock);
        acquire(lk);
    }
}
void wakeup(void *chan){
    process_t *p;
    for(p = proc; p < &proc[NPROC]; p++) {
        acquire(&p->lock);
        if(p->state == SLEEPING && p->chan == chan) {
            p->state = RUNNABLE;
        }
        release(&p->lock);
    }
}

static void wakeup1(process_t *p){
    if(!holding_lock(&p->lock))
        sys_exit(-1);
    if(p->chan == p && p->state == SLEEPING) {
        p->state = RUNNABLE;
    }
}
int wait(uint64 addr){
    process_t *np;
    int havekids, pid;
    struct proc *p = myproc();

    acquire(&p->lock);

    for(;;){
        havekids = 0;
        for(np = proc; np < &proc[NPROC]; np++){
            if(np->parent == p){
                acquire(&np->lock);
                havekids = 1;
                if(np->state == ZOMBIE){
                    pid = np->pid;
                    if(addr != 0 && copyout(p->pagetable, addr, (char *)&np->xstate,
                                            sizeof(np->xstate)) < 0) {
                        release(&np->lock);
                        release(&p->lock);
                        return -1;
                    }
                    freeproc(np);
                    release(&np->lock);
                    release(&p->lock);
                    return pid;
                }
                release(&np->lock);
            }
        }

        // No point waiting if we don't have any children.
        if(!havekids || p->killed){
            release(&p->lock);
            return -1;
        }

        // Wait for a child to exit.
        sleep(p, &p->lock);  //DOC: wait-sleep
    }
}
int
copyinstr(pagetable_t pagetable, char *dst, uint64 srcva, uint64 max)
{
    uint64 n, va0, pa0;
    int got_null = 0;

    while(got_null == 0 && max > 0){
        va0 = PGROUNDDOWN(srcva);
        // walkaddr checks that the user-supplied virtual address is partof the process’s user address space,
        // so programs cannot trick the kernel into reading other memory
        pa0 = walkaddr(pagetable, va0);
        if(pa0 == 0)
            return -1;
        n = PGSIZE - (srcva - va0);
        if(n > max)
            n = max;

        // Since the kernel maps all physical RAM addresses to the same kernel virtual address,
        // copyinstr can directly copy string bytes fromp a0 to dst
        char *p = (char *) (pa0 + (srcva - va0));
        while(n > 0){
            if(*p == '\0'){
                *dst = '\0';
                got_null = 1;
                break;
            } else {
                *dst = *p;
            }
            --n;
            --max;
            p++;
            dst++;
        }

        srcva = va0 + PGSIZE;
    }
    if(got_null){
        return 0;
    } else {
        return -1;
    }
}*/