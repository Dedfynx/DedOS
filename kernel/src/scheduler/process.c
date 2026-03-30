#include <scheduler/process.h>
#include <mm/heap.h>
#include <utils/log.h>
#include <stdint.h>

static uint64_t next_pid = 0;

process_t* process_create(const char* name) {
    process_t* proc = kmalloc(sizeof(process_t));
    if (!proc) {
        log_error("PROCESS", "kmalloc failed");
        return NULL;
    }

    proc->pid = next_pid++;
    proc->state = PROCESS_RUNNING;
    proc->next = NULL;

    uint32_t i = 0;
    while (name[i] && i < 63) {
        proc->name[i] = name[i];
        i++;
    }
    proc->name[i] = '\0';

    log_debug("PROCESS", "Processus '%s' cree (pid=%u)", proc->name, proc->pid);
    return proc;
}

void process_destroy(process_t* proc) {
    if (!proc) return;
    proc->state = PROCESS_DEAD;
    kfree(proc);
}
