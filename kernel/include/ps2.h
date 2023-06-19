#ifndef __PS2_H__
#define __PS2_H__

#include "stdint.h"

/// Read status of the PS/2 controller.
uint8_t ps2_read_data();
void ps2_write_data(uint8_t data);

void ps2_command(uint8_t command);
void ps2_command_write(uint8_t command, uint8_t data);
uint8_t ps2_command_read(uint8_t command);

#endif
