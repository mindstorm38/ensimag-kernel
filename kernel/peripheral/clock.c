#include "stdio.h"

#include "interrupt.h"
#include "console.h"
#include "clock.h"
#include "cmos.h"
#include "cpu.h"
#include "cga.h"

#define TIME_CMD 0x0043
#define TIME_CMD_FREQ 0x34
#define TIME_DATA 0x0040

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
    char time_str[17];
    snprintf(time_str, 17, "%3s %02d %02d:%02d:%02d\n", 
        months_reprs[month],
        day,
        hours, 
        minutes, 
        seconds
    );
    cga_fg_color(CGA_YELLOW);
    cga_write_bytes(time_str, 17);
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

void pit_interrupt_handler() {
    irq_eoi(0);
}

void rtc_interrupt_handler() {

    irq_eoi(8);
    get_cmos_reg(CMOS_REG_RTC_ACK);

    time_update();
    time_print();

}

void clock_init(void) {

    set_pit_frequency();
    irq_set_handler(0, pit_interrupt_handler);
    irq_mask(0, false);

    set_cmos_frequency();
    set_cmos_interrupt();
    irq_set_handler(8, rtc_interrupt_handler);
    irq_mask(2, false);
    irq_mask(8, false);

    time_update();
    time_print();

}
