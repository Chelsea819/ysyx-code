TARGET = example
TEST = $(wildcard ./csrc/*.cpp)
SIM = $(wildcard ./vsrc/*.v)
DIR = ./obj_dir
EXE = $(DIR)/$(TARGET)
WAVE = waveform.vcd

all:
	@echo "Write this Makefile by your self."

sim:
	$(call git_commit, "sim RTL") #DO NOT REMOVE THIS LINE!!!
	@echo "Write this Makefile by your self."
	verilator -Wall --cc -MMD --trace --build -top $(TARGET) -Mdir ./obj_dir $(SIM) $(TEST) --exe  -o $(TARGET) --gdb
	$(EXE)

.PHONY:wave clean sim

wave:
	gtkwave $(WAVE)

clean:
	$(RM) -r $(DIR) $i(WAVE) 
T

include ../Makefile
