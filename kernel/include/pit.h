/// Programmable Interval Timer

#ifndef __PIT_H__
#define __PIT_H__

#include "stdint.h"


/// Initialize the pit to a correct frequency for scheduling.
void pit_init(void);
/// Get the current clock ticks since startup.
uint32_t pit_clock(void);
/// Get current clock configuration.
void pit_clock_settings(uint32_t *quartz_freq, uint32_t *ticks);

#endif
