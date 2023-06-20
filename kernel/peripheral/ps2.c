#include "interrupt.h"
#include "ps2.h"
#include "cpu.h"

#include "stdio.h"
#include <stdint.h>

#define PS2_DATA_RW    0x0060
#define PS2_STATUS_R   0x0064
#define PS2_CMD_W      0x0064

#define PS2_STATUS_OUTPUT_BUFFER 0x01
#define PS2_STATUS_INPUT_BUFFER  0x02
#define PS2_STATUS_SYSTEM_FLAG   0x04
#define PS2_STATUS_CMD_DATA      0x08
#define PS2_STATUS_TIMEOUT_ERR   0x40
#define PS2_STATUS_PARITY_ERR    0x80

#define PS2_CMD_READ_CONF           0x20
#define PS2_CMD_WRITE_CONF          0x60
#define PS2_CMD_DISABLE_PORT_B      0xA7
#define PS2_CMD_ENABLE_PORT_B       0xA8
#define PS2_CMD_TEST_PORT_B         0xA9
#define PS2_CMD_TEST                0xAA
#define PS2_CMD_TEST_PORT_A         0xAB
#define PS2_CMD_DISABLE_PORT_A      0xAD
#define PS2_CMD_ENABLE_PORT_A       0xAE
#define PS2_CMD_SEND_PORT_B         0xD4

#define PS2_CONF_PORT_A_INT         0x01
#define PS2_CONF_PORT_B_INT         0x02
#define PS2_CONF_SYSTEM_FLAG        0x04
#define PS2_CONF_PORT_A_CLOCK       0x10
#define PS2_CONF_PORT_B_CLOCK       0x20
#define PS2_CONF_PORT_A_TRANSLATE   0x40

// My implementation follows the osdev wiki page about PS/2:
// https://wiki.osdev.org/%228042%22_PS/2_Controller


/// Indicates if the PS/2 controller is ready to be used.
static bool ps2_ready_ = false;
/// Indicates if port B is available.
static bool ps2_dual_ = true;

static uint16_t ps2_port_ids[2] = { PS2_DEV_INVALID, PS2_DEV_INVALID };
static ps2_device_handler_t ps2_device_handlers[2] = {0};


static inline uint8_t ps2_read_status(void) {
    return inb(PS2_STATUS_R);
}

/// Return true if output data can be read.
static inline bool ps2_read_ready(void) {
    return (ps2_read_status() & PS2_STATUS_OUTPUT_BUFFER) != 0;
}

/// Return true if input data can be written.
static inline bool ps2_write_ready(void) {
    return (ps2_read_status() & PS2_STATUS_INPUT_BUFFER) == 0;
}

static inline uint8_t ps2_read_data_unchecked(void) {
    return inb(PS2_DATA_RW);
}

static inline void ps2_write_data_unchecked(uint8_t data) {
    outb(data, PS2_DATA_RW);
}

static uint8_t ps2_read_data(void) {
    while (!ps2_read_ready());
    return ps2_read_data_unchecked();
}

static void ps2_write_data(uint8_t data) {
    while (!ps2_write_ready());
    ps2_write_data_unchecked(data);
}

/// Basic function to send the given command to the PS/2 controller.
static inline void ps2_command(uint8_t command) {
    outb(command, PS2_CMD_W);
}

/// Basic function to send the given command to the PS/2 controller
/// and wait for a response.
static inline uint8_t ps2_command_read(uint8_t command) {
    ps2_command(command);
    return ps2_read_data();
}

/// Basic function to send the given command to the PS/2 controller
/// followed by another data byte.
static inline void ps2_command_write(uint8_t command, uint8_t data) {
    ps2_command(command);
    ps2_write_data(data);
}

/// Read the PS/2 controller config byte.
static inline uint8_t ps2_config_read(void) {
    return ps2_command_read(PS2_CMD_READ_CONF);
}

/// Write the PS/2 controller config byte.
static inline void ps2_config_write(uint8_t config) {
    ps2_command_write(PS2_CMD_WRITE_CONF, config);
}

/// Send a PS/2 command to a device and wait for a response.
/// Some commands returns aa second byte after this command.
///
/// This function is "unchecked", which means that IRQs should not
/// be enabled for the given port. Otherwise they will catch return
/// values.
static uint8_t ps2_device_command_unchecked(enum ps2_port port, uint8_t command) {
    uint8_t data;
    do {
        if (port == PS2_SECOND) {
            ps2_command(PS2_CMD_SEND_PORT_B);
        }
        ps2_write_data(command);
        data = ps2_read_data();
    } while (data == PS2_DEV_RES_RESEND);
    return data;
}

/// Send the identify command to the given device port and read the
/// device identifier. Should be called when IRQs are disabled, in
/// the init code.
static uint16_t ps2_device_identify(enum ps2_port port) {

    // Disable scanning.
    if (ps2_device_command_unchecked(port, 0xF5) != PS2_DEV_RES_ACK)
        return PS2_DEV_INVALID;
    
    // Identify command.
    if (ps2_device_command_unchecked(port, 0xF2) != PS2_DEV_RES_ACK)
        return PS2_DEV_INVALID;

    // Read identity.
    uint16_t id = 0;
    for (int i = 0, shift = 0; i < 1000 && shift < 16; i++) {
        if (ps2_read_ready()) {
            id |= ((uint16_t) ps2_read_data_unchecked()) << shift;
            shift += 8;
        }
    }

    // Enable scanning.
    if (ps2_device_command_unchecked(port, 0xF4) != PS2_DEV_RES_ACK)
        return PS2_DEV_INVALID;

    return id;

}

