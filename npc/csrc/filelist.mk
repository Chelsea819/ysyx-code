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

CSRCS-y += $(NPC_HOME)/csrc/sim_main.cpp
DIRS-y += $(NPC_HOME)/csrc/cpu $(NPC_HOME)/csrc/monitor $(NPC_HOME)/csrc/utils $(NPC_HOME)/csrc/memory
# DIRS-$(CONFIG_MODE_SYSTEM) += $(NPC_HOME)/csrc/memory
DIRS-BLACKLIST-$(CONFIG_TARGET_AM) += $(NPC_HOME)/csrc/monitor/sdb

SHARE = $(if $(CONFIG_TARGET_SHARE),1,0)
LIBS += $(if $(CONFIG_TARGET_NATIVE_ELF),-lreadline -ldl -pie,)

ifdef mainargs
ASFLAGS += -DBIN_PATH=\"$(mainargs)\"
endif
