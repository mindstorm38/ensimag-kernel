/// This file contains code dispatching calls to the console to
/// specific peripherals.

#include "console.h"
#include "cga.h"


/// This function forward the console write to the correct peripheral.
void console_putbytes(const char *s, int32_t len) {
    cga_write_bytes(s, len);
}
