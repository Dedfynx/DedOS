#pragma once
#include <stdint.h>
#include <arch/x86_64/regs.h>

typedef registers_t interrupt_frame_t;

extern void isr0(void);
extern void isr1(void);
extern void isr14(void);

void isr_handler(interrupt_frame_t* frame);
