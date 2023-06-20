#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#include "stdint.h"


/// Layout to be used for converting keyboard scan codes into 
/// compatible key codes.
enum keyboard_layout {
    KEYBOARD_LAYOUT_FR,
    KEYBOARD_LAYOUT_COUNT,
};

/// Definition of key codes, converted from scan codes.
enum keyboard_key {
    // Unknown key translation.
    K_UNKOWN,
    // Control characters...
    K_ESCAPE,
    K_ENTER,
    K_BACKSPACE,
    K_DELETE,
    K_INSERT,
    K_END,
    K_HOME,
    K_CURSOR_UP,
    K_CURSOR_DOWN,
    K_CURSOR_RIGHT,
    K_CURSOR_LEFT,
    K_PAGE_DOWN,
    K_PAGE_UP,
    K_LEFT_ALT,
    K_RIGHT_ALT,
    K_LEFT_SHIFT,
    K_RIGHT_SHIFT,
    K_LEFT_CTRL,
    K_RIGHT_CTRL,
    K_LEFT_GUI,
    K_RIGHT_GUI,
    // Locks...
    K_CAPS_LOCK,
    K_NUM_LOCK,
    K_SCROLL_LOCK,
    // Multimedia...
    K_PRINT_SCREEN,
    K_CALCULATOR,
    K_APPS,
    K_MEDIA_SELECT,
    K_PREVIOUS_TRACK,
    K_NEXT_TRACK,
    K_PLAY_PAUSE,
    K_STOP,
    K_VOLUME_MUTE,
    K_VOLUME_DOWN,
    K_VOLUME_UP,
    K_WWW_SEARCH,
    K_WWW_HOME,
    K_WWW_FAVOURITES,
    K_WWW_REFRESH,
    K_WWW_STOP,
    K_WWW_FORWARD,
    K_WWW_BACK,
    K_MY_COMPUTER,
    K_EMAIL,
    // ACPI
    K_ACPI_POWER,
    K_ACPI_SLEEP,
    K_ACPI_WAKE,
    // Function characters...
    K_F1,
    K_F2,
    K_F3,
    K_F4,
    K_F5,
    K_F6,
    K_F7,
    K_F8,
    K_F9,
    K_F10,
    K_F11,
    K_F12,
    // Accents and special characters...
    K_SPACE,
    K_TAB,
    K_BACK_TICK,
    K_COMMA,
    K_PERIOD,
    K_SEMICOLON,
    K_SLASH,
    K_BACKSLASH,
    K_DASH,
    K_APOSTROPHE,
    K_EQUAL,
    K_PLUS,
    K_MINUS,
    K_MUL,
    K_OPEN_BRACKET,
    K_CLOSE_BRACKET,
    // Numbers...
    K_0,
    K_1,
    K_2,
    K_3,
    K_4,
    K_5,
    K_6,
    K_7,
    K_8,
    K_9,
    // Letters...
    K_A,
    K_B,
    K_C,
    K_D,
    K_E,
    K_F,
    K_G,
    K_H,
    K_I,
    K_J,
    K_K,
    K_L,
    K_M,
    K_N,
    K_O,
    K_P,
    K_Q,
    K_R,
    K_S,
    K_T,
    K_U,
    K_V,
    K_W,
    K_X,
    K_Y,
    K_Z,
} __attribute__((__packed__));

/// Define the action of a keyboard event.
enum keyboard_action {
    K_PRESS,
    K_RELEASE
};

#define K_MOD_CTRL      0x01
#define K_MOD_SHIFT     0x02
#define K_MOD_ALT       0x04
#define K_MOD_ALT_GRAPH 0x08

typedef void (*keyboard_key_handler_t)(enum keyboard_key key, uint32_t scancode, enum keyboard_action action, uint8_t mods);
typedef void (*keyboard_char_handler_t)(char ch);


/// Initialize keyboard interrupts.
void keyboard_init(void);
/// Select the layout of the keyboard, which define how raw scancodes
/// are converted to key codes (enum keyboard_key).
void keyboard_set_layout(enum keyboard_layout layout);
/// Set the key handler, called when a key event happens.
void keyboard_set_key_handler(keyboard_key_handler_t handler);
/// Set the character handler, called when a printable character is
/// pressed. This handler handles control, shift and other modifiers
/// for you.
void keyboard_set_char_handler(keyboard_char_handler_t handler);

#endif
