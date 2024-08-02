#***************************************************************************************
# Copyright (c) 2014-2022 Zihao Yu, Nanjing University
#
# NPC is licensed under Mulan PSL v2.
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

include $(NPC_HOME)/tools/difftest.mk

# compile_git:
# 	$(call git_commit, "compile NPC")
# $(BIN): compile_git

# Some convenient rules

override ARGS ?= --log=$(BUILD_DIR)/npc-log.txt  #--ftrace=$(BUILD_DIR)/$(ALL)-$(ARCH).elf
override ARGS += $(ARGS_DIFF) --batch

# Command to execute NPC
IMG ?=
NPC_EXEC := $(BIN) $(ARGS) $(IMG) 

run-env:  $(BIN) $(DIFF_REF_SO)

# run: run-env
# 	$(call git_commit, "run NPC")
# 	$(NPC_EXEC)

run:$(BIN)
	$(call git_commit, "sim RTL") #DO NOT REMOVE THIS LINE!!!
	$(info $(ARGS))
	$(info ARCH_H $(ARCH_H))
	$(EXE)  $(ARGS) $(IMG)
	
# gdb: run-env
# 	$(call git_commit, "gdb NPC")
# 	gdb -s $(BINARY) --args $(NPC_EXEC)

gdb:$(BIN)
	$(call git_commit, "sim RTL") #DO NOT REMOVE THIS LINE!!!
	gdb -s $(EXE) --args $(EXE) $(ARGS) $(IMG)

wave:
	gtkwave $(WAVE)

clean-tools = $(dir $(shell find ./tools -maxdepth 2 -mindepth 2 -name "Makefile"))
$(clean-tools):
	-@$(MAKE) -s -C $@ clean
clean-tools: $(clean-tools)
clean-all: clean distclean clean-tools

.PHONY: run gdb run-env clean-tools clean-all $(clean-tools)
