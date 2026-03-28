#pragma once
#include <stdint.h>

#define LAPIC_TIMER_VECTOR 32  // vecteur pour le timer

void lapic_timer_init(void);
void lapic_timer_calibrate(void);
uint64_t lapic_timer_freq(void);
