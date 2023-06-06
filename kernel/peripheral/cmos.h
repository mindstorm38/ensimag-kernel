#ifndef __CMOS_H__
#define __CMOS_H__

#include "stdint.h"

#define CMOS_REG_SECONDS 0x00
#define CMOS_REG_MINUTES 0x02
#define CMOS_REG_HOURS 0x04
#define CMOS_REG_DAY_IN_WEEK 0x06
#define CMOS_REG_DAY_IN_MONTH 0x07
#define CMOS_REG_MONTH 0x08
#define CMOS_REG_YEAR 0x09
#define CMOS_REG_RTC_FREQ 0x0A
#define CMOS_REG_RTC_TYPE 0x0B
#define CMOS_REG_RTC_ACK 0x0C
#define CMOS_REG_CENTURY 0x32

/// Get the value of a CMOS register.
uint8_t get_cmos_reg(uint8_t reg_num);
/// Get the BCD-decoded value of a CMOS register.
uint8_t get_cmos_reg_bcd(uint8_t reg_num);
/// Set the value of a CMOS register.
void set_cmos_reg(uint8_t reg_num, uint8_t data);

#endif
