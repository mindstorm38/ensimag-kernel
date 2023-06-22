#include "stdbool.h"
#include "string.h"
#include "stdint.h"

#include "cpu.h"
#include "cga.h"

#define CGA_CMD             0x03D4
#define CGA_CMD_CURSOR_LO   0x0F
#define CGA_CMD_CURSOR_HI   0x0E
#define CGA_DATA            0x03D5


static uint32_t line_ = 0;
static uint32_t column_ = 0;


/// This function get the pointer to the character at given position.
static inline uint16_t *char_pointer(uint32_t line, uint32_t column) {
    return (uint16_t *) 0xB8000 + (line * 80 + column);
}


void cga_char(uint32_t line, uint32_t column, char *ch, enum cga_color *fg, enum cga_color *bg) {
    uint16_t data = *char_pointer(line, column);
    *ch = data & 0xFF;
    *fg = (data >> 8) & 0b1111;
    *bg = (data >> 12) & 0b111;
}

void cga_set_char(uint32_t line, uint32_t column, char ch, enum cga_color fg, enum cga_color bg) {
    *char_pointer(line, column) = ((fg & 0b1111) << 8) | ((bg & 0b111) << 12) | ((uint16_t) ch);
}

void cga_cursor(uint32_t *line, uint32_t *column) {
    *line = line_;
    *column = column_;
}

void cga_set_cursor(uint32_t line, uint32_t column) {

    line_ = line;
    column_ = column;

    uint16_t index = column + line * 80;
    outb(CGA_CMD_CURSOR_LO, CGA_CMD);
    outb((uint8_t) (index & 0xFFFF), CGA_DATA);
    outb(CGA_CMD_CURSOR_HI, CGA_CMD);
    outb((uint8_t) ((index >> 8) & 0xFFFF), CGA_DATA);

}

void cga_clear(void) {
    uint8_t *ptr = (uint8_t *) 0xB8000;
    while (ptr < (uint8_t *) (0xB8000 + (CGA_COLS * CGA_ROWS * 2))) {
        *ptr = ' ';
        ptr++;
        *ptr = 0b00001111;
        ptr++;
    }
}

void cga_scroll_down(void) {
    
    uint8_t *first_line = (uint8_t *) 0xB8000;
    uint8_t *second_line = (uint8_t *) (0xB8000 + 160);
    uint8_t *last_line = (uint8_t *) (0xB8000 + 4000 - 160);

    memmove(first_line, second_line, 4000 - 160);
    memset(last_line, 0, 160);

}
