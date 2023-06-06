#ifndef __CGA_H__
#define __CGA_H__

#include "stdint.h"


#define CONSOLE_COLS 80
#define CONSOLE_ROWS 25


/// Colors available for foreground or background.
enum cga_color {
    CGA_BLACK = 0,
    CGA_BLUE = 1,
    CGA_GREEN = 2,
    CGA_CYAN = 3,
    CGA_RED = 4,
    CGA_MAGENTA = 5,
    CGA_BROWN = 6,
    CGA_GRAY = 7,
    CGA_DARK_GRAY = 8,
    CGA_LIGHT_BLUE = 9,
    CGA_LIGHT_GREEN = 10,
    CGA_LIGHT_CYAN = 11,
    CGA_LIGHT_RED = 12,
    CGA_LIGHT_MAGENTA = 13,
    CGA_YELLOW = 14,
    CGA_WHITE = 15
};

void cga_write_bytes(const char *s, int32_t len);
void cga_fg_color(enum cga_color color);
void cga_bg_color(enum cga_color color);

#endif
