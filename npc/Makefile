
DIR = ./obj_dir
EXE = $(BUILD_DIR)/$(TOPNAME)
WAVE = waveform.vcd

TOPNAME = ysyx_22041211_top
NXDC_FILES = constr/ysyx_22041211_top.nxdc
INC_PATH ?= /home/chelsea/ysyx-workbench/npc/include 

VERILATOR = verilator
VERILATOR_CFLAGS += -MMD --build --cc --trace -Wall\
				-O3 --x-assign fast --x-initial fast --noassert --threads 2

BUILD_DIR = ./build
OBJ_DIR = $(BUILD_DIR)/obj_dir
BIN = $(BUILD_DIR)/$(TOPNAME)

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
CSRCS = $(shell find $(abspath ./csrc) -name "*.c" -or -name "*.cc" -or -name "*.cpp")
#CSRCS += $(SRC_AUTO_BIND)

# rules for NVBoard
#include $(NVBOARD_HOME)/scripts/nvboard.mk

# rules for verilator
INCFLAGS = $(addprefix -I, $(INC_PATH))
CFLAGS += $(INCFLAGS) -DTOP_NAME="\"V$(TOPNAME)\"" 
LDFLAGS += -lSDL2 -lSDL2_image -lreadline -ldl -pie


$(BIN):$(VSRCS) $(CSRCS) #$(NVBOARD_ARCHIVE)
	$(warning INC_PATH = $(INC_PATH))
#@echo +++++$(INC_PATH)
	@rm -rf $(OBJ_DIR)
	$(VERILATOR) $(VERILATOR_CFLAGS) \
		--top-module $(TOPNAME) $^\
		$(addprefix -CFLAGS , $(CFLAGS)) $(addprefix -LDFLAGS , $(LDFLAGS)) \
		--Mdir $(OBJ_DIR) --exe -o $(abspath $(BIN))
	

$(shell mkdir -p $(BUILD_DIR))

run:  $(BIN)
	$(call git_commit, "sim RTL") #DO NOT REMOVE THIS LINE!!!
	$(EXE) $(IMG)

.PHONY:wave clean sim default all run

wave:
	gtkwave $(WAVE)

clean:
	$(RM) -r $(OBJ_DIR) $(WAVE) $(EXE)

include ../Makefile
