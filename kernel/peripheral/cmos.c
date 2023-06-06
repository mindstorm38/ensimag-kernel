#include "cpu.h"
#include "cmos.h"

#define CMOS_CMD 0x70
#define CMOS_DATA 0x71


uint8_t get_cmos_reg(uint8_t reg_num) {
    outb(0x80 | reg_num, CMOS_CMD);
    return inb(CMOS_DATA);
}

uint8_t get_cmos_reg_bcd(uint8_t reg_num) {
    uint8_t value = get_cmos_reg(reg_num);
    return (((value & 0xF0) >> 4) * 10) + (value & 0x0F);
}

void set_cmos_reg(uint8_t reg_num, uint8_t data) {
    outb(0x80 | reg_num, CMOS_CMD);
    outb(data, CMOS_DATA);
}
