#include "interrupt.h"
#include "console.h"
#include "stdio.h"
#include "cmos.h"
#include "cpu.h"
#include "cga.h"


#define TIME_CMD 0x43
#define TIME_CMD_FREQ 0x34
#define TIME_DATA 0x40

#define QUARTZ 0x1234DD
#define CLOCKFREQ 50


void pit_interrupt_handler_asm(void);
void rtc_interrupt_handler_asm(void);


// static uint8_t counter = 0;
static uint8_t seconds = 0;
static uint8_t minutes = 0;
static uint8_t hours = 0;
static uint8_t day = 0;
static uint8_t month = 0;

static const char* months_reprs[12] = {
    "Jan",
    "Feb",
    "Mar",
    "Apr",
    "May",
    "Jun",
    "Jul",
    "Aug",
    "Sep",
    "Oct",
    "Nov",
    "Dec"
};


static void time_print(void) {
    char time_str[16];
    snprintf(time_str, 16, "%3s %02d %02d:%02d:%02d", 
        months_reprs[month],
        day,
        hours, 
        minutes, 
        seconds
    );
    cga_fg_color(CGA_YELLOW);
    cga_write_bytes(time_str, 15);
    cga_fg_color(CGA_WHITE);
}

static void time_update() {
    seconds = get_cmos_reg_bcd(CMOS_REG_SECONDS);
    minutes = get_cmos_reg_bcd(CMOS_REG_MINUTES);
    hours = get_cmos_reg_bcd(CMOS_REG_HOURS);
    day = get_cmos_reg_bcd(CMOS_REG_DAY_IN_MONTH);
    month = get_cmos_reg_bcd(CMOS_REG_MONTH) - 1;
}

static void set_pit_frequency(void) {
    uint16_t freq_value = QUARTZ / CLOCKFREQ;
    outb(TIME_CMD_FREQ, TIME_CMD);
    outb(freq_value & 0xFF, 0x40);
    outb((freq_value >> 8) & 0xFFF, 0x40);
}

static void set_cmos_frequency(void) {
    uint8_t v = get_cmos_reg(CMOS_REG_RTC_FREQ);
    // Set frequency to 2Hz
    // 15 = log2(2^15 / 2Hz) + 1
    set_cmos_reg(CMOS_REG_RTC_FREQ, (v & 0xF0) | 0x0F);
}

static void set_cmos_interrupt(void) {
    uint8_t v = get_cmos_reg(CMOS_REG_RTC_TYPE);
    set_cmos_reg(CMOS_REG_RTC_TYPE, v | 0x40); // bit 6
}

// __attribute__((interrupt)) void pit_interrupt_handler(void *ctx) {
//     ((void) ctx);
//     interrupt_eoi(0);
// }

// __attribute__((interrupt)) void rtc_interrupt_handler(void *ctx) {
    
//     ((void) ctx);

//     interrupt_eoi(8);
//     get_cmos_reg(CMOS_REG_RTC_ACK);

//     time_update();
//     time_print();

// }

void time_start(void) {

    set_pit_frequency();
    // interrupt_set_handler(32, pit_interrupt_handler);
    interrupt_irq_mask(0, false);

    set_cmos_frequency();
    set_cmos_interrupt();
    // interrupt_set_handler(40, rtc_interrupt_handler);
    interrupt_irq_mask(2, false);
    interrupt_irq_mask(8, false);

    time_update();
    time_print();

}
