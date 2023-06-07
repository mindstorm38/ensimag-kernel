#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__

#include "stdbool.h"
#include "stdint.h"


typedef __attribute__((interrupt)) void (*interrupt_handler_t)(void*);

/// Set handle for a given interrupt (up to 256 handlers).
void interrupt_set_handler(uint8_t n, interrupt_handler_t handler);
/// Signal End Of Interrupt to the PIC.
void interrupt_eoi(uint8_t n);
/// Mask specific IRQ.
void interrupt_irq_mask(uint8_t n, bool masked);

#endif
