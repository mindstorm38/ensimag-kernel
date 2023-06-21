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

// static enum cga_color fg_color = CGA_WHITE;
// static enum cga_color bg_color = CGA_BLACK;
// /// True when waiting for a formatting color code.
// static bool waiting_format_code = false;


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


// /// This function writes the given character at given position with
// /// particular foreground and background colors.
// static inline void char_write(uint32_t line, uint32_t column, char ch, enum cga_color fg, enum cga_color bg) {
//     uint16_t *ptr = char_pointer(line, column);
//     *ptr = ((fg & 0b1111) << 8) | ((bg & 0b111) << 12) | ((uint16_t) ch);
// }

// /// Position the cursor.
// static void cursor_pos(uint32_t line, uint32_t column) {
//     uint16_t index = column + line * 80;
//     outb(CGA_CMD_CURSOR_LO, CGA_CMD);
//     outb((uint8_t) (index & 0xFFFF), CGA_DATA);
//     outb(CGA_CMD_CURSOR_HI, CGA_CMD);
//     outb((uint8_t) ((index >> 8) & 0xFFFF), CGA_DATA);
// }

// /// Clear the screen.
// static void screen_clear(void) {
//     uint8_t *ptr = (uint8_t *) 0xB8000;
//     while (ptr < (uint8_t *) (0xB8000 + (CGA_COLS * CGA_ROWS * 2))) {
//         *ptr = ' ';
//         ptr++;
//         *ptr = 0b00001111;
//         ptr++;
//     }
// }

// /// Scroll down.
// static void scroll_down(void) {
    
//     uint8_t *first_line = (uint8_t *) 0xB8000;
//     uint8_t *second_line = (uint8_t *) (0xB8000 + 160);
//     uint8_t *last_line = (uint8_t *) (0xB8000 + 4000 - 160);

//     memmove(first_line, second_line, 4000 - 160);
//     memset(last_line, 0, 160);

// }

// static void char_write_formatted(char c) {

//     if (waiting_format_code && (c >= '0' && c <= '9')) {
//         fg_color = c - '0';
//         waiting_format_code = false;
//     } else if (waiting_format_code && (c >= 'a' && c <= 'f')) {
//         fg_color = (c - 'a') + 10;
//         waiting_format_code = false;
//     } else if (waiting_format_code && c == 'r') {
//         fg_color = CGA_WHITE;
//         bg_color = CGA_BLACK;
//         waiting_format_code = false;
//     } else if (c >= 32 && c <= 126) {
//         char_write(lig, col++, c, fg_color, bg_color);
//     } else if (c == '\b') {
//         if (col > 0) {
//             col--;
//         }
//     } else if (c == '\t') {
//         col &= ~(0b1111);
//         col += 8;
//     } else if (c == '\n') {
//         lig++;
//         col = 0;
//     } else if (c == '\f') {
//         screen_clear();
//         lig = 0;
//         col = 0;
//     } else if (c == '\r') {
//         col = 0;
//     } else if (c == '\a') {
//         // We use the Bell character followed a color code for 
//         // changing the formatting of the output.
//         waiting_format_code = !waiting_format_code;
//     }

//     if (col >= CGA_COLS) {
//         col = 0;
//         lig++;
//     }

//     while (lig >= (CGA_ROWS - 1)) {
//         scroll_down();
//         lig--;
//     }

//     cursor_pos(lig, col);

// }

// void cga_write(const char *src, size_t len) {
//     while (len > 0) {
//         char_write_formatted(*src);
//         src++;
//         len--;
//     }
// }

// void cga_fg_color(enum cga_color color) {
//     fg_color = color;
// }

// void cga_bg_color(enum cga_color color) {
//     bg_color = color;
// }
