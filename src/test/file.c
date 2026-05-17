#define VIRTIO
#include "redFs.h"
#include <stdio.h>
#include <stdlib.h>

#define NOTY(content) printf("[FILE TEST]: "content"\n")


int main(int argc, char** argv){
	char buffer[64];
	char name[64];
	NOTY("Testing folder creation and directory navigation");
	if(argc < 3) {
		NOTY("not enough parameter to begin the testing sequence");
		return 1;
	}
	char* disk_name = argv[1];
	int size = atoi(argv[2]);
	redFs_open_static_virtual_memory(disk_name);
	redFs_init_disk(size);
	
	redFs_close_static_virtual_memory();
	NOTY("Testing completed");
	return 0;
}
