#include "stdio.h"

#include "interrupt.h"
#include "console.h"
#include "cmos.h"
#include "cpu.h"
#include "cga.h"
#include "rtc.h"


// static uint8_t counter = 0;
static uint8_t seconds = 0;
static uint8_t minutes = 0;
static uint8_t hours = 0;
static uint8_t day = 0;
static uint8_t month = 0;

// static const char* months_reprs[12] = {
//     "Jan",
//     "Feb",
//     "Mar",
//     "Apr",
//     "May",
//     "Jun",
//     "Jul",
//     "Aug",
//     "Sep",
//     "Oct",
//     "Nov",
//     "Dec"
// };


// static void time_print(void) {
//     char time_str[17];
//     snprintf(time_str, 17, "%3s %02d %02d:%02d:%02d\n", 
//         months_reprs[month],
//         day,
//         hours, 
//         minutes, 
//         seconds
//     );
//     cga_fg_color(CGA_YELLOW);
//     cga_write_bytes(time_str, 17);
//     cga_fg_color(CGA_WHITE);
// }

static void time_update() {
    seconds = get_cmos_reg_bcd(CMOS_REG_SECONDS, false);
    minutes = get_cmos_reg_bcd(CMOS_REG_MINUTES, false);
    hours = get_cmos_reg_bcd(CMOS_REG_HOURS, false);
    day = get_cmos_reg_bcd(CMOS_REG_DAY_IN_MONTH, false);
    month = get_cmos_reg_bcd(CMOS_REG_MONTH, false) - 1;
}

static void rtc_set_frequency(void) {
    uint8_t v = get_cmos_reg(CMOS_REG_STATUS_A, true);
    // Set frequency to 2Hz
    // 15 = log2(2^15 / 2Hz) + 1
    set_cmos_reg(CMOS_REG_STATUS_A, (v & 0xF0) | 0x0F, true);
}

static void rtc_enable_interrupt(void) {
    uint8_t v = get_cmos_reg(CMOS_REG_STATUS_B, true);
    set_cmos_reg(CMOS_REG_STATUS_B, v | 0x40, true); // bit 6
}

static void rtc_interrupt_handler() {
    
    get_cmos_reg(CMOS_REG_STATUS_C, false);

    time_update();
    // time_print();

    irq_eoi(IRQ_RTC);

}

void rtc_init(void) {

    panic("code to update");

    rtc_set_frequency();
    rtc_enable_interrupt();
    irq_set_handler(IRQ_RTC, rtc_interrupt_handler);
    irq_mask(IRQ_SLAVE, false);
    irq_mask(IRQ_RTC, false);

    // Re-enable NMI afterward.
    cmos_nmi_enable();

    time_update();
    // time_print();

}
