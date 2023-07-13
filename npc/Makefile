all:
	@echo "Write this Makefile by your self."

sim:
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	@echo "Write this Makefile by your self."
	verilator -Wall our.v sim_main.cpp --cc --trace --exe --build
	./obj_dir/Vour
	gtkwave wave.vcd

include ../Makefile
