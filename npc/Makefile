
DIR = ./obj_dir
EXE = $(BUILD_DIR)/$(TOPNAME)
WAVE = waveform.vcd

SO = -so
NAME = riscv32-npc

TOPNAME = ysyx_22041211_top
NXDC_FILES = constr/ysyx_22041211_top.nxdc
INC_PATH ?= /home/chelsea/ysyx-workbench/npc/include 

VERILATOR = verilator
VERILATOR_CFLAGS += -MMD --cc --trace -Wall --build -j 0  \
				-O3 --x-assign fast --x-initial fast --noassert \
				-CFLAGS -ggdb -LDFLAGS -ggdb -CFLAGS -DVL_DEBUG=1 -CFLAGS -D_GLIBCXX_DEBUG

BUILD_DIR = ./build
OBJ_DIR = $(BUILD_DIR)/obj_dir
BIN = $(BUILD_DIR)/$(TOPNAME)

CXXSRC = $(NPC_HOME)/utils/disasm.cc
# --cxxflags 选项表示获取与 C++ 相关的编译器选项，输出使用LLVM头文件所需要的C++编译选项
# llvm-config 让使用LLVM构建的应用程序变得很容易，他可以打印链接LLVM所需的编译器标志，编译器标志和对象库
CXXFLAGS += $(shell llvm-config --cxxflags -DLLVM_ENABLE_DUMP=ON) -fPIE 
# --libs 打印链接指定LLVM 组件所需的所有库 ，包括任何依赖项。
LIBS += $(shell llvm-config --libs)

all:default

default: $(BIN)


$(shell mkdir -p $(BUILD_DIR))

# constraint file
SRC_AUTO_BIND = $(abspath $(BUILD_DIR)/auto_bind.cpp)
$(SRC_AUTO_BIND): $(NXDC_FILES)
	python3 $(NVBOARD_HOME)/scripts/auto_pin_bind.py $^ $@

# project source
VSRCS = $(shell find $(abspath ./vsrc) -name "*.v")
#HSRCS = $(shell find $(abspath ./csrc) -name "*.h")
CSRCS = $(shell find $(abspath ./csrc) -name "*.c" -or -name "*.cpp" -or -name "*.cc")
# #XXX: Use another .mk to set the source code path
# CSRCS += $(shell find $(abspath ./utils) -name "*.c" -or -name "*.cpp" -or -name "*.cc")
SRCS = $(CXXSRC) $(CSRCS)
# OBJS = $(CXXSRC:%.cc=$(OBJ_DIR)/%.o) $(CSRCS:%.cpp=$(OBJ_DIR)/%.o)

# rules for verilator
INCFLAGS = $(addprefix -I, $(INC_PATH))
CFLAGS += $(INCFLAGS) -DTOP_NAME="\"V$(TOPNAME)\"" 
LDFLAGS += -lSDL2 -lSDL2_image -lreadline -ldl -pie $(LIBS)


override ARGS ?= --log=$(BUILD_DIR)/nemu-log.txt  #--batch


$(BIN):$(SRCS) $(VSRCS) 
	$(info "hhhh1")
	$(warning INC_PATH = $(INC_PATH))
	@rm -rf $(OBJ_DIR)
	$(info "hhhh2")
	$(VERILATOR) $(VERILATOR_CFLAGS) \
		--top-module $(TOPNAME) $^ $(addprefix -CFLAGS , $(CFLAGS)) $(addprefix -LDFLAGS , $(LDFLAGS)) \
		$(addprefix -CFLAGS , $(CXXFLAGS))	\
		--Mdir $(OBJ_DIR) --exe -o $(abspath $(BIN))
	


# $(OBJ_DIR)/%.o: %.cc
# 	@echo + CXX $<
# 	@mkdir -p $(dir $@)
# 	@$(CXX) $(CFLAGS) -c -o $@ $<
# 	$(call call_fixdep, $(@:.o=.d), $@)

# $(OBJ_DIR)/%.o: %.cpp
# 	@echo + CXX $<
# 	@mkdir -p $(dir $@)
# 	@$(CXX) $(CFLAGS) $(LDFLAGS) -c -o $@ $<
# 	$(call call_fixdep, $(@:.o=.d), $@)

# $(BINARY): $(OBJS) 
# 	@echo + LD $@
# 	@$(LD) -o $@ $(OBJS) $(LDFLAGS) $(LIBS)


$(shell mkdir -p $(BUILD_DIR))

run:$(BIN)
	$(info "hhhh")
	$(call git_commit, "sim RTL") #DO NOT REMOVE THIS LINE!!!
	$(EXE) $(IMG) $(ARGS)

.PHONY:wave clean sim default all run 

wave:
	gtkwave $(WAVE)

clean:
	$(RM) -r $(OBJ_DIR) $(WAVE) $(EXE)

include ../Makefile
