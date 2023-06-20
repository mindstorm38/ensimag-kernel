#include "ps2.h"
#include "cpu.h"

#include "stdio.h"

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

#define PS2_CONF_PORT_A_INT         0x01
#define PS2_CONF_PORT_B_INT         0x02
#define PS2_CONF_SYSTEM_FLAG        0x04
#define PS2_CONF_PORT_A_CLOCK       0x10
#define PS2_CONF_PORT_B_CLOCK       0x20
#define PS2_CONF_PORT_A_TRANSLATE   0x40


// My implementation follows the osdev wiki page about PS/2:
// https://wiki.osdev.org/%228042%22_PS/2_Controller


static inline uint8_t ps2_read_status() {
    return inb(PS2_STATUS_R);
}

/// Return true if output data can be read.
static inline bool ps2_output_ready() {
    return (ps2_read_status() & PS2_STATUS_OUTPUT_BUFFER) != 0;
}

/// Return true if input data can be written.
static inline bool ps2_input_ready() {
    return (ps2_read_status() & PS2_STATUS_INPUT_BUFFER) == 0;
}

static inline uint8_t ps2_read_data_unchecked() {
    return inb(PS2_DATA_RW);
}

static inline void ps2_write_data_unchecked(uint8_t data) {
    outb(data, PS2_DATA_RW);
}

static inline uint8_t ps2_config_read() {
    return ps2_command_read(PS2_CMD_READ_CONF);
}

static inline void ps2_config_write(uint8_t config) {
    ps2_command_write(PS2_CMD_WRITE_CONF, config);
}


/// Indicates if the PS/2 controller is ready to be used.
static bool ps2_ready_ = false;
/// Indicates if port B is available.
static bool ps2_dual = true;


void ps2_init() {

    printf("[    ] PS/2 driver init...");

    uint8_t conf, port_test;

    // Disable Devices
    ps2_command(PS2_CMD_DISABLE_PORT_A);
    ps2_command(PS2_CMD_DISABLE_PORT_B);

    // Flush The Output Buffer 
    if (ps2_output_ready())
        ps2_read_data_unchecked();

    // Set the Controller Configuration Byte
    conf = ps2_config_read();

    // Port B clock == 0 means enable, which is illegal at init so
    // we know port B isn't present. If not zero, we don't know.
    if ((conf & PS2_CONF_PORT_B_CLOCK) == 0) {
        ps2_dual = false;
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
    if (ps2_dual) {
        
        ps2_command(PS2_CMD_ENABLE_PORT_B);

        // Clock should be clear (0 == enabled), if not we are not
        // in dual channel controller.
        if ((ps2_config_read() & PS2_CONF_PORT_B_CLOCK) != 0) {
            ps2_dual = false;
        } else {
            // Disable it again because init is not done.
            ps2_command(PS2_CMD_DISABLE_PORT_B);
        }

    }

    // Perform Interface Tests 
    if ((port_test = ps2_command_read(PS2_CMD_TEST_PORT_A)) != 0) {
        printf("\r[\acFAIL\ar] PS/2 port 1 failed test, code: %d\n", port_test);
        return;
    }

    if (ps2_dual && (port_test = ps2_command_read(PS2_CMD_TEST_PORT_B)) != 0) {
        printf("\r[\acFAIL\ar] PS/2 port 2 failed test, code: %d\n", port_test);
        return;
    }

    // Enable Devices
    ps2_command(PS2_CMD_ENABLE_PORT_A);
    if (ps2_dual)
        ps2_command(PS2_CMD_ENABLE_PORT_B);

    conf = ps2_config_read();
    conf |= PS2_CONF_PORT_A_INT;
    if (ps2_dual)
        conf |= PS2_CONF_PORT_B_INT;
    ps2_config_write(conf);

    ps2_ready_ = true;
    printf("\r[ \aaOK\ar ] PS/2 driver ready.  \n");

}

bool ps2_ready() {
    return ps2_ready_;
}

uint8_t ps2_read_data() {
    while (!ps2_output_ready());
    return ps2_read_data_unchecked();
}

void ps2_write_data(uint8_t data) {
    while (!ps2_input_ready());
    ps2_write_data_unchecked(data);
}

void ps2_command(uint8_t command) {
    outb(command, PS2_CMD_W);
}

uint8_t ps2_command_read(uint8_t command) {
    ps2_command(command);
    return ps2_read_data();
}

void ps2_command_write(uint8_t command, uint8_t data) {
    ps2_command(command);
    ps2_write_data(data);
}


// bool ps2_test() {
//     return ps2_command_read(PS2_CMD_TEST) == 0x55;
// }

// bool ps2_test_port_a() {
//     return ps2_command_read(PS2_CMD_TEST_PORT_A) == 0;
// }

// bool ps2_test_port_b() {
//     return ps2_command_read(PS2_CMD_TEST_PORT_B) == 0;
// }

// void ps2_enable_port_a(bool enabled) {
//     ps2_command(enabled ? PS2_CMD_ENABLE_PORT_A : PS2_CMD_DISABLE_PORT_A);
// }

// void ps2_enable_port_b(bool enabled) {
//     ps2_command(enabled ? PS2_CMD_ENABLE_PORT_B : PS2_CMD_DISABLE_PORT_B);
// }
