#include "ps2.h"
#include "cpu.h"

#define PS2_DATA_RW    0x0060
#define PS2_STATUS_R   0x0064
#define PS2_CMD_W      0x0064

#define PS2_STATUS_OUTPUT_BUFFER 0x01
#define PS2_STATUS_INPUT_BUFFER  0x02
#define PS2_STATUS_SYSTEM_FLAG   0x04
#define PS2_STATUS_CMD_DATA      0x08
#define PS2_STATUS_TIMEOUT_ERR   0x40
#define PS2_STATUS_PARITY_ERR    0x80


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

void ps2_command_write(uint8_t command, uint8_t data) {
    ps2_command(command);
    ps2_write_data(data);
}

uint8_t ps2_command_read(uint8_t command) {
    ps2_command(command);
    return ps2_read_data();
}
