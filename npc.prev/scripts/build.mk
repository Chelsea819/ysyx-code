#***************************************************************************************
# Copyright (c) 2014-2022 Zihao Yu, Nanjing University
#
# NEMU is licensed under Mulan PSL v2.
# You can use this software according to the terms and conditions of the Mulan PSL v2.
# You may obtain a copy of Mulan PSL v2 at:
#          http://license.coscl.org.cn/MulanPSL2
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
#
# See the Mulan PSL v2 for more details.
#**************************************************************************************/

-include $(NPC_HOME)/../Makefile
include $(NPC_HOME)/scripts/build.mk


compile_git:
	$(call git_commit, "sim RTL") #DO NOT REMOVE THIS LINE!!!

$(BINARY):compile_git $(SRCS) $(VSRCS) 
	$(info SRCS = $(SRCS))
	$(info INC_PATH = $(INC_PATH))
	$(info LINKAGE = $(LINKAGE))
	$(info LDFLAGS = $(LDFLAGS))
	$(VERILATOR) $(VERILATOR_CFLAGS) \
		--top-module $(TOPNAME) $^ $(addprefix -CFLAGS , $(CFLAGS))  \
		$(addprefix -LDFLAGS , $(LDFLAGS)) \
		$(addprefix -CFLAGS , $(CXXFLAGS))	\
		--Mdir $(OBJ_DIR) --exe -o $(abspath $(BINARY))

# Some convenient rules

override ARGS ?= --log=$(BUILD_DIR)/nemu-log.txt  --ftrace=$(BUILD_DIR)/$(ALL)-$(ARCH).elf -b
override ARGS += $(ARGS_DIFF) #--batch

# Command to execute NEMU
IMG ?=
NEMU_EXEC := $(BINARY) $(ARGS) $(IMG) 

run-env:  $(BINARY) $(DIFF_REF_SO)

run:$(BINARY)
	$(call git_commit, "sim RTL") #DO NOT REMOVE THIS LINE!!!
	$(info $(ARGS))
	$(info $(BUILD_DIR))
	$(EXE)  $(ARGS) $(IMG)
	
gdb: run-env
	$(call git_commit, "gdb NEMU")
	gdb -s $(BINARY) --args $(NEMU_EXEC)

clean-tools = $(dir $(shell find ./tools -maxdepth 2 -mindepth 2 -name "Makefile"))
$(clean-tools):
	-@$(MAKE) -s -C $@ clean
clean-tools: $(clean-tools)
clean-all: clean distclean clean-tools

.PHONY: run gdb run-env clean-tools clean-all $(clean-tools)
