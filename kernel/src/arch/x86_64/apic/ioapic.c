#include <arch/x86_64/apic/ioapic.h>
#include <acpi/madt.h>
#include <mm/vmm.h>
#include <kernel/log.h>
#include <stdint.h>

extern uint64_t hhdm_offset;

static uint64_t ioapic_base = 0;

static uint32_t ioapic_read(uint8_t reg) {
    *(volatile uint32_t*)(ioapic_base + IOAPIC_REGSEL) = reg;
    return *(volatile uint32_t*)(ioapic_base + IOAPIC_IOWIN);
}

static void ioapic_write(uint8_t reg, uint32_t val) {
    *(volatile uint32_t*)(ioapic_base + IOAPIC_REGSEL) = reg;
    *(volatile uint32_t*)(ioapic_base + IOAPIC_IOWIN) = val;
}

static uint64_t ioapic_read_redir(uint8_t entry) {
    uint64_t lo = ioapic_read(IOAPIC_REDTBL + entry * 2);
    uint64_t hi = ioapic_read(IOAPIC_REDTBL + entry * 2 + 1);
    return lo | (hi << 32);
}

static void ioapic_write_redir(uint8_t entry, uint64_t val) {
    ioapic_write(IOAPIC_REDTBL + entry * 2, (uint32_t)(val & 0xFFFFFFFF));
    ioapic_write(IOAPIC_REDTBL + entry * 2 + 1, (uint32_t)(val >> 32));
}

void ioapic_redirect(uint8_t irq, uint8_t vector, uint32_t flags) {
    uint64_t entry = vector | flags;
    ioapic_write_redir(irq, entry);
}

void ioapic_mask(uint8_t irq) {
    uint64_t entry = ioapic_read_redir(irq);
    ioapic_write_redir(irq, entry | IOAPIC_MASKED);
}

void ioapic_unmask(uint8_t irq) {
    uint64_t entry = ioapic_read_redir(irq);
    ioapic_write_redir(irq, entry & ~(uint64_t)IOAPIC_MASKED);
}

void ioapic_init(void) {
    ioapic_base = madt_get_ioapic_addr();

    pml4_t* pml4 = vmm_get_current();
    vmm_map(pml4, ioapic_base, ioapic_base - hhdm_offset, VMM_KERNEL);

    uint32_t ver = ioapic_read(IOAPIC_VER);
    uint8_t max_redir = (ver >> 16) & 0xFF;
    log_info("IOAPIC", "Version=%x Max_redir=%u", ver & 0xFF, max_redir);

    for (uint8_t i = 0; i <= max_redir; i++)
        ioapic_write_redir(i, IOAPIC_MASKED);
}
