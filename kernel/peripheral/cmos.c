#include "cmos.h"
#include "cpu.h"

#define CMOS_CMD    0x0070
#define CMOS_DATA   0x0071


void cmos_nmi_enable(void) {
    cli();
    outb(inb(CMOS_CMD) & 0x7F, CMOS_CMD);
    inb(CMOS_DATA);
    sti();
}

void cmos_nmi_disable(void) {
    cli();
    outb(inb(CMOS_CMD) | 0x80, CMOS_CMD);
    inb(CMOS_DATA);
    sti();
}

uint8_t get_cmos_reg(uint8_t reg_num, bool mask_nmi) {
    cli();
    outb((mask_nmi << 7) | reg_num, CMOS_CMD);
    uint8_t data = inb(CMOS_DATA);
    sti();
    return data;
}

uint8_t get_cmos_reg_bcd(uint8_t reg_num, bool mask_nmi) {
    uint8_t value = get_cmos_reg(reg_num, mask_nmi);
    return (((value & 0xF0) >> 4) * 10) + (value & 0x0F);
}

void set_cmos_reg(uint8_t reg_num, uint8_t data, bool mask_nmi) {
    cli();
    outb((mask_nmi << 7) | reg_num, CMOS_CMD);
    outb(data, CMOS_DATA);
    sti();
}
