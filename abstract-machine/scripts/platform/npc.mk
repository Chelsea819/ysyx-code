# include $(AM_HOME)/scripts/isa/riscv.mk
AM_SRCS := riscv/npc/start.S \
           riscv/npc/trm.c \
           riscv/npc/ioe.c \
           riscv/npc/timer.c \
		   riscv/npc/gpu.c \
           riscv/npc/input.c \
           riscv/npc/cte.c \
           riscv/npc/trap.S \
           platform/dummy/vme.c \
           platform/dummy/mpe.c
ALL ?= $(NAME)
BUILD_DIR = $(shell dirname $(IMAGE).elf)
CFLAGS    += -fdata-sections -ffunction-sections
LDFLAGS   += -T $(AM_HOME)/scripts/linker.ld \
						 --defsym=_pmem_start=0x80000000 --defsym=_entry_offset=0x0
LDFLAGS   += --gc-sections -e _start
CFLAGS += -DMAINARGS=\"$(mainargs)\"

NPCFLAGS += --log=$(shell dirname $(IMAGE).elf)/npc-log.txt
NPCFLAGS += $(ARGS_DIFF)
NPCFLAGS += --ftrace=$(shell dirname $(IMAGE).elf)/$(ALL)-$(ARCH).elf --diff=$(NEMU_HOME)/build/riscv32-nemu-interpreter-so

CFLAGS += -I$(AM_HOME)/am/include -I$(AM_HOME)/am/src/riscv/npc/libgcc -I$(AM_HOME)/am/src/riscv/npc/include

.PHONY: $(AM_HOME)/am/src/riscv/npc/trm.c $(DIFF_REF_SO)

image: $(IMAGE).elf
	@$(OBJDUMP) -d $(IMAGE).elf > $(IMAGE).txt
	@echo + OBJCOPY "->" $(IMAGE_REL).bin
	@$(OBJCOPY) -S --set-section-flags .bss=alloc,contents -O binary $(IMAGE).elf $(IMAGE).bin

gdb: image
	$(MAKE) -C $(NPC_HOME) ISA=$(ISA) gdb ARGS="$(NPCFLAGS)" IMG=$(IMAGE).bin  BUILD_DIR="$(BUILD_DIR)"

run: image
	$(info DIFF_REF_SO:$(DIFF_REF_SO))
	$(info ARGS_DIFF:$(ARGS_DIFF)) 
	bear --output ~/ysyx-workbench/npc/compile_commands.json -- $(MAKE) -C $(NPC_HOME) ISA=$(ISA) run ARGS="$(NPCFLAGS)" IMG=$(IMAGE).bin

