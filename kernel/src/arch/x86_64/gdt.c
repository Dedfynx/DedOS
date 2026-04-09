#include <arch/x86_64/gdt.h>

static gdt_entry_t gdt[7];
static gdtr_t gdtr;

struct tss_entry {
    uint32_t reserved0;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserved1;
    uint64_t ist[7];
    uint32_t reserved2;
    uint32_t reserved3;
    uint16_t reserved4;
    uint16_t iopb_offset;
} __attribute__((packed));

static struct tss_entry tss;

static void gdt_set_entry(int i, uint8_t access, uint8_t granularity) {
    gdt[i].limit_low = 0xFFFF;
    gdt[i].base_low = 0;
    gdt[i].base_mid = 0;
    gdt[i].access = access;
    gdt[i].granularity = granularity;
    gdt[i].base_high = 0;
}

void gdt_init(void) {
    gdt_set_entry(0, 0x00, 0x00);                                                           // Null descriptor
    gdt_set_entry(1, GDT_PRESENT | GDT_KERNEL | GDT_CODE, GDT_GRANULARITY | GDT_LONGMODE);  // Kernel Code
    gdt_set_entry(2, GDT_PRESENT | GDT_KERNEL | GDT_DATA, 0x00);                            // Kernel Data
    gdt_set_entry(3, GDT_PRESENT | GDT_USER | GDT_CODE, GDT_GRANULARITY | GDT_LONGMODE);    // User Code
    gdt_set_entry(4, GDT_PRESENT | GDT_USER | GDT_DATA, 0x00);                              // User Data

    uint64_t tss_base = (uint64_t)&tss;
    uint32_t tss_limit = sizeof(tss) - 1;

    gdt[5].limit_low = tss_limit & 0xFFFF;
    gdt[5].base_low = tss_base & 0xFFFF;
    gdt[5].base_mid = (tss_base >> 16) & 0xFF;
    gdt[5].access = 0x89;
    gdt[5].granularity = ((tss_limit >> 16) & 0x0F);
    gdt[5].base_high = (tss_base >> 24) & 0xFF;

    uint32_t* tss_upper = (uint32_t*)&gdt[6];
    *tss_upper = (uint32_t)(tss_base >> 32);
    *(tss_upper + 1) = 0;

    gdtr.limit = sizeof(gdt) - 1;
    gdtr.base = (uint64_t)&gdt;

    gdt_load(&gdtr);

    __asm__ volatile("ltr %%ax" : : "a"(0x28));
}

void gdt_update_tss_rsp(uint64_t rsp) {
    tss.rsp0 = rsp;
}
