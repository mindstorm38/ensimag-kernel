#include "stdbool.h"
#include "stddef.h"
#include "stdio.h"

#include "interrupt.h"
#include "keyboard.h"
#include "cpu.h"
#include "ps2.h"


/// Key for scancode set 2 in normal code mode.
static enum keyboard_key normal[256] = { 0, K_F9, 0, K_F5, K_F3, K_F1, K_F2, K_F12, 0, K_F10, K_F8, K_F6, K_F4, K_TAB, K_BACK_TICK, 0, 0, K_LEFT_ALT, K_LEFT_SHIFT, 0, K_LEFT_CTRL, K_Q, K_1, 0, 0, 0, K_Z, K_S, K_A, K_W, K_2, 0, 0, K_C, K_X, K_D, K_E, K_4, K_3, 0, 0, K_SPACE, K_V, K_F, K_T, K_R, K_5, 0, 0, K_N, K_B, K_H, K_G, K_Y, K_6, 0, 0, 0, K_M, K_J, K_U, K_7, K_8, 0, 0, K_COMMA, K_K, K_I, K_O, K_0, K_9, 0, 0, K_PERIOD, K_SLASH, K_L, K_SEMICOLON, K_P, K_DASH, 0, 0, 0, K_APOSTROPHE, 0, K_OPEN_BRACKET, K_EQUAL, 0, 0, K_CAPS_LOCK, K_RIGHT_SHIFT, K_ENTER, K_CLOSE_BRACKET, 0, K_BACKSLASH, 0, 0, 0, 0, 0, 0, 0, 0, K_BACKSPACE, 0, 0, K_KP_1, 0, K_KP_4, K_KP_7, 0, 0, 0, K_KP_0, K_KP_PERIOD, K_KP_2, K_KP_5, K_KP_6, K_KP_8, K_ESCAPE, K_NUM_LOCK, K_F11, K_KP_ADD, K_KP_3, K_KP_SUBTRACT, K_KP_MULTIPLY, K_KP_9, K_SCROLL_LOCK, 0, 0, 0, 0, K_F7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
/// Key for scancode set 2 in extended code mode.
static enum keyboard_key extended[256] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, K_WWW_SEARCH, K_RIGHT_ALT, K_PRINT_SCREEN, 0, K_RIGHT_CTRL, K_PREVIOUS_TRACK, 0, 0, K_WWW_FAVOURITES, 0, 0, 0, 0, 0, 0, K_LEFT_GUI, K_WWW_REFRESH, K_VOLUME_DOWN, 0, K_VOLUME_MUTE, 0, 0, 0, K_RIGHT_GUI, K_WWW_STOP, 0, 0, K_CALCULATOR, 0, 0, 0, K_APPS, K_WWW_FORWARD, 0, K_VOLUME_UP, 0, K_PLAY_PAUSE, 0, 0, K_ACPI_POWER, K_WWW_BACK, 0, K_WWW_HOME, K_STOP, 0, 0, 0, K_ACPI_SLEEP, K_MY_COMPUTER, 0, 0, 0, 0, 0, 0, 0, K_EMAIL, 0, K_KP_DIVIDE, 0, 0, K_NEXT_TRACK, 0, 0, K_MEDIA_SELECT, 0, 0, 0, 0, 0, 0, 0, 0, 0, K_KP_ENTER, 0, 0, 0, K_ACPI_WAKE, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, K_END, 0, K_CURSOR_LEFT, K_HOME, 0, 0, 0, K_KP_INSERT, K_DELETE, K_CURSOR_DOWN, 0, K_CURSOR_RIGHT, K_CURSOR_UP, 0, 0, 0, 0, K_PAGE_DOWN, 0, 0, K_PAGE_UP, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };


/// Following tables are defining conversion from scancode to ascii characters, returning zero if not a visual character.
/// For each layout, there are 3 tables: [default, shift, altgr]
struct keyboard_layout_impl {
    enum keyboard_layout id;
    /// Default printable characters.
    char printable[K_PRINTABLE_COUNT];
    /// Shift printable characters
    char printable_shift[K_PRINTABLE_COUNT];
    /// Alt graph printable characters
    char printable_alt_graph[K_PRINTABLE_COUNT];
};

