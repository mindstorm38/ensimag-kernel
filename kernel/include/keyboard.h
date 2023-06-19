#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#include "stdint.h"

#define KEYBOARD_LAYOUT_EN 0
#define KEYBOARD_LAYOUT_FR 1

/// Initialize keyboard interrupts.
void keyboard_init(void);

/// Select the layout of the keyboard, used to differentiate scancode.
void keyboard_select_layout(uint16_t layout);

#endif
