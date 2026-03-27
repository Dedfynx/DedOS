#include <idt.h>
#include <isr.h>
#include <stdint.h>

#define IDT_SIZE 256

static idt_entry_t idt[IDT_SIZE];
static idtr_t idtr;

extern void* isr_stub_table[];

void idt_set_entry(uint8_t vector, void* handler, uint8_t flags) {
    uint64_t addr = (uint64_t)handler;
    idt[vector].isr_low = addr & 0xFFFF;
    idt[vector].isr_mid = (addr >> 16) & 0xFFFF;
    idt[vector].isr_high = (addr >> 32) & 0xFFFFFFFF;
    idt[vector].kernel_cs = 0x08;  // kernel code segment
    idt[vector].ist = 0;
    idt[vector].attributes = flags;
    idt[vector].reserved = 0;
}

void idt_init(void) {
    idtr.limit = sizeof(idt) - 1;
    idtr.base = (uint64_t)&idt;

    for (uint8_t i = 0; i < 32; i++) {
        idt_set_entry(i, isr_stub_table[i], 0x8E);
    }
    __asm__ volatile("lidt %0" : : "m"(idtr));
}
