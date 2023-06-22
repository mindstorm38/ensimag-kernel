/// This file contains code dispatching calls to the console to
/// specific peripherals.

#include "console.h"
#include "stdio.h"

#include "keyboard.h"
#include "string.h"
#include "memory.h"
#include "cons.h"
#include "cga.h"
#include <stddef.h>
#include <stdio.h>


// Variables related to writing a formatted character at particular
// location with particular color.
static enum cga_color fg_color = CGA_WHITE;
static enum cga_color bg_color = CGA_BLACK;
static uint32_t write_line = 0;
static uint32_t write_column = 0;
static bool waiting_format_code = false;

#define ALL_BUFFER_CAP 2048
static char all_buffer[ALL_BUFFER_CAP];
static size_t all_buffer_len = 0;

static cons_wake_t wake_func = NULL;

#define LINE_BUFFER_CAP 1024
static char line_buffer[LINE_BUFFER_CAP];
static size_t line_buffer_len = 0;
static size_t line_buffer_cursor = 0;
static size_t line_buffer_line = 0;
static size_t line_buffer_column = 0;
static bool line_buffer_insert = true;

/// True if the input keyboard should be echo-ed to the display.
static bool echo_ = true;


static void cons_write_char(char ch) {

    if (waiting_format_code && (ch >= '0' && ch <= '9')) {
        fg_color = ch - '0';
        waiting_format_code = false;
    } else if (waiting_format_code && (ch >= 'a' && ch <= 'f')) {
        fg_color = (ch - 'a') + 10;
        waiting_format_code = false;
    } else if (waiting_format_code && ch == 'r') {
        fg_color = CGA_WHITE;
        bg_color = CGA_BLACK;
        waiting_format_code = false;
    } else if (ch >= 32 && ch <= 126) {
        cga_set_char(write_line, write_column++, ch, fg_color, bg_color);
    } else if (ch == '\b') {
        if (write_column > 0) {
            write_column--;
        }
    } else if (ch == '\t') {
        write_column &= ~(0b1111);
        write_column += 8;
    } else if (ch == '\n') {
        write_line++;
        write_column = 0;
    } else if (ch == '\f') {
        cga_clear();
        write_line = 0;
        write_column = 0;
    } else if (ch == '\r') {
        write_column = 0;
    } else if (ch == '\a') {
        // We use the Bell character followed a color code for 
        // changing the formatting of the output.
        waiting_format_code = !waiting_format_code;
    }

    if (write_column >= CGA_COLS) {
        write_column = 0;
        write_line++;
    }

    while (write_line >= (CGA_ROWS - 1)) {
        cga_scroll_down();
        write_line--;
        line_buffer_line--;
    }

}

/// Echo the line buffer if needed.
static void cons_echo_line_buffer(void) {

    if (echo_) {

        write_line = line_buffer_line;
        write_column = line_buffer_column;

        if (line_buffer_cursor == 0) {
            cga_set_cursor(write_line, write_column);
        }

        for (size_t i = 0; i < line_buffer_len; i++) {
            cons_write_char(line_buffer[i]);
            if (i + 1 == line_buffer_cursor) {
                cga_set_cursor(write_line, write_column);
            }
        }

        cons_write_char(' ');

    }

}

/// Flush the line buffer to the global buffer.
static void cons_flush_line_buffer(void) {

    // -1 for one line break at the end of the line.
    size_t copy_len = ALL_BUFFER_CAP - all_buffer_len - 1;
    if (copy_len > line_buffer_len) {
        copy_len = line_buffer_len;
    }

    memcpy(all_buffer + all_buffer_len, line_buffer, copy_len);
    all_buffer_len += copy_len;
    all_buffer[all_buffer_len++] = '\n';

    line_buffer_len = 0;
    line_buffer_cursor = 0;

    if (wake_func != NULL) {
        cons_wake_t wake = wake_func;
        wake_func = NULL;
        wake();
    }

}

