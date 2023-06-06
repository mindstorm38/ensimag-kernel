#include "process.h"
#include "stddef.h"
#include "stdint.h"
#include "stdio.h"
#include "string.h"
#include "cpu.h"


static struct process processes[PROCESS_MAX_COUNT];
static size_t processes_count = 0;
static struct process *running_process;

/// Function defined in assembly (process_context.S).
void process_context_switch(struct process_context *prev_ctx, struct process_context *next_ctx);

/// Internal function to initialize ESP register of a process that i
/// is not yet scheduled. It also push the address of the entry point
/// on top of the stack.
static void process_init_esp(struct process *proc, void *ret_addr) {
    void **ret_ptr = (void **) &proc->stack.data[PROCESS_STACK_SIZE - sizeof(void *)];
    *ret_ptr = ret_addr;
    proc->context.esp = (uint32_t) ret_ptr;
}

int32_t get_pid() {
    return running_process->pid;
}

char *get_name() {
    return running_process->name;
}

void schedule() {

    int32_t pid = running_process->pid;
    int32_t next_pid = (pid + 1) % PROCESS_MAX_COUNT;

    struct process *prev_process = running_process;
    struct process *next_process = &processes[next_pid];

    running_process = next_process;
    process_context_switch(&prev_process->context, &next_process->context);

}


void _idle(void);
void _proc1(void);


void process_init() {

    struct process *idle = &processes[0];
    idle->pid = 0;
    strcpy(idle->name, "idle");
    idle->state = PROCESS_AVAILABLE;

    struct process *proc1 = &processes[1];
    proc1->pid = 1;
    strcpy(proc1->name, "proc1");
    proc1->state = PROCESS_AVAILABLE;
    process_init_esp(proc1, (void *) _proc1);

    running_process = idle;
    _idle();

    processes_count = 2;

}


void _idle(void) {
  for (;;) {
    printf("[%s] pid = %i\n", get_name(), get_pid());
    for (int32_t i = 0; i < 100000000; i++)
      ;
    schedule();
  }
}

void _proc1(void) {
  for (;;) {
    printf("[%s] pid = %i\n", get_name(), get_pid());
    for (int32_t i = 0; i < 100000000; i++)
      ;
    schedule();
  }
}
