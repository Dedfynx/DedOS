#include <scheduler/scheduler.h>
#include <scheduler/thread.h>
#include <scheduler/process.h>
#include <mm/heap.h>
#include <utils/log.h>
#include <stddef.h>
#include <stdint.h>

extern volatile uint64_t timer_ticks;

static thread_t* run_queue = NULL;
static thread_t* current = NULL;
static uint8_t locked = 0;

static thread_t idle_thread;
static process_t idle_process;

static void idle_fn(void) {
    while (1) __asm__ volatile("hlt");
}

void scheduler_init(void) {
    idle_process.pid = 0;
    idle_process.state = PROCESS_RUNNING;

    idle_thread.id = UINT64_MAX;
    idle_thread.state = THREAD_READY;
    idle_thread.process = &idle_process;
    idle_thread.quantum = SCHEDULER_QUANTUM;
    idle_thread.next = NULL;
    idle_thread.wake_tick = 0;

    uint64_t rsp;
    __asm__ volatile("mov %%rsp, %0" : "=r"(rsp));
    idle_thread.context.rip = (uint64_t)idle_fn;
    idle_thread.context.rsp = rsp;
    idle_thread.context.rbp = 0;
    idle_thread.context.rbx = 0;
    idle_thread.context.cs = 0x08;
    idle_thread.context.ss = 0x10;
    idle_thread.context.rflags = 0x202;
    idle_thread.context.r12 = 0;
    idle_thread.context.r13 = 0;
    idle_thread.context.r14 = 0;
    idle_thread.context.r15 = 0;

    current = &idle_thread;
    log_info("SCHED", "Scheduler initialisé");
}

void scheduler_lock(void) { locked = 1; }
void scheduler_unlock(void) { locked = 0; }

thread_t* scheduler_current(void) { return current; }

void scheduler_add_thread(thread_t* thread) {
    thread->next = run_queue;
    run_queue = thread;
}

void scheduler_yield(void) {
    if (scheduler_current())
        scheduler_current()->quantum = 0;
    __asm__ volatile("int $0x20");
}

void scheduler_tick_irq(interrupt_frame_t* frame) {
    if (!current || locked) return;

    thread_t* t = run_queue;
    while (t) {
        if (t->state == THREAD_BLOCKED && timer_ticks >= t->wake_tick)
            t->state = THREAD_READY;
        t = t->next;
    }

    thread_t* prev = NULL;
    t = run_queue;
    while (t) {
        if (t->state == THREAD_DEAD) {
            thread_t* dead = t;
            t = t->next;
            if (prev)
                prev->next = t;
            else
                run_queue = t;
            thread_destroy(dead);
        } else {
            prev = t;
            t = t->next;
        }
    }

    if (current->quantum > 0)
        current->quantum--;
    if (current->quantum > 0) return;

    thread_t* next = NULL;
    prev = NULL;
    thread_t* candidate = run_queue;
    while (candidate) {
        switch (candidate->state) {
            case THREAD_READY:
                if (prev)
                    prev->next = candidate->next;
                else
                    run_queue = candidate->next;
                next = candidate;
                break;
            case THREAD_BLOCKED:
            case THREAD_RUNNING:
            case THREAD_DEAD:
                prev = candidate;
                candidate = candidate->next;
                continue;
        }
        break;
    }

    if (!next) next = &idle_thread;

    if (current != &idle_thread) {
        switch (current->state) {
            case THREAD_RUNNING:
                current->state = THREAD_READY;
                current->quantum = SCHEDULER_QUANTUM;
            case THREAD_READY:
            case THREAD_BLOCKED:
                current->next = run_queue;
                run_queue = current;
                break;
            case THREAD_DEAD:
                break;
        }
        current->context.rip = frame->rip;
        current->context.rsp = frame->rsp;
        current->context.rbp = frame->rbp;
        current->context.cs = frame->cs;
        current->context.ss = frame->ss;
        current->context.rflags = frame->rflags;
        current->context.rax = frame->rax;
        current->context.rbx = frame->rbx;
        current->context.rcx = frame->rcx;
        current->context.rdx = frame->rdx;
        current->context.rsi = frame->rsi;
        current->context.rdi = frame->rdi;

        current->context.r8 = frame->r8;
        current->context.r9 = frame->r9;
        current->context.r10 = frame->r10;
        current->context.r11 = frame->r11;
        current->context.r12 = frame->r12;
        current->context.r13 = frame->r13;
        current->context.r14 = frame->r14;
        current->context.r15 = frame->r15;
    }

    /*
    if (current->process && current->process->pagemap) {
        vmm_switch(current->process->pagemap);
    }

    if (current->context.cs == 0x1B) {  // Si c'est un thread User
        gdt_update_tss_rsp(current->kernel_stack_top);
    }
    */

    next->state = THREAD_RUNNING;
    next->quantum = SCHEDULER_QUANTUM;
    current = next;

    frame->rip = next->context.rip;
    frame->rsp = next->context.rsp;
    frame->rbp = next->context.rbp;
    frame->rax = next->context.rax;
    frame->rbx = next->context.rbx;
    frame->rcx = next->context.rcx;
    frame->rdx = next->context.rdx;
    frame->rsi = next->context.rsi;
    frame->rdi = next->context.rdi;
    frame->ss = next->context.ss;
    frame->cs = next->context.cs;
    frame->rflags = next->context.rflags;

    frame->r8 = next->context.r8;
    frame->r9 = next->context.r9;
    frame->r10 = next->context.r10;
    frame->r11 = next->context.r11;
    frame->r12 = next->context.r12;
    frame->r13 = next->context.r13;
    frame->r14 = next->context.r14;
    frame->r15 = next->context.r15;
}
