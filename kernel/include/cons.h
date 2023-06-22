#ifndef __CONS_H__
#define __CONS_H__

#include "stdbool.h"
#include "stddef.h"

typedef void (*cons_wake_t)(void);

/// Start the console mode, this is not required to write to the
/// console, but this is required to read from. It allows the user
void cons_start(void);

/// Enable or not the echo of the keyboard input to the console.
void cons_echo(bool echo);

/// Write the given source characters of the given length to the 
/// display.
void cons_write(const char *src, size_t len);

/// This function tries to read characters from the console's global
/// buffer. If 'len' characters are available on the next line, then 
/// these characters copied, if an end-of-line '\n' is encountered
/// before reaching 'len', the full line is copied (but '\n' is not
/// returned).
///
/// If given 'len' is 0, true is returned and length is unchanged.
/// 
/// If not enough data is available this function returns false. If 
/// the given 'wake' function pointer is not null, it will be called 
/// when enough data is available.
bool cons_try_read(char *dst, size_t *len, cons_wake_t wake);

#endif
