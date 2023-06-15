#include "stdbool.h"
#include "stdio.h"

#include "interrupt.h"
#include "segment.h"
#include "cpu.h"
#include <stdint.h>

#define PIC_MASTER_CMD  0x0020
#define PIC_MASTER_MASK 0x0021

#define PIC_SLAVE_CMD   0x00A0
#define PIC_SLAVE_MASK  0x00A1

#define IDT_PRESENT         0x8000
#define IDT_INTERRUPT_GATE  0x0E00
#define IDT_TRAP_GATE       0x0F00
#define IDT_TASK_GATE       0x0500
#define IDT_DPL_SHIFT       13


/// Internal function to set gate's value.
static inline void idt_set_gate(uint8_t index, uint16_t segment, uint32_t offset, uint16_t flags) {
    uint32_t *entry = ((uint32_t *) 0x1000) + index * 2;
    entry[0] = (((uint32_t) segment) << 16) | (offset & 0xFFFF);
    entry[1] = (offset & 0xFFFF0000) | (uint32_t) flags;
}

void idt_interrupt_gate(uint8_t index, uint32_t handler_offset, uint8_t dpl) {
    idt_set_gate(index, KERNEL_CS, handler_offset, IDT_PRESENT | IDT_INTERRUPT_GATE | (dpl & 0x3) << IDT_DPL_SHIFT);
}

void idt_trap_gate(uint8_t index, uint32_t handler_offset, uint8_t dpl) {
    idt_set_gate(index, KERNEL_CS, handler_offset, IDT_PRESENT | IDT_TRAP_GATE | (dpl & 0x3) << IDT_DPL_SHIFT);
}

void idt_task_gate(uint8_t index, uint16_t tss_segment, uint8_t dpl) {
    idt_set_gate(index, tss_segment, 0, IDT_PRESENT | IDT_TASK_GATE | (dpl & 0x3) << IDT_DPL_SHIFT);
}

void idt_remove_gate(uint8_t index) {
    idt_set_gate(index, 0, 0, 0);
}


/// This array is defined in assembly.
extern uint32_t irq_handlers_entry[16];
/// This array is exported to the assembly.
static irq_handler_t irq_handlers[16];

/// Called from assembly.
void irq_generic_handler(uint8_t n) {
    irq_handlers[n]();
}

void irq_set_handler(uint8_t n, irq_handler_t handler) {
    idt_interrupt_gate(32 + n, irq_handlers_entry[n], 0);
    irq_handlers[n] = handler;
}

void irq_eoi(uint8_t n) {
    outb(0x20, PIC_MASTER_CMD);
    if (n >= 8) {
        outb(0x20, PIC_SLAVE_CMD);
    }
}

void irq_mask(uint8_t n, bool masked) {
    bool master = n < 8;
    uint8_t mask_port = master ? PIC_MASTER_MASK : PIC_SLAVE_MASK;
    uint8_t current_mask = inb(mask_port);
    uint8_t mask = 1 << (master ? n : (n - 8));
    outb(masked ? (current_mask | mask) : (current_mask & ~(mask)), mask_port);
}
