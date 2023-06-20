#ifndef __PS2_H__
#define __PS2_H__

#include "stdint.h"
#include "stdbool.h"

/// Initialize the PS/2 controller driver.
void ps2_init(void);

/// Returns true if the PS/2 controller drive is ready to use.
bool ps2_ready(void);
bool ps2_dual(void);

/// Read status of the PS/2 controller.
uint8_t ps2_read_data(void);
void ps2_write_data(uint8_t data);

void ps2_command(uint8_t command);
uint8_t ps2_command_read(uint8_t command);
void ps2_command_write(uint8_t command, uint8_t data);

void ps2_send_port_a(uint8_t data);
void ps2_send_port_b(uint8_t data);

#endif
