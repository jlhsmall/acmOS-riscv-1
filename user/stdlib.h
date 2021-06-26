#ifndef ACMOS_STDLIB_H
#define ACMOS_STDLIB_H
#include "answer_printk.h"
#include <lock.h>
typedef __builtin_va_list va_list;
#define va_start(v, l) __builtin_va_start(v, l)
#define va_end(v) __builtin_va_end(v)
#define va_arg(v, l) __builtin_va_arg(v, l)
#define va_copy(d, s) __builtin_va_copy(d, s)

#define SYS_EXIT 1
#define SYS_PUTC 2
#define SYS_YIELD 3

// 参考 asm volatile 的插入方式，将以下3个syscall函数插入完整
// 参考资料：https://gcc.gnu.org/onlinedocs/gcc/Extended-Asm.html
// 注意：我们是 RISCV 架构，最后进入内核态需要 ecall 指令
void sys_putc(char val);
void sys_exit(int value);
void sys_yield();
#endif