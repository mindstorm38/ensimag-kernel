# Tools
CC := gcc
AS := gcc
LD := ld
AR := ar
DEP := gcc
OBJCOPY := objcopy
MKDIR := mkdir
CP := cp

# Flags
DEFS   := -D__KERNEL__
CFLAGS := -m32 \
		  -Wall -Wextra -Werror -std=gnu17\
		  -g \
		  -pipe \
		  -nostdinc \
		  -nostdlib \
		  -fno-stack-protector \
		  -fno-pie -no-pie \
		  -mgeneral-regs-only \
		  -march=pentium \
		  $(DEFS)
ASFLAGS := -m32 -g\
		   -DASSEMBLER \
		   $(DEFS)
ARFLAGS := rcs
LDFLAGS := -g -melf_i386 -no-pie

# Source files
SRCS := $(wildcard *.[cS])
SRCS += $(wildcard boot/*.[cS])
SRCS += $(wildcard shared/*.[cS])
SRCS += $(wildcard peripheral/*.[cS])
SRCS += $(wildcard process/*.[cS])
SRCS += $(wildcard memory/*.[cS])
SRCS += $(wildcard debug/*.[cS])
INCS := include shared

# Compute output
OUTPUT := out
INCSFLAGS := $(addprefix -I, $(INCS))
DEPS := $(addprefix $(OUTPUT)/, $(addsuffix .d, $(SRCS)))
OBJS := $(addprefix $(OUTPUT)/, $(addsuffix .o, $(SRCS))) $(OUTPUT)/task_dump_screen.o

# Dependency rules
$(OUTPUT)/%.c.d: %.c
	mkdir -p $(@D)
	$(DEP) $(INCSFLAGS) -MT $(OUTPUT)/$<.o -MM $< -MF $@

$(OUTPUT)/%.S.d: %.S
	mkdir -p $(@D)
	$(DEP) -MM -MD -o $@ $< 
	$(DEP) $(INCSFLAGS) -MT $(OUTPUT)/$<.o -MM $< -MF $@

# Compile rule
$(OUTPUT)/%.c.o: %.c
	$(CC) $(CFLAGS) $(INCSFLAGS) -c $< -o $@

$(OUTPUT)/%.S.o: %.S
	$(AS) $(ASFLAGS) $(INCSFLAGS) -c $< -o $@

$(OUTPUT)/task_dump_screen.o: $(OUTPUT)/empty.c.o debug/task_dump_screen.txt
	$(OBJCOPY) $< \
		--add-section=.task_dump_screen=$(filter-out $<, $^) \
		--set-section-flags=.task_dump_screen=contents,alloc,load,data $@

$(OUTPUT)/userdata.o: ../user/user.bin
	$(MAKE) -C ../user
	objcopy $(OUTPUT)/empty.c.o --add-section=.user=$^ --set-section-flags=.user=contents,alloc,load,data $@

# Global rule
out/kernel.bin: build/kernel.lds $(OBJS) $(OUTPUT)/userdata.o
	$(LD) $(LDFLAGS) -e entry -T$< $(filter-out %.lds, $^) -o $@

.PHONY: clean
clean:
	rm -rf $(OUTPUT)

# Include automatic dependency files
include $(DEPS)
