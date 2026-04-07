#include <utils/log.h>
#include <acpi/madt.h>
#include <acpi/acpi.h>
#include <stdint.h>

extern uint64_t hhdm_offset;

#define MAX_CPUS 16
#define MAX_IOAPICS 4
#define MAX_ISOS 16

static uint64_t lapic_addr = 0;
static uint8_t cpu_count = 0;
static uint8_t lapic_ids[MAX_CPUS];

static uint64_t ioapic_addr = 0;
static uint32_t ioapic_gsi_base = 0;

static madt_iso_t* isos[MAX_ISOS];
static uint8_t iso_count = 0;

void madt_init(void) {
    madt_t* madt = (madt_t*)acpi_find_table("APIC");
    if (!madt) {
        log_error("MADT", "Table non trouvee !");
        return;
    }

    lapic_addr = (uint64_t)madt->lapic_address;

    uint8_t* ptr = (uint8_t*)madt + sizeof(madt_t);
    uint8_t* end = (uint8_t*)madt + madt->header.length;

    while (ptr < end) {
        madt_entry_t* entry = (madt_entry_t*)ptr;

        switch (entry->type) {
            case 0: {  // Local APIC
                madt_lapic_t* lapic = (madt_lapic_t*)entry;
                if (lapic->flags & 1) {  // CPU enabled
                    if (cpu_count < MAX_CPUS) {
                        lapic_ids[cpu_count++] = lapic->apic_id;
                        log_debug("MADT", "CPU %u APIC ID %u",
                            lapic->processor_id, lapic->apic_id);
                    }
                }
                break;
            }
            case 1: {  // I/O APIC
                madt_ioapic_t* ioapic = (madt_ioapic_t*)entry;
                ioapic_addr = (uint64_t)ioapic->ioapic_address;
                ioapic_gsi_base = ioapic->gsi_base;
                log_debug("MADT", "IOAPIC addr=%p gsi_base=%u",
                    (void*)ioapic_addr, ioapic_gsi_base);
                break;
            }
            case 2: {  // Interrupt Source Override
                madt_iso_t* iso = (madt_iso_t*)entry;
                if (iso_count < MAX_ISOS)
                    isos[iso_count++] = iso;
                log_debug("MADT", "ISO IRQ %u -> GSI %u flags=%x",
                    iso->irq, iso->gsi, iso->flags);
                break;
            }
            case 4: {  // Local APIC NMI
                madt_lapic_nmi_t* nmi = (madt_lapic_nmi_t*)entry;
                log_debug("MADT", "LAPIC NMI proc=%u lint=%u",
                    nmi->processor_id, nmi->lint);
                break;
            }
            default:
                log_warn("MADT", "Entrée type %u ignoree", entry->type);
                break;
        }

        ptr += entry->length;
    }
    log_info("MADT", "%u CPU(s), LAPIC=%p IOAPIC=%p",
        cpu_count, (void*)lapic_addr, (void*)ioapic_addr);
}

uint64_t madt_get_lapic_addr(void) { return lapic_addr + hhdm_offset; }
uint8_t madt_get_cpu_count(void) { return cpu_count; }
uint64_t madt_get_ioapic_addr(void) { return ioapic_addr + hhdm_offset; }
uint32_t madt_get_ioapic_gsi_base(void) { return ioapic_gsi_base; }
