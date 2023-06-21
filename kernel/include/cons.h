#ifndef __CONS_H__
#define __CONS_H__

#include "stdbool.h"
#include "stddef.h"

/// Start the console mode, this is not required to write to the
/// console, but this is required to read from. It allows the user
void cons_start(void);

/// Write the given source characters of the given length to the 
/// display.
void cons_write(const char *src, size_t len);

/// Enable or not the echo of the keyboard input to the console.
void cons_echo(bool echo);

/// Read from the console, only valid after 'cons_start' was called.
size_t cons_read(char *dst, size_t len);

#endif
