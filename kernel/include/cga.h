#ifndef __CGA_H__
#define __CGA_H__

#include "stdint.h"
#include "stddef.h"

#define CGA_COLS 80
#define CGA_ROWS 25


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

/// Get a character and its color at the given position.
void cga_char(uint32_t line, uint32_t column, char *ch, enum cga_color *fg, enum cga_color *bg);
/// Write the given character at the given position with given color.
void cga_set_char(uint32_t line, uint32_t column, char ch, enum cga_color fg, enum cga_color bg);

/// Get the cursor position.
void cga_cursor(uint32_t *line, uint32_t *column);
/// Set the cursor position.
void cga_set_cursor(uint32_t line, uint32_t column);

/// Clear the screen.
void cga_clear(void);
/// Scroll down the screen.
void cga_scroll_down(void);

#endif
