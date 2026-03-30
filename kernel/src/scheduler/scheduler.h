#pragma once
#include <scheduler/thread.h>
#include <scheduler/process.h>
#include <arch/x86_64/isr.h>

#define SCHEDULER_QUANTUM 10

void scheduler_init(void);
void scheduler_tick_irq(interrupt_frame_t* frame);
void scheduler_yield(void);
void scheduler_add_thread(thread_t* thread);
void scheduler_lock(void);
void scheduler_unlock(void);
thread_t* scheduler_current(void);
