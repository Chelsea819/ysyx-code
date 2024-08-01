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

# ifdef CONFIG_DIFFTEST
GUEST_ISA = riscv32
CONFIG_DIFFTEST_REF_NAME = nemu-interpreter
DIFF_REF_PATH = /home/chelsea/ysyx-workbench/nemu
DIFF_REF_SO = $(DIFF_REF_PATH)/build/$(GUEST_ISA)-$(CONFIG_DIFFTEST_REF_NAME)-so
MKFLAGS = GUEST_ISA=$(GUEST_ISA) SHARE=1 ENGINE=interpreter
ARGS_DIFF = --diff=$(DIFF_REF_SO)

# ifndef CONFIG_DIFFTEST_REF_NEMU
# $(DIFF_REF_SO):
# 	$(MAKE) -s -C $(DIFF_REF_PATH) $(MKFLAGS)
# endif
# endif

NPCFLAGS += --log=$(shell dirname $(IMAGE).elf)/npc-log.txt
NPCFLAGS += $(ARGS_DIFF)
NPCFLAGS += --ftrace=$(shell dirname $(IMAGE).elf)/$(ALL)-$(ARCH).elf

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
# -$(MAKE) -C $(NEMU_HOME) ISA=riscv32 run ARGS="$(NEMUFLAGS)"  IMG=$(IMAGE).bin
# $(MAKE) -C $(NPC_HOME) ISA=$(ISA) run ARGS="$(NPCFLAGS)" IMG=$(IMAGE).bin  BUILD_DIR="$(BUILD_DIR)"
	$(MAKE) -C $(NPC_HOME) ISA=$(ISA) run ARGS="$(NPCFLAGS)" IMG=$(IMAGE).bin

