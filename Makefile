.PHONY: clean all

all:
	$(MAKE) -C user/ all VERBOSE=$(VERBOSE)
	$(MAKE) -C kernel/ out/kernel.bin VERBOSE=$(VERBOSE)

clean:
	$(MAKE) clean -C kernel/
	$(MAKE) clean -C user/

qemu: all
	qemu-system-i386 -machine q35 -m 256 -kernel kernel/out/kernel.bin

qemu-term: all
	qemu-system-i386 -machine q35 -m 256 -kernel kernel/out/kernel.bin -display curses
