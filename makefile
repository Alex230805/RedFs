COMP:=gcc
FLAGS:=-Wall -Wextra -O3

INC:=-I./src/include
LIB:=-L./src/bin

all: null

null: 
	# no option selected, please set one of the following target: 
	# 
	# - test: compile test program
	# - lib: compile libredfs static library

test: lib_virt ./src/test/main.c ./src/test/format.c ./src/test/dir.c ./src/test/file.c
	$(COMP) $(FLAGS) $(INC) $(LIB) ./src/test/file.c -o ./src/test/bin/file -lredfs
	$(COMP) $(FLAGS) $(INC) $(LIB) ./src/test/dir.c -o ./src/test/bin/dir -lredfs
	$(COMP) $(FLAGS) $(INC) $(LIB) ./src/test/format.c -o ./src/test/bin/format -lredfs
	$(COMP) $(FLAGS) $(INC) $(LIB) ./src/test/main.c -o ./src/test/test -lredfs

lib_virt: ./src/lib/redFs.c ./src/lib/redFs_io.c ./src/lib/redFs_folder.c ./src/lib/redFs_node.c
	$(COMP) $(FLAGS) $(INC) -g -c ./src/lib/redFs.c -o ./src/bin/redFs.o
	$(COMP) $(FLAGS) $(INC) -g -DVIRTIO -c ./src/lib/redFs_io.c -o ./src/bin/redFs_io.o
	$(COMP) $(FLAGS) $(INC) -g -c ./src/lib/redFs_node.c -o ./src/bin/redFs_node.o
	$(COMP) $(FLAGS) $(INC) -g -c ./src/lib/redFs_folder.c -o ./src/bin/redFs_folder.o
	$(COMP) $(FLAGS) $(INC) -c ./src/lib/redFs_file.c -o ./src/bin/redFs_file.o
	ar rc ./src/bin/libredfs.a ./src/bin/*.o
	ranlib ./src/bin/libredfs.a


lib: ./src/lib/redFs.c ./src/lib/redFs_io.c ./src/lib/redFs_folder.c
	$(COMP) $(FLAGS) $(INC) -c ./src/lib/redFs.c -o ./src/bin/redFs.o
	$(COMP) $(FLAGS) $(INC) -c ./src/lib/redFs_io.c -o ./src/bin/redFs_io.o
	$(COMP) $(FLAGS) $(INC) -c ./src/lib/redFs_node.c -o ./src/bin/redFs_node.o
	$(COMP) $(FLAGS) $(INC) -c ./src/lib/redFs_folder.c -o ./src/bin/redFs_folder.o
	$(COMP) $(FLAGS) $(INC) -c ./src/lib/redFs_file.c -o ./src/bin/redFs_file.o
	ar rc ./src/bin/libredfs.a ./src/bin/*.o
	ranlib ./src/bin/libredfs.a
