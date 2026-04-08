#pragma once
#include <stdint.h>
#include <arch/x86_64/regs.h>

#define THREAD_STACK_SIZE 4096  // 64KB

typedef enum {
    THREAD_READY,
    THREAD_RUNNING,
    THREAD_BLOCKED,
    THREAD_DEAD,
} thread_state_t;

typedef struct {
    uint64_t rip, rsp, rbp, rbx;
    uint64_t r12, r13, r14, r15;
} thread_context_t;

struct process_t;

typedef struct thread_t {
    uint64_t id;
    thread_state_t state;
    thread_context_t context;
    void* stack;
    uint64_t wake_tick;
    uint8_t quantum;
    struct process_t* process;
    struct thread_t* next;
} thread_t;

thread_t* thread_create(struct process_t* proc, void (*entry)(void));
void thread_destroy(thread_t* thread);
void thread_exit(void);
void thread_sleep(uint64_t ms);
