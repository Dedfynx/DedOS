#include <scheduler/thread.h>
#include <scheduler/process.h>
#include <scheduler/scheduler.h>
#include <mm/heap.h>
#include <utils/log.h>
#include <stdint.h>
#include <libc/string.h>

extern volatile uint64_t timer_ticks;
static uint64_t next_tid = 0;

thread_t* thread_create(process_t* proc, void (*entry)(void)) {
    thread_t* thread = kmalloc(sizeof(thread_t));
    if (!thread) {
        log_error("THREAD", "kmalloc thread failed");
        return NULL;
    }
    memset(thread, 0, sizeof(thread_t));

    void* stack = kmalloc(THREAD_STACK_SIZE);
    if (!stack) {
        log_error("THREAD", "kmalloc stack failed");
        kfree(thread);
        return NULL;
    }

    thread->id = next_tid++;
    thread->state = THREAD_READY;
    thread->process = proc;
    thread->quantum = SCHEDULER_QUANTUM;
    thread->stack = stack;
    thread->next = NULL;
    thread->wake_tick = 0;
    thread->kernel_stack_top = (uint64_t)stack + THREAD_STACK_SIZE;

    uint64_t* sp = (uint64_t*)thread->kernel_stack_top;
    sp = (uint64_t*)((uint64_t)sp & ~0xFULL);
    *--sp = (uint64_t)thread_exit;

    thread->context.rip = (uint64_t)entry;
    thread->context.rsp = (uint64_t)sp;
    thread->context.rbp = 0;
    thread->context.rbx = 0;
    thread->context.cs = 0x08;
    thread->context.ss = 0x10;
    thread->context.rflags = 0x202;
    thread->context.r12 = 0;
    thread->context.r13 = 0;
    thread->context.r14 = 0;
    thread->context.r15 = 0;

    log_debug("THREAD", "Thread %u cree (Kernel) entry=%p kstack=%p",
        thread->id, (void*)entry, (void*)thread->kernel_stack_top);
    return thread;
}

void thread_destroy(thread_t* thread) {
    if (!thread) return;
    kfree(thread->stack);
    kfree(thread);
    log_debug("THREAD", "Thread detruit");
}

void thread_exit(void) {
    __asm__ volatile("cli");
    thread_t* t = scheduler_current();
    t->state = THREAD_DEAD;
    log_debug("THREAD", "Thread %u terminé", t->id);
    __asm__ volatile("sti");
    scheduler_yield();
    while (1) __asm__ volatile("hlt");
}

void thread_sleep(uint64_t ms) {
    __asm__ volatile("cli");
    thread_t* t = scheduler_current();
    t->state = THREAD_BLOCKED;
    t->wake_tick = timer_ticks + ms;
    __asm__ volatile("sti");
    scheduler_yield();
}