/// IRQ handler for PS/2 port A.
static void ps2_port_a_handler(void) {
    irq_eoi(IRQ_PS2_A);
    ps2_device_handler_t handler = ps2_device_handlers[PS2_FIRST];
    uint8_t data = ps2_read_data_unchecked();
    // We check that we don't get a ACK, because the interrupt
    // may be triggered because of device command done in kernel
    // mode.
    // printf("data: 0x%02X\n", data);
    if (handler != NULL && data != PS2_DEV_RES_ACK)
        handler(data);
}

/// IRQ handler for PS/2 port B.
static void ps2_port_b_handler(void) {
    irq_eoi(IRQ_PS2_B);
    ps2_device_handler_t handler = ps2_device_handlers[PS2_SECOND];
    uint8_t data = ps2_read_data_unchecked();
    if (handler != NULL && data != PS2_DEV_RES_ACK)
        handler(data);
}


void ps2_init(void) {

    printf("[    ] PS/2 driver init...");

    uint8_t conf, port_test;
    uint16_t id;

    // Disable Devices
    ps2_command(PS2_CMD_DISABLE_PORT_A);
    ps2_command(PS2_CMD_DISABLE_PORT_B);

    // Flush The Output Buffer 
    if (ps2_read_ready())
        ps2_read_data_unchecked();

    // Set the Controller Configuration Byte
    conf = ps2_config_read();

    // Port B clock == 0 means enable, which is illegal at init so
    // we know port B isn't present. If not zero, we don't know.
    if ((conf & PS2_CONF_PORT_B_CLOCK) == 0) {
        ps2_dual_ = false;
    }

    // Disable IRQs while setup and translation to code set 2 from 1.
    conf &= ~PS2_CONF_PORT_A_INT;
    conf &= ~PS2_CONF_PORT_B_INT;
    conf &= ~PS2_CONF_PORT_A_TRANSLATE;
    
    ps2_config_write(conf);

    // Perform Controller Self Test
    if ((port_test = ps2_command_read(PS2_CMD_TEST)) != 0x55) {
        printf("\r[\acFAIL\ar] PS/2 driver failed self test, code: %d\n", port_test);
        return;
    }

    // Ensure that configuration is still up-to-date.
    ps2_config_write(conf);

    // Determine If There Are 2 Channels.
    if (ps2_dual_) {
        
        ps2_command(PS2_CMD_ENABLE_PORT_B);

        // Clock should be clear (0 == enabled), if not we are not
        // in dual channel controller.
        if ((ps2_config_read() & PS2_CONF_PORT_B_CLOCK) != 0) {
            ps2_dual_ = false;
        } else {
            // Disable it again because init is not done.
            ps2_command(PS2_CMD_DISABLE_PORT_B);
        }

    }

    // Perform Interface Tests 
    if ((port_test = ps2_command_read(PS2_CMD_TEST_PORT_A)) != 0) {
        printf("\r[\acFAIL\ar] PS/2 first port failed test, code: %d\n", port_test);
        return;
    }

    if (ps2_dual_ && (port_test = ps2_command_read(PS2_CMD_TEST_PORT_B)) != 0) {
        printf("\r[\acFAIL\ar] PS/2 second port failed test, code: %d\n", port_test);
        return;
    }

    // Enable Devices
    ps2_command(PS2_CMD_ENABLE_PORT_A);
    if (ps2_dual_)
        ps2_command(PS2_CMD_ENABLE_PORT_B);

    // Identify Devices
    if ((id = ps2_device_identify(PS2_FIRST)) == PS2_DEV_INVALID) {
        printf("\r[\acFAIL\ar] PS/2 failed to identify first device\n");
    } else {
        ps2_port_ids[PS2_FIRST] = id;
    }

    if (ps2_dual_ && (id = ps2_device_identify(PS2_SECOND)) == PS2_DEV_INVALID) {
        printf("\r[\acFAIL\ar] PS/2 failed to identify second device\n");
    } else if (ps2_dual_) {
        ps2_port_ids[PS2_SECOND] = id;
    }

    // Re-enable IRQs generation.
    conf = ps2_config_read();
    conf |= PS2_CONF_PORT_A_INT;
    if (ps2_dual_)
        conf |= PS2_CONF_PORT_B_INT;
    ps2_config_write(conf);

    // Setup interrupts handlers
    irq_set_handler(IRQ_PS2_A, ps2_port_a_handler);
    irq_mask(IRQ_PS2_A, false);

    if (ps2_dual_) {
        irq_set_handler(IRQ_PS2_B, ps2_port_b_handler);
        irq_mask(IRQ_PS2_B, false);
    }

    ps2_ready_ = true;
    printf("\r[ \aaOK\ar ] PS/2 driver ready%s     \n", ps2_dual_ ? " (dual channel)" : "");
    printf("       first id: 0x%04X, second id: 0x%04X\n", ps2_port_ids[PS2_FIRST], ps2_port_ids[PS2_SECOND]);

}

bool ps2_ready(void) {
    return ps2_ready_;
}

bool ps2_dual(void) {
    return ps2_dual_;
}

uint16_t ps2_device_id(enum ps2_port port) {
    return ps2_port_ids[port];
}

void ps2_device_set_handler(enum ps2_port port, ps2_device_handler_t handler) {
    assert(handler != NULL);
    ps2_device_handlers[port] = handler;
}

uint8_t ps2_device_command(enum ps2_port port, uint8_t command, bool follow_ack) {
    uint8_t data = ps2_device_command_unchecked(port, command);
    if (follow_ack && data == PS2_DEV_RES_ACK) {
        return ps2_read_data();
    } else {
        return data;
    }
}
