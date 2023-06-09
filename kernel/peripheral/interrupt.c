#include "stdbool.h"
#include "stdio.h"

#include "interrupt.h"
#include "segment.h"
#include "cpu.h"

#define PIC_MASTER_CMD  0x0020
#define PIC_MASTER_MASK 0x0021

#define PIC_SLAVE_CMD   0x00A0
#define PIC_SLAVE_MASK  0x00A1


/// Raw function to set an interrupt handler at a particular index in
/// the Interrupt Descriptor Table.
///
/// Important to note that the index in the IDT is not related at all
/// with the IRQ number.
void idt_configure(uint8_t index, uint32_t handler_addr) {
    uint32_t *entry = ((uint32_t *) 0x1000) + index * 2;
    entry[0] = (KERNEL_CS << 16) | (handler_addr & 0xFFFF);
    entry[1] = (handler_addr & 0xFFFF0000) | 0x8E00;
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
    idt_configure(32 + n, irq_handlers_entry[n]);
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
