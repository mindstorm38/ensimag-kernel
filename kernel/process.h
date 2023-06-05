#include "stdint.h"
#include "stddef.h"


/// States that a process can take, used for scheduling.
enum process_state {
    PROCESS_ELECTED,
    PROCESS_AVAILABLE,
};

/// The process structure.
struct process {
    size_t pid;
    const char name[128];
    enum process_state state;
    size_t registers[5];
    char stack[512];
};
