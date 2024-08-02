DIR = ./obj_dir
EXE = $(BUILD_DIR)/$(TOPNAME)
WAVE = waveform.vcd

# SO = -so
# NAME = riscv32-npc

GUEST_ISA ?= riscv32e
TOPNAME = ysyx_22041211_top


# Include all filelist.mk to merge file lists
FILELIST_MK = $(shell find -L ./csrc -name "filelist.mk")
include $(FILELIST_MK)

# Filter out directories and files in blacklist to obtain the final set of source files
DIRS-BLACKLIST-y += $(DIRS-BLACKLIST)
CSRCS-BLACKLIST-y += $(CSRCS-BLACKLIST) $(shell find -L $(DIRS-BLACKLIST-y) -name "*.c")
CSRCS-y += $(shell find -L $(DIRS-y) -name "*.c")
CSRCS = $(filter-out $(CSRCS-BLACKLIST-y),$(CSRCS-y))

# project source
VSRCS = $(shell find $(abspath ./vsrc) -name "*.v")
# CSRCS = $(shell find $(abspath ./csrc) -name "*.c" -or -name "*.cpp" -or -name "*.cc")
# CSRCS += $(shell find $(abspath ./csrc/device) -name "*.c" -or -name "*.cpp" -or -name "*.cc")
SRCS = $(CXXSRC) $(CSRCS)


NXDC_FILES = constr/ysyx_22041211_top.nxdc
INC_PATH += $(NPC_HOME)/include 

VERILATOR = verilator
VERILATOR_CFLAGS += -MMD --cc  -Wall --build -j 0  \
				-O3 --x-assign fast --x-initial fast --noassert #--trace

#-ggdb -LDFLAGS -ggdb -CFLAGS -DVL_DEBUG=1 
# -CFLAGS -D_GLIBCXX_DEBUG

WORK_DIR  = $(shell pwd)
override BUILD_DIR ?= $(WORK_DIR)/build
OBJ_DIR = $(BUILD_DIR)/obj_dir
BIN = $(BUILD_DIR)/$(TOPNAME)

CXXSRC = $(NPC_HOME)/csrc/utils/disasm.cc
# --cxxflags 选项表示获取与 C++ 相关的编译器选项，输出使用LLVM头文件所需要的C++编译选项
# llvm-config 让使用LLVM构建的应用程序变得很容易，他可以打印链接LLVM所需的编译器标志，编译器标志和对象库
CXXFLAGS += $(shell llvm-config --cxxflags) -fPIE 

# --libs 打印链接指定LLVM 组件所需的所有库 ，包括任何依赖项。

LIBS += $(shell llvm-config --libs)

all:default

default: $(BIN)


$(shell mkdir -p $(BUILD_DIR))

# constraint file
SRC_AUTO_BIND = $(abspath $(BUILD_DIR)/auto_bind.cpp)
$(SRC_AUTO_BIND): $(NXDC_FILES)
	python3 $(NVBOARD_HOME)/scripts/auto_pin_bind.py $^ $@

# rules for verilator
INCLUDES += $(addprefix -I, $(INC_PATH))
CFLAGS +=  $(INCLUDES) -Wall -DTOP_NAME="\"V$(TOPNAME)\"" #-D__GUEST_ISA__=$(GUEST_ISA)
CXXFLAGS := $(filter-out -D__STDC_FORMAT_MACROS, $(CXXFLAGS))
CFLAGS := $(filter-out -D__STDC_FORMAT_MACROS, $(CFLAGS))
LDFLAGS += -lreadline -ldl -lSDL2  -pie $(LIBS) #-fsanitize=address


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

$(BIN):$(SRCS) $(VSRCS) 
	$(warning INC_PATH = $(INC_PATH))
	$(VERILATOR) $(VERILATOR_CFLAGS) \
		--top-module $(TOPNAME) $^ $(addprefix -CFLAGS , $(CFLAGS)) \
		$(addprefix -LDFLAGS , $(LDFLAGS)) \
		$(addprefix -CFLAGS , $(CXXFLAGS))	\
		--Mdir $(OBJ_DIR) --exe -o $(abspath $(BIN))
	


$(shell mkdir -p $(BUILD_DIR))

run:$(BIN)
	$(call git_commit, "sim RTL") #DO NOT REMOVE THIS LINE!!!
	$(info $(ARGS))
	$(info ARCH_H $(ARCH_H))
	$(EXE)  $(ARGS) $(IMG)

gdb:$(BIN)
	$(call git_commit, "sim RTL") #DO NOT REMOVE THIS LINE!!!
	gdb -s $(EXE) --args $(EXE) $(ARGS) $(IMG)

.PHONY:wave clean sim default all run $(DIFF_REF_SO)

wave:
	gtkwave $(WAVE)

clean:
	$(RM) -r $(BUILD_DIR) $(WAVE) $(EXE)

include ../Makefile
