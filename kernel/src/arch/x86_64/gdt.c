#include "gdt.h"

static gdt_entry_t gdt[3];
static gdtr_t gdtr;

static void gdt_set_entry(int i, uint8_t access, uint8_t granularity) {
    gdt[i].limit_low = 0xFFFF;
    gdt[i].base_low = 0;
    gdt[i].base_mid = 0;
    gdt[i].access = access;
    gdt[i].granularity = granularity;
    gdt[i].base_high = 0;
}

void gdt_init(void) {
    // Null descriptor
    gdt_set_entry(0, 0x00, 0x00);
    // Kernel code: present, ring0, code, longmode, granularity
    gdt_set_entry(1, GDT_PRESENT | GDT_KERNEL | GDT_CODE, GDT_GRANULARITY | GDT_LONGMODE);
    // Kernel data: present, ring0, data
    gdt_set_entry(2, GDT_PRESENT | GDT_KERNEL | GDT_DATA, 0x00);

    gdtr.limit = sizeof(gdt) - 1;
    gdtr.base = (uint64_t)&gdt;

    gdt_load(&gdtr);
}
