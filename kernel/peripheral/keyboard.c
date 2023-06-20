#include "stdbool.h"
#include "stddef.h"
#include "stdio.h"

#include "interrupt.h"
#include "keyboard.h"
#include "cpu.h"
#include "ps2.h"
#include <stdint.h>


/// Following tables are defining conversion from scancode to ascii characters, returning zero if not a visual character.
/// For each layout, there are 3 tables: [default, shift, altgr]
struct keyboard_layout_impl {
    enum keyboard_layout id;
    char initial[256];
    char shift[256];
    char alt_graph[256];
};

/// Definition of all layouts.
static struct keyboard_layout_impl all_layouts[KEYBOARD_LAYOUT_COUNT] = {
    {
        KEYBOARD_LAYOUT_FR,
        {0, 0, 0, 0, 0, 0, },
        {0},
        {0}
    }
};

// static struct keyboard_layout KEYBOARD_LAYOUT_FR_ = {
//     KEYBOARD_LAYOUT_FR,
//     {0, 0, '&', 'e', '"', '\'', '(', '-', 'e', '_', 'c', 'a', ')', '=', 0, 0, 'a', 'z', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '^', '$', 0, 0, 'q', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 'm', 'u', '*', 0, '\\', 'w', 'x', 'c', 'v', 'b', 'n', ',', ';', ':', '!', 0, 0, 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//     {0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 0, '+', 0, 0, 'A', 'Z', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', 0, 0, 0, 0, 'Q', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', 'M', '%', 'u', 0, '\\', 'W', 'X', 'C', 'V', 'B', 'N', '?', '.', '/', 0, 0, 0, 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//     {0, 0, 0, '~', '#', '{', '[', '|', '`', '\\', '^', '@', ']', '}', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '~', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '`', 0, '\\', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
// };

static struct keyboard_layout_impl *current_layout;

// static uint8_t last_scancode = 0;
// static bool shift = false;
// static bool ctrl = false;
// static bool alt = false;
// static bool altgr = false;


static uint8_t last_scancode = 0;


static void keyboard_handler(uint8_t scancode) {

    printf("0x%02X\n", scancode);

    // bool break_scancode = false;

    // if (last_scancode == 0xF0) {
    //     break_scancode = true;
    // }

    last_scancode = scancode;

}


void keyboard_init(void) {

    printf("[    ] Keyboard driver init...");

    if (!ps2_ready()) {
        printf("\r[\acFAIL\ar] Keyboard driver failed because PS/2 driver has failed\n");
        return;
    }

    if (ps2_device_id(PS2_FIRST) != PS2_DEV_KEYBOARD_MF2) {
        printf("\r[\acFAIL\ar] Keyboard driver failed because no PS/2 keyboard detected: %d\n", ps2_device_id(PS2_FIRST));
        return;
    }

    // Select scan code set 2.
    if (ps2_device_command(PS2_FIRST, 0x02F0, true, false) != PS2_DEV_RES_ACK) {
        printf("\r[\acFAIL\ar] Keyboard driver failed to set scancode set 2\n");
        return;
    }

    // Check that the scan code was set to 2.
    if (ps2_device_command(PS2_FIRST, 0x00F0, true, true) != 0x02) {
        printf("\r[\acFAIL\ar] Keyboard driver failed to set scancode set 2\n");
        return;
    }

    keyboard_select_layout(KEYBOARD_LAYOUT_FR);
    ps2_device_set_handler(PS2_FIRST, keyboard_handler);

    printf("\r[ \aaOK\ar ] Keyboard driver ready       \n");

}

void keyboard_select_layout(enum keyboard_layout layout) {
    current_layout = &all_layouts[layout];
}
