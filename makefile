COMP:=gcc
FLAGS:=-Wall -Wextra 

INC:=-I./src/include
LIB:=-L./src/bin


all: null


null: 
	# no option selected, please set one of the following target: 
	# 
	# - testbench: compile a testbench to test the filesystem
	# - lib: compile libredfs static library


testbench: lib redFs_testbench.c
	$(COMP) $(FLAGS) $(INC) $(LIB) redFs_testbench.c -o testbench -lredfs

lib: ./src/lib/redFs.c
	$(COMP) $(FLAGS) $(INC) -c ./src/lib/redFs.c -o ./src/bin/redFs.o
	ar rc ./src/bin/libredfs.a ./src/bin/*.o
	ranlib ./src/bin/libredfs.a
