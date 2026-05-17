#include "redFs.h"
#include <stdio.h>
#include <stdlib.h>


#define NOTY(content) printf("[PARTITIONING TEST]: "content"\n")

int main(int argc, char** argv){
	NOTY("Testing partitioning function and disk initialization");
	if(argc < 3){
		NOTY("Not enough argument to begin the testing sequence");
		return 1;
	}
	char* disk_name = argv[1];
	int size = atoi(argv[2]);
	redFs_open_static_virtual_memory(disk_name);
	int ret = 0;
	uint32_t partition_id = 0;
	char* partition_name = "partition_test";
	if(!redFs_partition_defined(NULL)){
		ret = redFs_init_disk(size);
		if(ret) return ret;
		ret = redFs_create_partition(partition_name, size);
		if(ret) return ret;
	}else if(!redFs_partition_defined(partition_name)){
		ret = redFs_create_partition(partition_name, size);
		if(ret) return ret;
	}
	partition_id = redFs_get_partition_id_from_name(partition_name);
	if(!partition_id) return PARTITION_NOT_FOUND_ERROR;

	Red_Header* rh = {0};
	redFs_get_partition_header(partition_id, &rh);
	ret = redFs_partition_header_sanity_check(&rh);
	if(ret) return ret;
	
	printf("Incomplete testing program\n");
	abort();

	return 0;
}
