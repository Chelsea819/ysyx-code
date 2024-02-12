
DIR = ./obj_dir
EXE = $(BUILD_DIR)/$(TOPNAME)
WAVE = waveform.vcd
ENGINE ?= interpreter
NAME    = $(GUEST_ISA)-npc-$(ENGINE)

# SO = -so
# NAME = riscv32-npc

TOPNAME = ysyx_22041211_top
NXDC_FILES = constr/ysyx_22041211_top.nxdc
INC_PATHNPC := /home/chelsea/ysyx-workbench/npc/include /home/chelsea/ysyx-workbench/npc/include/device /home/chelsea/ysyx-workbench/abstract-machine/am/include

# project source
VSRCS = $(shell find $(abspath ./vsrc) -name "*.v")
#HSRCS = $(shell find $(abspath ./csrc) -name "*.h")
CSRCS_NPC := $(shell find $(abspath ./csrc) -name "*.c" -or -name "*.cpp" -or -name "*.cc")
SRCS := $(CXXSRC_NPC) $(CSRCS_NPC)

# rules for verilator
INCFLAGS_NPC := $(addprefix -I, $(INC_PATHNPC))
CFLAGS_NPC := $(INCFLAGS_NPC) -DTOP_NAME="\"V$(TOPNAME)\"" #$(CFLAGS)
# LDFLAGS := -shared -fPIC -lSDL2 -lSDL2_image -lreadline -ldl -fsanitize=address -fno-omit-frame-pointer -pie $(LDFLAGS)
LDFLAGS := -shared -fPIC -lSDL2 -lSDL2_image -lreadline -ldl -fsanitize=address -fno-omit-frame-pointer $(LDFLAGS)


VERILATOR = verilator
VERILATOR_CFLAGS += -MMD --cc --trace -Wall --build -j 0  \
				-O3 --x-assign fast --x-initial fast --noassert \
				-CFLAGS -ggdb -LDFLAGS -ggdb -CFLAGS -DVL_DEBUG=1 
# -CFLAGS -D_GLIBCXX_DEBUG

WORK_DIR  = $(shell pwd)
override BUILD_DIR ?= $(WORK_DIR)/build
OBJ_DIR = $(BUILD_DIR)/obj_dir
BINARY_NPC = $(BUILD_DIR)/$(TOPNAME)

CXXSRC_NPC = $(NPC_HOME)/utils/disasm.cc
# --cxxflags 选项表示获取与 C++ 相关的编译器选项，输出使用LLVM头文件所需要的C++编译选项
# llvm-config 让使用LLVM构建的应用程序变得很容易，他可以打印链接LLVM所需的编译器标志，编译器标志和对象库
CXXFLAGS_NPC += $(shell llvm-config --cxxflags) -fPIE 
# --libs 打印链接指定LLVM 组件所需的所有库 ，包括任何依赖项。

LIBS += $(shell llvm-config --libs)

all:default

default: $(BINARY_NPC)


$(shell mkdir -p $(BUILD_DIR))

# constraint file
SRC_AUTO_BIND = $(abspath $(BUILD_DIR)/auto_bind.cpp)
$(SRC_AUTO_BIND): $(NXDC_FILES)
	python3 $(NVBOARD_HOME)/scripts/auto_pin_bind.py $^ $@


ifdef CONFIG_DIFFTEST
GUEST_ISA = riscv32
CONFIG_DIFFTEST_REF_NAME = nemu-interpreter
DIFF_REF_PATH = $(NEMU_HOME)
DIFF_REF_SO = $(DIFF_REF_PATH)/build/$(GUEST_ISA)-$(CONFIG_DIFFTEST_REF_NAME)-so
# MKFLAGS = GUEST_ISA=$(GUEST_ISA) SHARE=1 ENGINE=interpreter
ARGS_DIFF = --diff=$(DIFF_REF_SO)

# ifndef CONFIG_DIFFTEST_REF_NEMU
# $(DIFF_REF_SO):
# 	$(MAKE) -s -C $(DIFF_REF_PATH) $(MKFLAGS)
# endif

endif

override ARGS ?= --log=$(BUILD_DIR)/npc-log.txt  #--ftrace=$(BUILD_DIR)/$(ALL)-$(ARCH).elf
override ARGS += $(ARGS_DIFF) --batch


$(BINARY_NPC):$(SRCS) $(VSRCS) 
	$(warning CFLAGS = $(CFLAGS_NPC))
	$(warning INC_PATH = $(INC_PATHNPC))
# @rm -rf $(OBJ_DIR)
	$(VERILATOR) $(VERILATOR_CFLAGS) \
		--top-module $(TOPNAME) $^ $(addprefix -CFLAGS , $(CFLAGS_NPC)) $(addprefix -CFLAGS , $(CFLAGS_NPC)) \
		$(addprefix -LDFLAGS , $(LDFLAGS_NPC)) \
		$(addprefix -CFLAGS , $(CXXFLAGS_NPC))	\
		--Mdir $(OBJ_DIR) --exe -o $(abspath $(BINARY_NPC))
	


$(shell mkdir -p $(BUILD_DIR))

sim:$(BINARY_NPC)
	$(call git_commit, "sim RTL") #DO NOT REMOVE THIS LINE!!!
	$(info $(ARGS))
	$(info $(BUILD_DIR))
	$(EXE)  $(ARGS) $(IMG)

gdb-sim:$(BINARY_NPC)
	$(call git_commit, "sim RTL") #DO NOT REMOVE THIS LINE!!!
	gdb -s $(EXE) --args $(EXE) $(ARGS) $(IMG)

.PHONY:wave clean sim default all run $(DIFF_REF_SO)

# ifdef CONFIG_TARGET_AM
include $(AM_HOME)/Makefile
# LINKAGE += $(ARCHIVES)
# endif

wave:
	gtkwave $(WAVE)

clean:
	$(RM) -r $(BUILD_DIR) $(WAVE) $(EXE)

include ../Makefile
