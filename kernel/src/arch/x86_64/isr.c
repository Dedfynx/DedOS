#include <isr.h>
#include <pic.h>
#include <io.h>
#include <keyboard.h>
#include <kprintf.h>

static const char* exception_names[] = {
    "Division by zero",
    "Debug",
    "Non-maskable interrupt",
    "Breakpoint",
    "Overflow",
    "Bound range exceeded",
    "Invalid opcode",
    "Device not available",
    "Double fault",
    "Coprocessor segment overrun",
    "Invalid TSS",
    "Segment not present",
    "Stack fault",
    "General protection fault",
    "Page fault",
};

void isr_handler(interrupt_frame_t* frame) {
    if (frame->vector < 32) {
        kprintf("EXCEPTION: %s\n", exception_names[frame->vector]);
        kprintf("  vector: %u  error: %x\n", frame->vector, frame->error_code);
        kprintf("  rip: %p  rsp: %p\n", (void*)frame->rip, (void*)frame->rsp);
        __asm__ volatile("cli; hlt");
    } else {
        uint8_t irq = frame->vector - 32;
        if (irq == 1) {
            uint8_t scancode = inb(0x60);
            kprintf("key: %x\n", scancode);
            keyboard_handler();
        }
        pic_sendEOI(irq);
    }
}
