#include <arch/x86_64/isr.h>
#include <arch/x86_64/pic.h>
#include <arch/x86_64/io.h>
#include <arch/x86_64/apic/lapic.h>
#include <drivers/keyboard.h>
#include <utils/log.h>

volatile uint64_t timer_ticks = 0;

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
        log_error("ISR", "EXCEPTION: %s | vector: %u error: %x | rip: %p rsp: %p",
            exception_names[frame->vector],
            frame->vector, frame->error_code,
            (void*)frame->rip, (void*)frame->rsp);

        __asm__ volatile("cli; hlt");
    } else {
        uint8_t irq = frame->vector - 32;
        if (irq == 0) {
            // timer tick
            timer_ticks++;
        } else if (irq == 1) {
            keyboard_handler();
        }
        lapic_eoi();  // pic_sendEOI(irq);
    }
}
