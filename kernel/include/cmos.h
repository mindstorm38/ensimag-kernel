#ifndef __CMOS_H__
#define __CMOS_H__

#include "stdbool.h"
#include "stdint.h"

#define CMOS_REG_SECONDS 0x00
#define CMOS_REG_MINUTES 0x02
#define CMOS_REG_HOURS 0x04
#define CMOS_REG_DAY_IN_WEEK 0x06
#define CMOS_REG_DAY_IN_MONTH 0x07
#define CMOS_REG_MONTH 0x08
#define CMOS_REG_YEAR 0x09
#define CMOS_REG_STATUS_A 0x0A
#define CMOS_REG_STATUS_B 0x0B
#define CMOS_REG_STATUS_C 0x0C
#define CMOS_REG_CENTURY 0x32


/// Enable non-maskable interrupts. This should be really carefully 
/// done and is typically used for setting up the RTC clock.
void cmos_nmi_enable(void);
/// Disable non-maskable interrupts.
void cmos_nmi_disable(void);


/// Get the value of a CMOS register.
uint8_t get_cmos_reg(uint8_t reg_num, bool mask_nmi);
/// Get the BCD-decoded value of a CMOS register.
uint8_t get_cmos_reg_bcd(uint8_t reg_num, bool mask_nmi);
/// Set the value of a CMOS register.
void set_cmos_reg(uint8_t reg_num, uint8_t data, bool mask_nmi);

#endif
