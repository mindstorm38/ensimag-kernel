#ifndef __PS2_H__
#define __PS2_H__

#include "stdint.h"
#include "stdbool.h"

#define PS2_CONF_PORT_A_INT         0x01
#define PS2_CONF_PORT_B_INT         0x02
#define PS2_CONF_SYSTEM_FLAG        0x04
#define PS2_CONF_PORT_A_CLOCK       0x10
#define PS2_CONF_PORT_B_CLOCK       0x20
#define PS2_CONF_PORT_A_TRANSLATE   0x40

/// Initialize the PS/2 controller driver.
void ps2_init();

/// Read status of the PS/2 controller.
uint8_t ps2_read_data();
void ps2_write_data(uint8_t data);

void ps2_command(uint8_t command);
uint8_t ps2_command_read(uint8_t command);
void ps2_command_write(uint8_t command, uint8_t data);

uint8_t ps2_config_read();
void ps2_config_write(uint8_t config);

/// Test the PS/2 controller, return true if test passed.
bool ps2_test();
/// Test the PS/2 first port, return true if test passed.
bool ps2_test_port_a();
/// Test the PS/2 second port, return true if test passed.
bool ps2_test_port_b();

/// Set the PS/2 first port enabled or not.
void ps2_enable_port_a(bool enabled);
/// Set the PS/2 second port enabled or not.
void ps2_enable_port_b(bool enabled);

#endif
