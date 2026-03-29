#include <arch/x86_64/apic/lapic.h>
#include <acpi/madt.h>
#include <utils/log.h>
#include <mm/vmm.h>
#include <stdint.h>

extern uint64_t hhdm_offset;

static uint64_t lapic_base = 0;

// Lit un registre LAPIC
inline uint32_t lapic_read(uint32_t reg) {
    return *(volatile uint32_t*)(lapic_base + reg);
}

// Écrit un registre LAPIC
inline void lapic_write(uint32_t reg, uint32_t val) {
    *(volatile uint32_t*)(lapic_base + reg) = val;
}

void lapic_init(void) {
    lapic_base = madt_get_lapic_addr();
    pml4_t* pml4 = vmm_get_current();
    vmm_map(pml4, lapic_base, lapic_base - hhdm_offset, VMM_KERNEL);

    // Active le LAPIC via le SVR
    lapic_write(LAPIC_SVR, lapic_read(LAPIC_SVR) | LAPIC_SVR_ENABLE | 0xFF);

    lapic_write(0x350, 0x00010000);  // LVT0 masked
    lapic_write(0x360, 0x00010400);  // LVT1 NMI LAPIC_TIMER_MASKED
                                     //
    lapic_write(LAPIC_TPR, 0);
    log_info("LAPIC", "ID = %u Version = %x", lapic_read(LAPIC_ID) >> 24, lapic_read(LAPIC_VERSION));
}

void lapic_eoi(void) {
    lapic_write(LAPIC_EOI, 0);
}

uint32_t lapic_id(void) {
    return lapic_read(LAPIC_ID) >> 24;
}

void lapic_timer_start(uint8_t vector, uint32_t count) {
    // Configure le diviseur à 16
    lapic_write(LAPIC_TIMER_DIV, 0x3);

    // Configure le timer en mode périodique
    lapic_write(LAPIC_TIMER, LAPIC_TIMER_PERIODIC | vector);

    // Démarre le timer
    lapic_write(LAPIC_TIMER_INIT, count);
}

void lapic_timer_stop(void) {
    lapic_write(LAPIC_TIMER_INIT, 0);
    lapic_write(LAPIC_TIMER, LAPIC_TIMER_MASKED);
}
