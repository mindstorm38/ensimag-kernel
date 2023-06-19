/// This module provides control over the Interrupt Descriptor Table
/// and provides an abstraction for Interrupt ReQuests.

#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__

#include "stdbool.h"
#include "stdint.h"

/// Offset of the first interrupt for IRQ.
#define IRQ_INTERRUPT_OFFSET 32

// From Intel ICH9 spec.
#define IRQ_PIT         0
#define IRQ_KEYBOARD    1
#define IRQ_SLAVE       2
#define IRQ_SERIAL_A    3
#define IRQ_SERIAL_B    4
#define IRQ_GENERIC_5   5
#define IRQ_FLOPPY      6
#define IRQ_GENERIC_7   7
#define IRQ_RTC         8
#define IRQ_GENERIC_9   9
#define IRQ_GENERIC_10  10
#define IRQ_GENERIC_11  11
#define IRQ_MOUSE       12
#define IRQ_INTERNAL_13 13
#define IRQ_SATA_14     14
#define IRQ_SATA_15     15

/// Type alias for interrupt handler function.
typedef void (*irq_handler_t)();

void idt_interrupt_gate(uint8_t index, uint32_t handler_offset, uint8_t dpl);
void idt_trap_gate(uint8_t index, uint32_t handler_offset, uint8_t dpl);
void idt_task_gate(uint8_t index, uint16_t tss_segment, uint8_t dpl);
void idt_remove_gate(uint8_t index);

/// Set handle for a given IRQ (up to 16 handlers). The handler can
/// be directly written in C because this function handles context
/// saving and restoration before 'iret'.
void irq_set_handler(uint8_t n, irq_handler_t handler);
/// Signal End Of Interrupt to the PIC.
void irq_eoi(uint8_t n);
/// Mask specific IRQ.
void irq_mask(uint8_t n, bool masked);

#endif