/// Definition of all layouts.
static struct keyboard_layout_impl all_layouts[KEYBOARD_LAYOUT_COUNT] = {
    {
        KEYBOARD_LAYOUT_EN,
        { 
            ' ', '`', ',', '.', ';', '/', '\\', '\'', '=', '+', '-', '*', '[', ']', 
            '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
            'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
            '.', '+', '-', '*', '/', '0', 0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9'
        },
        { 
            ' ', '`', ',', '.', ';', '/', '\\', '\'', '=', '+', '-', '*', '[', ']', 
            '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
            'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
            '.', '+', '-', '*', '/', '0', 0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9'
        },
        { 
            ' ', '`', ',', '.', ';', '/', '\\', '\'', '=', '+', '-', '*', '[', ']', 
            '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
            'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
            '.', '+', '-', '*', '/', '0', 0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9'
        },
    },
    {
        KEYBOARD_LAYOUT_FR,
        { 
            ' ', '`', ';', ':', 'm', '!', '*', 'u', '=', '+', ')', '*', '^', '$', 
            'a', '&', 'e', '"', '\'', '(', '-', 'e', '_', 'c',
            'q', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', ',', 'n', 'o', 'p', 'a', 'r', 's', 't', 'u', 'v', 'z', 'x', 'y', 'w',
            '.', '+', '-', '*', '/', 0, 0, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'
        },
        { 
            ' ', '`', '.', '/', 'M', ' ', 'u', '%', '+', '+', ' ', '*', ' ', ' ', 
            '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
            'Q', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', '?', 'N', 'O', 'P', 'A', 'R', 'S', 'T', 'U', 'V', 'Z', 'X', 'Y', 'W',
            '.', '+', '-', '*', '/', 0, 0, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'
        },
        { 
            0, 0, 0, 0, 0, 0, 0, 0, '}', 0, ']', 0, 0, 0, 
            '@', 0, '~', '#', '{', '[', '|', '`', '\\', '^',
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        },
    }
};

static struct keyboard_layout_impl *current_layout;

static bool scan_break = false;
static bool scan_extended = false;

static bool key_shift = false;
static bool key_alt_graph = false;

static keyboard_key_handler_t key_handler = NULL;
static keyboard_char_handler_t char_handler = NULL;


/// Internal keyboard key code handler.
static void keyboard_key_handler(enum keyboard_key key, uint32_t scancode, enum keyboard_action action) {
    
    if (key_handler != NULL)
        key_handler(key, scancode, action, 0);

    if (key == K_LEFT_SHIFT || key == K_RIGHT_SHIFT) {
        key_shift = (action == K_PRESS);
    } else if (key == K_RIGHT_ALT) {
        key_alt_graph = (action == K_PRESS);
    }

    if (action == K_PRESS && key >= K_PRINTABLE_FIRST && key <= K_PRINTABLE_LAST) {
        
        char c;
        if (key_alt_graph) {
            c = current_layout->printable_alt_graph[key - K_PRINTABLE_FIRST];
        } else if (key_shift) {
            c = current_layout->printable_shift[key - K_PRINTABLE_FIRST];
        } else {
            c = current_layout->printable[key - K_PRINTABLE_FIRST];
        }

        if (char_handler != NULL && c != 0)
            char_handler(c);

    }

}


/// Internal PS/2 interrupt handler for scancode.
static void keyboard_handler(uint8_t scancode) {

    if (scancode == 0xF0) {
        scan_break = true;
    } else if (scancode == 0xE0) {
        scan_extended = true;
    } else {

        enum keyboard_key key;
        if (scan_extended) {
            key = extended[scancode];
            scan_extended = false;
        } else {
            key = normal[scancode];
        }

        keyboard_key_handler(key, scancode, scan_break ? K_RELEASE : K_PRESS);
        scan_break = false;

    }

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

    keyboard_set_layout(KEYBOARD_LAYOUT_FR);
    ps2_device_set_handler(PS2_FIRST, keyboard_handler);

    printf("\r[ \aaOK\ar ] Keyboard driver ready       \n");

}

void keyboard_set_layout(enum keyboard_layout layout) {
    current_layout = &all_layouts[layout];
}

void keyboard_set_key_handler(keyboard_key_handler_t handler) {
    assert(handler != NULL);
    key_handler = handler;
}

void keyboard_set_char_handler(keyboard_char_handler_t handler) {
    assert(handler != NULL);
    char_handler = handler;
}
