#include <arch/x86_64/apic/lapic_timer.h>
#include <arch/x86_64/apic/lapic.h>
#include <arch/x86_64/io.h>
#include <utils/log.h>
#include <stdint.h>

#define PIT_CHANNEL0 0x40
#define PIT_CMD 0x43
#define PIT_FREQ 1193182
static uint64_t lapic_freq = 0;

static void pit_sleep_ms(uint32_t ms) {
    uint32_t divisor = (PIT_FREQ * ms) / 1000;

    // Mode 0 (one-shot), channel 0, binary
    outb(PIT_CMD, 0x30);
    outb(PIT_CHANNEL0, divisor & 0xFF);
    outb(PIT_CHANNEL0, (divisor >> 8) & 0xFF);
}

static uint16_t pit_read(void) {
    outb(PIT_CMD, 0x00);  // latch channel 0
    uint16_t lo = inb(PIT_CHANNEL0);
    uint16_t hi = inb(PIT_CHANNEL0);
    return (hi << 8) | lo;
}

void lapic_timer_calibrate(void) {
    // Configure diviseur à 16
    lapic_write(LAPIC_TIMER_DIV, 0x3);
    lapic_write(LAPIC_TIMER, LAPIC_TIMER_MASKED);
    lapic_write(LAPIC_TIMER_INIT, 0xFFFFFFFF);

    uint16_t pit_ticks = (PIT_FREQ * 10) / 1000;  // ~11932
    outb(PIT_CMD, 0x30);
    outb(PIT_CHANNEL0, pit_ticks & 0xFF);
    outb(PIT_CHANNEL0, (pit_ticks >> 8) & 0xFF);

    uint16_t last = 0xFFFF;
    while (1) {
        outb(PIT_CMD, 0x00);  // latch
        uint8_t lo = inb(PIT_CHANNEL0);
        uint8_t hi = inb(PIT_CHANNEL0);
        uint16_t curr = ((uint16_t)hi << 8) | lo;
        if (curr > last) break;  // le compteur a wrappé -> terminé
        last = curr;
    }

    uint32_t ticks = 0xFFFFFFFF - lapic_read(LAPIC_TIMER_CURR);
    lapic_write(LAPIC_TIMER_INIT, 0);

    lapic_freq = (uint64_t)ticks * 16 * 100;
    log_info("LAPIC", "Timer: freq = %u MHz", (uint32_t)(lapic_freq / 1000000));
}

void lapic_timer_init(void) {
    lapic_timer_calibrate();

    uint32_t count = lapic_freq / 16 / 1000;

    lapic_write(LAPIC_TIMER_DIV, 0x3);
    lapic_write(LAPIC_TIMER, LAPIC_TIMER_PERIODIC | LAPIC_TIMER_VECTOR);
    lapic_write(LAPIC_TIMER_INIT, count);
}
