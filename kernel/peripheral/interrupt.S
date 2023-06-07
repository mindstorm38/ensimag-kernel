#define SERV_IRQ_TASK(id) \
	.text; \
irq_handler_##id: \
	pushl $(id); \
	jmp irq_generic_handler_entry; \
	.data; \
	.long irq_handler_##id

	.text
irq_generic_handler_entry:
	pushl %eax
	pushl %ebx
	pushl %ecx
	pushl %edx
	pushl %esi
	pushl %edi
	pushl 24(%esp)  // Repush the IRQ number as parameter 1.
	cld
	call irq_generic_handler
	addl $4, %esp    // Remove parameter.
	popl %edi
	popl %esi
	popl %edx
	popl %ecx
	popl %ebx
	popl %eax
	addl $4, %esp    // Remove original IRQ number.
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
