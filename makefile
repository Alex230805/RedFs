COMP:=gcc
FLAGS:=-Wall -Wextra -ggdb -O3

INC:=-I./src/include
LIB:=-L./src/bin

all: null


null: 
	# no option selected, please set one of the following target: 
	# 
	# - testbench: compile a testbench to test the filesystem
	# - lib: compile libredfs static library

testbench: lib_virt redFs_testbench.c
	$(COMP) $(FLAGS) $(INC) $(LIB) redFs_testbench.c -o testbench -lredfs

lib_virt: ./src/lib/redFs.c ./src/lib/redFs_io.c ./src/lib/redFs_folder.c
	$(COMP) $(FLAGS) $(INC) -c ./src/lib/redFs.c -o ./src/bin/redFs.o
	$(COMP) $(FLAGS) $(INC) -DVIRTIO -c ./src/lib/redFs_io.c -o ./src/bin/redFs_io.o
	$(COMP) $(FLAGS) $(INC) -c ./src/lib/redFs_folder.c -o ./src/bin/redFs_folder.o
	ar rc ./src/bin/libredfs.a ./src/bin/*.o
	ranlib ./src/bin/libredfs.a


lib: ./src/lib/redFs.c ./src/lib/redFs_io.c ./src/lib/redFs_folder.c
	$(COMP) $(FLAGS) $(INC) -c ./src/lib/redFs.c -o ./src/bin/redFs.o
	$(COMP) $(FLAGS) $(INC) -c ./src/lib/redFs_io.c -o ./src/bin/redFs_io.o
	$(COMP) $(FLAGS) $(INC) -c ./src/lib/redFs_folder.c -o ./src/bin/redFs_folder.o
	ar rc ./src/bin/libredfs.a ./src/bin/*.o
	ranlib ./src/bin/libredfs.a
