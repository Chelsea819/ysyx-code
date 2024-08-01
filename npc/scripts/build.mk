.DEFAULT_GOAL = app

# Add necessary options if the target is a shared library
ifeq ($(SHARE),1)
CFLAGS  += -fPIC -fvisibility=hidden
LDFLAGS += -shared -fPIC
endif

WORK_DIR  = $(shell pwd)
override BUILD_DIR ?= $(WORK_DIR)/build
OBJ_DIR = $(BUILD_DIR)/obj_dir
BIN = $(BUILD_DIR)/$(TOPNAME)
WAVE = waveform.vcd

INC_PATH := $(WORK_DIR)/include $(INC_PATH)

# Compilation flags
ifeq ($(CC),clang)
CXX := clang++
else
CXX := g++
endif
LD := $(CXX)
INCLUDES = $(addprefix -I, $(INC_PATH))
CFLAGS  := -O2 -MMD -Wall -Werror $(INCLUDES) $(CFLAGS) -Wno-div-by-zero
LDFLAGS := -O2 $(LDFLAGS)

# fixdep:一个聪明的小工具
## 通过分析文件中的CONFIG_XX宏将对autoconf.h的以来分成若干空文件的依赖
## Kconfig更新配置选项时，更新相应空文件的时间戳

$(BIN):$(SRCS) $(VSRCS) 
	$(warning INC_PATH = $(INC_PATH))
	$(VERILATOR) $(VERILATOR_CFLAGS) \
		--top-module $(TOPNAME) $^ $(addprefix -CFLAGS , $(CFLAGS)) \
		$(addprefix -LDFLAGS , $(LDFLAGS)) \
		$(addprefix -CFLAGS , $(CXXFLAGS))	\
		--Mdir $(OBJ_DIR) --exe -o $(abspath $(BIN))

.PHONY: app clean

app: $(BIN)

clean:
	-rm -rf $(BUILD_DIR)
