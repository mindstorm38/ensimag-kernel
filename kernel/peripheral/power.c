#include "power.h"
#include "cpu.h"


void power_off(void) {
    // Specific to QEMU (for now).
    outw(0x2000, 0x604);
}
