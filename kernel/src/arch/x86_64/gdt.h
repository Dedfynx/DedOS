#pragma once
#include <stdint.h>

typedef enum {
    GDT_PRESENT = 0x80,
    GDT_KERNEL = 0x00,
    GDT_USER = 0x60,
    GDT_S = 0x10,
    GDT_EX = 0x08,
    GDT_DC = 0x04,
    GDT_RW = 0x02,
    GDT_AC = 0x01,
    GDT_CODE = (GDT_S | GDT_EX),
    GDT_DATA = (GDT_S | GDT_RW),
    GDT_GRANULARITY = 0x80,
    GDT_OPERAND_SIZE = 0x40,
    GDT_LONGMODE = 0x20,
} gdt_bits_t;

typedef struct gdt_entry_t {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed)) gdt_entry_t;

typedef struct gdtr_t {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed)) gdtr_t;

extern void gdt_load(gdtr_t* gdtr);

void gdt_init(void);
void gdt_update_tss_rsp(uint64_t rsp);
