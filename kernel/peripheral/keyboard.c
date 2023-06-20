#include "stdbool.h"
#include "stddef.h"
#include "stdio.h"

#include "interrupt.h"
#include "keyboard.h"
#include "cpu.h"
#include "ps2.h"
#include <stdint.h>


/* On utilise, http://www.vetra.com/scancodes.html

function generateTable(table, setCell) {
  const ret = [];
  for (let i = 0; i < 8; ++i) {
      ret.push(new Array(256));
  }
  const rows = table.querySelectorAll("tr");
  
    for (let i = 1; i < rows.length; ++i) {

        const cells = rows[i].querySelectorAll("td");

        const baseCase = cells[4].textContent.trim();

        const cellCodes = cells[setCell].textContent.trim().split("/");
        const makeCodes = cellCodes[0].split(" ");
        for (let j = 0; j < makeCodes.length; ++j) {
            const makeCode = parseInt(makeCodes[j], 16);
            ret[j][makeCode] = baseCase;
        }
        
    }

    return ret;

}

function formatTable(tab) {
    let nullCount = 0;
    let buf = "{";
    for (let i = 0; i < tab.length; ++i) {
        if (i > 0) {
            buf += ", ";
        }
        if (tab[i] == null) {
            buf += "0";
            nullCount++;
        } else {
            buf += "'" + tab[i] + "'";
        }
    }
    return (nullCount == tab.length) ? null : (buf + "};");
}

function generateTables() {
    const table = generateTable(document.querySelector("body > h3:nth-child(5) > table:nth-child(1)"), 1);
    for (let i = 0; i < table.length; ++i) {
        const formattedTable = formatTable(table[i]);
        if (formattedTable != null) {
            console.log("#" + i + ": " + formattedTable);
        }
    }
}

generateTables();

*/


// Following tables are defining conversion from scancode to ascii characters, returning zero if not a visual character.
// For each layout, there are 3 tables: [default, shift, altgr]

struct keyboard_layout {
    uint16_t id;
    uint8_t initial[256];
    uint8_t shift[256];
    uint8_t alt_graph[256];
};


static struct keyboard_layout KEYBOARD_LAYOUT_EN_ = {
    KEYBOARD_LAYOUT_EN,
    {0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0, 0, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 0, 0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0, 0, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 0, 0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0, 0, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 0, 0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

static struct keyboard_layout KEYBOARD_LAYOUT_FR_ = {
    KEYBOARD_LAYOUT_FR,
    {0, 0, '&', 'e', '"', '\'', '(', '-', 'e', '_', 'c', 'a', ')', '=', 0, 0, 'a', 'z', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '^', '$', 0, 0, 'q', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 'm', 'u', '*', 0, '\\', 'w', 'x', 'c', 'v', 'b', 'n', ',', ';', ':', '!', 0, 0, 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 0, '+', 0, 0, 'A', 'Z', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', 0, 0, 0, 0, 'Q', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', 'M', '%', 'u', 0, '\\', 'W', 'X', 'C', 'V', 'B', 'N', '?', '.', '/', 0, 0, 0, 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, '~', '#', '{', '[', '|', '`', '\\', '^', '@', ']', '}', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '~', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '`', 0, '\\', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

static struct keyboard_layout *current_layout;

// static uint8_t last_scancode = 0;
// static bool shift = false;
// static bool ctrl = false;
// static bool alt = false;
// static bool altgr = false;


static void keyboard_handler(uint8_t data) {
    printf("keyboard: %d\n", data);
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
    if (ps2_device_command(PS2_FIRST, 0xF0) != PS2_DEV_RES_ACK) {
        printf("\r[\acFAIL\ar] Keyboard driver scancode set 2 failed\n");
        return;
    }

    keyboard_select_layout(KEYBOARD_LAYOUT_FR);
    ps2_device_set_handler(PS2_FIRST, keyboard_handler);

    printf("\r[ \aaOK\ar ] Keyboard driver ready       \n");

}

void keyboard_select_layout(uint16_t layout) {
    current_layout = NULL;
    switch (layout) {
    case KEYBOARD_LAYOUT_EN:
        current_layout = &KEYBOARD_LAYOUT_EN_;
        break;
    case KEYBOARD_LAYOUT_FR:
        current_layout = &KEYBOARD_LAYOUT_FR_;
        break;
    default:
        return;
    }
}

// void keyboard_interrupt_handler(void) {

//     interrupt_eoi(1);

//     uint8_t scancode_raw = keyboard_read_data();
//     uint8_t scancode = scancode_raw & 0x7F;
//     bool make = (scancode_raw & 0x80) == 0;

//     if (last_scancode != 0) {

//         switch (scancode_raw) {
//             case 0x1D:
//                 ctrl = true;
//                 break;
//             case 0x9D:
//                 ctrl = false;
//                 break;
//             case 0x38:
//                 altgr = true;
//                 break;
//             case 0xB8:
//                 altgr = false;
//                 break;
//             default:
//                 printf("^^%02X", scancode_raw);
//         }

//         last_scancode = 0;
//         return;

//     }

//     switch (scancode_raw) {
//     case 0x0F:
//         printf("\t"); 
//         break;
//     case 0x1C:
//         printf("\n"); 
//         break;
//     case 0x0E:
//         printf("\b \b");
//         break;
//     case 0x2A:
//     case 0x36:
//         shift = true;
//         break;
//     case 0xAA:
//     case 0xB6:
//         shift = false;
//         break;
//     case 0x1D:
//         ctrl = true;
//         break;
//     case 0x9D:
//         ctrl = false;
//         break;
//     case 0x38:
//         alt = true;
//         break;
//     case 0xB8:
//         alt = false;
//         break;
//     case 0xE0:
//         last_scancode = scancode;
//         break;
//     case 0x8F: // break tab
//     case 0x8E: // break backspace
//     case 0x9C: // break enter
//         break;
//     default: ;

//         uint8_t chr;
//         if (altgr) {
//             chr = current_layout->alt_graph[scancode];
//         } else if (shift) {
//             chr = current_layout->shift[scancode];
//         } else {
//             chr = current_layout->initial[scancode];
//         }

//         if (chr != 0) {
//             if (make) printf("%c", chr);
//         } else {
//             printf("^%02X", scancode_raw);
//         }

//     }

// }
