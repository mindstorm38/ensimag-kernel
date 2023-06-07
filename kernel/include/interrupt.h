/// This module provides control over the Interrupt Descriptor Table
/// and provides an abstraction for Interrupt ReQuests.

#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__

#include "stdbool.h"
#include "stdint.h"


/// Type alias for interrupt handler function.
typedef void (*irq_handler_t)();

/// Configure an descriptor in the Interrupt Descriptor Table.
void idt_configure(uint8_t index, uint32_t handler_addr);

/// Set handle for a given interrupt (up to 16 handlers).
void irq_set_handler(uint8_t n, irq_handler_t handler);
/// Signal End Of Interrupt to the PIC.
void irq_eoi(uint8_t n);
/// Mask specific IRQ.
void irq_mask(uint8_t n, bool masked);

#endif
