#ifndef __PS2_H__
#define __PS2_H__

#include "stdint.h"
#include "stdbool.h"

/// The PS/2 port to select.
enum ps2_port {
    PS2_FIRST,
    PS2_SECOND,
};

/// Initialize the PS/2 controller driver.
void ps2_init(void);
/// Return true if the PS/2 controller drive is ready to use.
bool ps2_ready(void);
/// Return true if the PS/2 controller supports dual devices.
bool ps2_dual(void);
/// Get a device identifier for the given port.
uint16_t ps2_device_id(enum ps2_port port);


// /// Read status of the PS/2 controller.
// uint8_t ps2_read_data(void);
// void ps2_write_data(uint8_t data);

// void ps2_command(uint8_t command);
// uint8_t ps2_command_read(uint8_t command);
// void ps2_command_write(uint8_t command, uint8_t data);

// void ps2_send_port_a(uint8_t data);
// void ps2_send_port_b(uint8_t data);

#endif
