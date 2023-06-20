#ifndef __PS2_H__
#define __PS2_H__

#include "stdint.h"
#include "stdbool.h"


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
/// Get a device identifier for the given port.
uint16_t ps2_device_id(enum ps2_port port);
/// Set the IRQ handler for the given device port.
void ps2_device_set_handler(enum ps2_port port, ps2_device_handler_t handler);
/// Send a PS/2 command to a device and wait for a response.
/// Some commands returns aa second byte after this command.
///
/// This function automatically disables IRQs for the port, to avoid
/// the return values of being caught by handlers.
uint8_t ps2_device_command(enum ps2_port port, uint8_t command, bool follow_ack);

#endif
