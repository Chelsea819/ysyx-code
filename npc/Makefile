BUILD_DIR = ./build

PRJ ?= dcs

test:
	./mill -i $(PRJ).test

# VSRCS = $(shell find $(abspath ./vsrc) -name "*.v")
# CSRCS = $(shell find $(abspath ./csrc) -name "*.c" -or -name "*.cpp" -or -name "*.cc")
# SRCS = $(CXXSRC) $(CSRCS)
# $(BIN):$(SRCS) $(VSRCS) 
# 	$(warning INC_PATH = $(INC_PATH))
# 	$(VERILATOR) $(VERILATOR_CFLAGS) \
# 		--top-module $(TOPNAME) $^ $(addprefix -CFLAGS , $(CFLAGS)) \
# 		$(addprefix -LDFLAGS , $(LDFLAGS)) \
# 		$(addprefix -CFLAGS , $(CXXFLAGS))	\
# 		--Mdir $(OBJ_DIR) --exe -o $(abspath $(BIN))

verilog:
	$(call git_commit, "generate verilog")
	mkdir -p $(BUILD_DIR)
	./mill -i $(PRJ).runMain Elaborate --target-dir $(BUILD_DIR)

help:
	./mill -i $(PRJ).runMain Elaborate --help

reformat:
	./mill -i __.reformat

checkformat:
	./mill -i __.checkFormat

bsp:
	./mill -i mill.bsp.BSP/install

idea:
	./mill -i mill.idea.GenIdea/idea

clean:
	-rm -rf $(BUILD_DIR)

.PHONY: test verilog help reformat checkformat clean

sim:
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	@echo "Write this Makefile by yourself."

-include ../Makefile
