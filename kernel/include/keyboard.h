#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#include "stdint.h"


enum keyboard_layout {
    KEYBOARD_LAYOUT_FR,
    KEYBOARD_LAYOUT_COUNT,
};


/// Initialize keyboard interrupts.
void keyboard_init(void);

/// Select the layout of the keyboard, used to differentiate scancode.
void keyboard_select_layout(enum keyboard_layout layout);

#endif
