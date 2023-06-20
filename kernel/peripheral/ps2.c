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


void ps2_init() {

    printf("[    ] PS/2 driver init...");

    printf("\r[ OK ] PS/2 driver ready.  \n");

}


static uint8_t ps2_read_status() {
    return inb(PS2_STATUS_R);
}

uint8_t ps2_read_data() {
    while ((ps2_read_status() & PS2_STATUS_OUTPUT_BUFFER) == 0);
    return inb(PS2_DATA_RW);
}

void ps2_write_data(uint8_t data) {
    while ((ps2_read_status() & PS2_STATUS_INPUT_BUFFER) != 0);
    outb(data, PS2_DATA_RW);
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

uint8_t ps2_config_read() {
    return ps2_command_read(PS2_CMD_READ_CONF);
}

void ps2_config_write(uint8_t config) {
    ps2_command_write(PS2_CMD_WRITE_CONF, config);
}


bool ps2_test() {
    return ps2_command_read(PS2_CMD_TEST) == 0x55;
}

bool ps2_test_port_a() {
    return ps2_command_read(PS2_CMD_TEST_PORT_A) == 0;
}

bool ps2_test_port_b() {
    return ps2_command_read(PS2_CMD_TEST_PORT_B) == 0;
}

void ps2_enable_port_a(bool enabled) {
    ps2_command(enabled ? PS2_CMD_ENABLE_PORT_A : PS2_CMD_DISABLE_PORT_A);
}

void ps2_enable_port_b(bool enabled) {
    ps2_command(enabled ? PS2_CMD_ENABLE_PORT_B : PS2_CMD_DISABLE_PORT_B);
}
