#include "stddef.h"

#include "interrupt.h"
#include "pit.h"
#include "cpu.h"


#define PIT_CHAN0_DATA  0x0040
#define PIT_CHAN1_DATA  0x0041
#define PIT_CHAN2_DATA  0x0042

#define PIT_CMD         0x0043
// PIT_CMD_FREQ = channel 0 | access mode lo/hi | rate generator | binary mode
#define PIT_CMD_FREQ    0x34

// 1.193181 MHz
#define PIT_QUARTZ_FREQ 0x1234DD
#define PIT_FREQ        50
#define PIT_INTERVAL    ((uint16_t) ((int) (PIT_QUARTZ_FREQ) / (PIT_FREQ)))


static uint32_t clock = 0;
static pit_handler_t active_handler = NULL;


/// Internal function to initialize frequency of the PIT.
static void pit_set_frequency(void) {
    const uint16_t freq_value = PIT_INTERVAL;
    outb(PIT_CMD_FREQ, PIT_CMD);
    outb(freq_value & 0xFF, PIT_CHAN0_DATA);
    outb((freq_value >> 8) & 0xFF, PIT_CHAN0_DATA);
}

/// Internal function that handless PIT interrupts.
static void pit_interrupt_handler(void) {
    irq_eoi(0);
    clock++;
    if (active_handler != NULL) {
        active_handler(clock);
    }
}

void pit_init(void) {

    pit_set_frequency();
    irq_set_handler(0, pit_interrupt_handler);
    irq_mask(0, false);

}

uint32_t pit_clock() {
    return clock;
}

void pit_clock_settings(uint32_t *quartz_freq, uint32_t *ticks) {
    *quartz_freq = PIT_FREQ;
    *ticks = PIT_INTERVAL;
}

/// Set the PIT handler that will be called upon interruption.
void pit_set_handler(pit_handler_t handler) {
    active_handler = handler;
}
