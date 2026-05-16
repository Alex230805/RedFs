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
	const char* disk_name = argv[1];
	int size = atoi(argv[2]);
	redFs_open_static_virtual_memory(disk_name);
	redFs_init_disk(size);
	
	// TODO: add check to search for an available partition 
	NOTY("Initializing test ....\n");
	int ret = 0;
	char* partition_name = "dir_test";
	ret = redFs_create_partition(partition_name, partition_0_size);
	redFs_strerror(ret);
	if(ret) return ret;
	
	// TODO: add redFs_get_partition_id 
	int id = 1001;
	Red_Header header = {0};
	redFs_get_partition_header(id, &header);
	
	
	
	printf("Testing program incomplete\n");
	abort();
	
	redFs_close_static_virtual_memory();
	NOTY("Testing completed");
	return 0;
}
