#pragma once
#include <stdint.h>

// Registres LAPIC (offsets depuis la base)
#define LAPIC_ID 0x020          // ID
#define LAPIC_VERSION 0x030     // Version
#define LAPIC_TPR 0x080         // Task Priority
#define LAPIC_EOI 0x0B0         // End of Interrupt
#define LAPIC_SVR 0x0F0         // Spurious Interrupt Vector
#define LAPIC_ICR_LOW 0x300     // Interrupt Command (low)
#define LAPIC_ICR_HIGH 0x310    // Interrupt Command (high)
#define LAPIC_TIMER 0x320       // Timer LVT
#define LAPIC_TIMER_INIT 0x380  // Timer Initial Count
#define LAPIC_TIMER_CURR 0x390  // Timer Current Count
#define LAPIC_TIMER_DIV 0x3E0   // Timer Divide Config
#define LAPIC_LVT0 0x350
#define LAPIC_LVT1 0x360
// Flags SVR
#define LAPIC_SVR_ENABLE (1 << 8)

// Flags Timer
#define LAPIC_TIMER_PERIODIC (1 << 17)
#define LAPIC_TIMER_MASKED (1 << 16)

void lapic_init(void);
void lapic_eoi(void);
uint32_t lapic_id(void);
void lapic_timer_start(uint8_t vector, uint32_t count);
void lapic_timer_stop(void);
uint32_t lapic_read(uint32_t reg);
void lapic_write(uint32_t reg, uint32_t val);
