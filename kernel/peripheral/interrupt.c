#include "interrupt.h"
#include "stdbool.h"
#include "stdio.h"

#include "segment.h"
#include "cpu.h"

#define PIC_MASTER_CMD 0x20
#define PIC_MASTER_MASK 0x21

#define PIC_SLAVE_CMD 0xA0
#define PIC_SLAVE_MASK 0xA1


void interrupt_set_handler(uint8_t n, interrupt_handler_t handler) {
    uint32_t handle_ptr = (uint32_t) handler;
    uint32_t *entry = (uint32_t *) (0x1000 + n * 8);
    entry[0] = (KERNEL_CS << 16) | (handle_ptr & 0xFFFF);
    entry[1] = (handle_ptr & 0xFFFF0000) | 0x8E00;
}

void interrupt_eoi(uint8_t n) {
    outb(0x20, PIC_MASTER_CMD);
    if (n >= 8) {
        outb(0x20, PIC_SLAVE_CMD);
    }
}

void interrupt_irq_mask(uint8_t n, bool masked) {
    bool master = n < 8;
    uint8_t mask_port = master ? PIC_MASTER_MASK : PIC_SLAVE_MASK;
    uint8_t current_mask = inb(mask_port);
    uint8_t mask = 1 << (master ? n : (n - 8));
    outb(masked ? (current_mask | mask) : (current_mask & ~(mask)), mask_port);
}
