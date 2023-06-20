#ifndef __PS2_H__
#define __PS2_H__

#include "stdbool.h"
#include "stdint.h"


#define PS2_DEV_RES_ACK         0xFA
#define PS2_DEV_RES_RESEND      0xFE

#define PS2_DEV_MOUSE           0x0000
#define PS2_DEV_KEYBOARD_MF2    0x83AB
#define PS2_DEV_INVALID         0xFFFF


/// The PS/2 port to select.
enum ps2_port {
    PS2_FIRST,
    PS2_SECOND,
};

/// Type lias for PS/2 device handler.
typedef void (*ps2_device_handler_t)(uint8_t);


/// Initialize the PS/2 controller driver.
void ps2_init(void);
/// Return true if the PS/2 controller drive is ready to use.
bool ps2_ready(void);
/// Return true if the PS/2 controller supports dual devices.
bool ps2_dual(void);

/// Get a device identifier for the given port. This function returns
/// 'PS2_DEV_INVALID' if this device port is not available.
uint16_t ps2_device_id(enum ps2_port port);
/// Set the IRQ handler for the given device port, the data port is
/// automatically read and forwarded to the handler.
void ps2_device_set_handler(enum ps2_port port, ps2_device_handler_t handler);
/// Send a PS/2 command to a device and wait for a response. Some 
/// commands uses a second byte, this is why the command is two byte.
/// If a subcommand is being used, the subcommand is placed in the
/// most significant byte. If the follow ack boolean is true, and if
/// the command returns ACK, the next byte is read and returned.
uint8_t ps2_device_command(enum ps2_port port, uint16_t command, bool subcommand, bool follow_ack);

#endif
