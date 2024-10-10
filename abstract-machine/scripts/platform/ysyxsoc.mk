# include $(AM_HOME)/scripts/isa/riscv.mk
AM_SRCS := riscv/ysyxsoc/start.S \
           riscv/ysyxsoc/trm.c \
           riscv/ysyxsoc/ioe.c \
           riscv/ysyxsoc/timer.c \
		   riscv/ysyxsoc/gpu.c \
           riscv/ysyxsoc/input.c \
           riscv/ysyxsoc/cte.c \
           riscv/ysyxsoc/trap.S \
           platform/dummy/vme.c \
           platform/dummy/mpe.c
ALL ?= $(NAME)
BUILD_DIR = $(shell dirname $(IMAGE).elf)
CFLAGS    += -fdata-sections -ffunction-sections
LDFLAGS   += -T $(AM_HOME)/scripts/linker.ld \
						 --defsym=_pmem_start=0x20000000 --defsym=_entry_offset=0x0
LDFLAGS   += --gc-sections -e _start
CFLAGS += -DMAINARGS=\"$(mainargs)\"

NPCFLAGS += --log=$(shell dirname $(IMAGE).elf)/ysyxsoc-log.txt #--batch
NPCFLAGS += $(ARGS_DIFF)
NPCFLAGS += --ftrace=$(shell dirname $(IMAGE).elf)/$(ALL)-$(ARCH).elf --diff=$(NPC_HOME)/diff-ref/riscv32-nemu-interpreter-so

CFLAGS += -I$(AM_HOME)/am/include -I$(AM_HOME)/am/src/riscv/ysyxsoc/libgcc -I$(AM_HOME)/am/src/riscv/ysyxsoc/include

.PHONY: $(AM_HOME)/am/src/riscv/ysyxsoc/trm.c $(DIFF_REF_SO)

image: $(IMAGE).elf
	@$(OBJDUMP) -d $(IMAGE).elf > $(IMAGE).txt
	@echo + OBJCOPY "->" $(IMAGE_REL).bin
	@$(OBJCOPY) -S --set-section-flags .bss=alloc,contents -O binary $(IMAGE).elf $(IMAGE).bin

gdb: image
	$(MAKE) -C $(NPC_HOME) ISA=$(ISA) gdb ARGS="$(NPCFLAGS)" IMG=$(IMAGE).bin  BUILD_DIR="$(BUILD_DIR)"

run: image
	$(info DIFF_REF_SO:$(DIFF_REF_SO))
	$(info ARGS_DIFF:$(ARGS_DIFF)) 
# $(MAKE) -C $(NPC_HOME) ISA=$(ISA) run ARGS="$(NPCFLAGS)" IMG=$(IMAGE).bin
	bear --output ~/ysyx-workbench/.vscode/compile_commands.json -- $(MAKE) -C $(NPC_HOME) ISA=$(ISA) run ARGS="$(NPCFLAGS)" IMG=$(IMAGE).bin
# bear --output ~/ysyx-workbench/npc/build/compile_commands.json -- $(MAKE) -C $(NPC_HOME) ISA=$(ISA) run ARGS="$(NPCFLAGS)" IMG=$(IMAGE).bin

