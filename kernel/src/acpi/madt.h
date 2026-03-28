#pragma once
#include <stdint.h>
#include <acpi/acpi.h>

typedef struct {
    acpi_sdt_header_t header;
    uint32_t lapic_address;
    uint32_t flags;
} __attribute__((packed)) madt_t;

typedef struct {
    uint8_t type;
    uint8_t length;
} __attribute__((packed)) madt_entry_t;

typedef struct {
    madt_entry_t header;
    uint8_t processor_id;
    uint8_t apic_id;
    uint32_t flags;
} __attribute__((packed)) madt_lapic_t;

typedef struct {
    madt_entry_t header;
    uint8_t ioapic_id;
    uint8_t reserved;
    uint32_t ioapic_address;
    uint32_t gsi_base;
} __attribute__((packed)) madt_ioapic_t;

typedef struct {
    madt_entry_t header;
    uint8_t bus;
    uint8_t irq;
    uint32_t gsi;
    uint16_t flags;
} __attribute__((packed)) madt_iso_t;

typedef struct {
    madt_entry_t header;
    uint8_t processor_id;
    uint16_t flags;
    uint8_t lint;
} __attribute__((packed)) madt_lapic_nmi_t;

void madt_init(void);

uint64_t madt_get_lapic_addr(void);
uint8_t madt_get_cpu_count(void);
uint64_t madt_get_ioapic_addr(void);
uint32_t madt_get_ioapic_gsi_base(void);
