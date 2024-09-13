
SRCS = ./*.cpp ./*.c
LIBS += $(shell llvm-config --libs)

LDFLAGS += -lreadline -ldl -pie $(LIBS) 


run: $(SRCS)
	$(info $(SRCS))
	g++ -c $(LDFLAGS) $(SRCS)
	g++ -o gen_expr ./*.o

.PHONY: run