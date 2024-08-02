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

# ifneq ($(CONFIG_ITRACE)$(CONFIG_IQUEUE),)
CXXSRC = $(shell find $(abspath ./utils) -name "*.cc")
# CXXSRC = utils/disasm.cc
CXXFLAGS = $(shell llvm-config --cxxflags) -fPIE
CXXFLAGS += $(filter-out -D__STDC_FORMAT_MACROS, $(CXXFLAGS_N))
LIBS += $(shell llvm-config --libs)
# endif
