#include "stdint.h"

#include "interrupt.h"
#include "pit.h"
#include "cpu.h"

#include "stdio.h"


#define PIT_CHAN0_DATA  0x0040
#define PIT_CHAN1_DATA  0x0041
#define PIT_CHAN2_DATA  0x0042

#define PIT_CMD         0x0043
// PIT_CMD_FREQ = channel 0 | access mode lo/hi | rate generator | binary mode
#define PIT_CMD_FREQ    0x34

// 1.193181 MHz
#define PIT_QUARTZ_FREQ 0x1234DD
#define PIT_CONF_FREQ   50


static void pit_set_frequency(void) {
    uint16_t freq_value = (uint16_t) ((int) PIT_QUARTZ_FREQ / PIT_CONF_FREQ);
    cli();
    outb(PIT_CMD_FREQ, PIT_CMD);
    outb(freq_value & 0xFF, PIT_CHAN0_DATA);
    outb((freq_value >> 8) & 0xFF, PIT_CHAN0_DATA);
    sti();
}

static void pit_interrupt_handler(void) {
    irq_eoi(0);
}

void pit_init(void) {

    pit_set_frequency();
    irq_set_handler(0, pit_interrupt_handler);
    irq_mask(0, false);

}
