#ifndef __snow_internal_h__
#define __snow_internal_h__

#include "snow_arch.h"

#define arch_assert(cond, str)	do{if (!(cond)) {arch_print("SNOW assert(%s, %d): %s\n", __func__, __LINE__, str);}}while(0)

extern struct snow_thread_t *current, *idle;

long arch_get_pc(void *regs);
void arch_set_pc(void *regs, long pc);
long arch_get_sp(void *regs);
void arch_set_sp(void *regs, long sp);
long arch_get_arg(void *regs, int index);
void arch_set_arg(void *regs, int index, long arg);
int arch_syscall(int call, long arg);
void arch_disable_interrupt(void);
void arch_enable_interrupt(void);

void __snow_sched(void);
void __snow_syscall_handler(void *regs);
void __snow_tick_handler(void *regs);

#endif

