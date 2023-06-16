/// Programmable Interval Timer

#ifndef __PIT_H__
#define __PIT_H__

#include "stdint.h"


/// Type alias for a Programmable Internal Timer.
typedef void (*pit_handler_t)(uint32_t);

/// Initialize the pit to a correct frequency for scheduling.
void pit_init(void);
/// Get the current clock ticks since startup.
uint32_t pit_clock_get(void);
/// Get current clock configuration.
void pit_clock_settings(uint32_t *quartz_freq, uint32_t *ticks);
/// Set the PIT handler that will be called upon interruption.
void pit_set_handler(pit_handler_t handler);

#endif
