#define SERV_IRQ_TASK(id) \
.text; \
irq_handler_##id: \
	pushl $(id); \
	jmp irq_generic_handler_entry; \
.data; \
	.long irq_handler_##id

.text
irq_generic_handler_entry:
	# This pushes the only 3 registers that are caller-saved in CDECL.
	push %eax
	push %ecx
	push %edx
	# Get the IRQ id stored in parameter 1.
	mov 12(%esp), %eax
	# Call the handler at the offset in the table.
	lea irq_handlers, %ecx
	mov (%ecx,%eax,4), %ecx
	call %ecx
	# Restore context and pop the IRQ number.
	pop %edx
	pop %ecx
	pop %eax
	add $4, %esp
	iret

.data
.globl irq_handlers_entry
irq_handlers_entry:
	SERV_IRQ_TASK(0)
	SERV_IRQ_TASK(1)
	SERV_IRQ_TASK(2)
	SERV_IRQ_TASK(3)
	SERV_IRQ_TASK(4)
	SERV_IRQ_TASK(5)
	SERV_IRQ_TASK(6)
	SERV_IRQ_TASK(7)
	SERV_IRQ_TASK(8)
	SERV_IRQ_TASK(9)
	SERV_IRQ_TASK(10)
	SERV_IRQ_TASK(11)
	SERV_IRQ_TASK(12)
	SERV_IRQ_TASK(13)
	SERV_IRQ_TASK(14)
	SERV_IRQ_TASK(15)
