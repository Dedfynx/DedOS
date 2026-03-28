#pragma once
#include <stdint.h>

// Registres IOAPIC
#define IOAPIC_REGSEL 0x00  // Register Select
#define IOAPIC_IOWIN 0x10   // I/O Window

// Registres internes
#define IOAPIC_ID 0x00
#define IOAPIC_VER 0x01
#define IOAPIC_ARB 0x02
#define IOAPIC_REDTBL 0x10  // Redirection Table (2 regs par entrée)

// Flags redirection
#define IOAPIC_MASKED (1 << 16)

void ioapic_init(void);
void ioapic_redirect(uint8_t irq, uint8_t vector, uint32_t flags);
void ioapic_mask(uint8_t irq);
void ioapic_unmask(uint8_t irq);
