#pragma once
#include <stdint.h>
#include <scheduler/thread.h>

typedef enum {
    PROCESS_RUNNING,
    PROCESS_DEAD,
} process_state_t;

typedef struct process_t {
    uint64_t pid;
    char name[64];
    process_state_t state;
    struct process_t* next;
} process_t;

process_t* process_create(const char* name);
void process_destroy(process_t* proc);