/// Internal function that is triggered when a character is received.
static void cons_char_handler(char ch) {

    if (line_buffer_cursor < LINE_BUFFER_CAP) {

        // When inserting, we need to increase move all characters
        // to right.
        if (line_buffer_insert) {

            // Cannot insert if length is already at its maximum cap.
            if (line_buffer_len >= LINE_BUFFER_CAP) {
                return;
            }

            line_buffer_len++;
            for (size_t i = line_buffer_len - 1; i > line_buffer_cursor; i--) {
                line_buffer[i] = line_buffer[i - 1];
            }

        }

        line_buffer[line_buffer_cursor++] = ch;

        if (line_buffer_cursor > line_buffer_len) {
            line_buffer_len = line_buffer_cursor;
        }

    }

    cons_echo_line_buffer();

}

/// Internal function that is triggered when a key is pressed.
static void cons_key_handler(enum keyboard_key key, uint32_t scancode, enum keyboard_action action, uint8_t mods) {

    (void) scancode;
    (void) mods;

    if (action == K_PRESS) {

        switch (key) {
        case K_BACKSPACE:
            if (line_buffer_cursor > 0) {
                
                line_buffer_cursor--;
                for (size_t i = line_buffer_cursor; i < line_buffer_len - 1; i++) {
                    line_buffer[i] = line_buffer[i + 1];
                }

                line_buffer_len--;
                cons_echo_line_buffer();

            }
            break;
        
        case K_DELETE:
            if (line_buffer_cursor < line_buffer_len) {
                for (size_t i = line_buffer_cursor; i < line_buffer_len - 1; i++) {
                    line_buffer[i] = line_buffer[i + 1];
                }
                line_buffer_len--;
                cons_echo_line_buffer();
            }
            break;

        case K_CURSOR_LEFT:
            if (line_buffer_cursor > 0) {
                line_buffer_cursor--;
                cons_echo_line_buffer();
            }
            break;
        
        case K_CURSOR_RIGHT:
            if (line_buffer_cursor < line_buffer_len) {
                line_buffer_cursor++;
                cons_echo_line_buffer();
            }
            break;
        
        case K_HOME:
            line_buffer_cursor = 0;
            cons_echo_line_buffer();
            break;
        
        case K_END:
            line_buffer_cursor = line_buffer_len;
            cons_echo_line_buffer();
            break;
        
        case K_INSERT:
        case K_KP_INSERT:
            line_buffer_insert = !line_buffer_insert;
            break;

        case K_ENTER:
            cons_flush_line_buffer();
            if (echo_)
                cons_write("\n", 1);
            break;

        default:
            break;
        }

    }

}

// Public functions

void cons_start(void) {
    keyboard_set_key_handler(cons_key_handler);
    keyboard_set_char_handler(cons_char_handler);
}

void cons_echo(bool echo) {
    echo_ = echo;
}

void cons_write(const char *src, size_t len) {

    while (len > 0) {
        cons_write_char(*src);
        src++;
        len--;
    }

    line_buffer_line = write_line;
    line_buffer_column = write_column;

    cga_set_cursor(write_line, write_column);

}

bool cons_try_read(char *dst, size_t *len, cons_wake_t wake) {

    size_t max_len = *len;

    if (max_len == 0)
        return true;

    if (all_buffer_len == 0) {
        if (wake != NULL)
            wake_func = wake;
        return false;
    }

    // Cannot get more length than available.
    if (max_len > all_buffer_len) {
        max_len = all_buffer_len;
    }

    // Copy each character one by one.
    size_t read_len = 0;
    bool read_eol = false;

    for (; !read_eol && read_len < max_len; read_len++) {
        char ch = all_buffer[read_len];
        if (ch == '\n') {
            read_eol = true;
        } else {
            dst[read_len] = ch;
        }
    }

    all_buffer_len -= read_len;

    // Remove the read part.
    for (size_t j = 0; j < all_buffer_len; j++) {
        all_buffer[j] = all_buffer[j + read_len];
    }

    if (read_eol)
        read_len--;
    
    *len = read_len;
    return true;

}

/// This function is an alias for 'cons_write', used by given 
/// stdio library.
void console_putbytes(const char *s, int32_t len) {
    if (len > 0) {
        cons_write(s, len);
    }
}
