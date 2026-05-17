#define VIRTIO
#include "redFs.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NOTY(content) printf("[PARTITIONING TEST]: "content"\n")
#define NOTYF(content, ...) printf("[PARTITIONING TEST]: "content"\n", __VA_ARGS__);

int main(int argc, char** argv){
	char buffer[64];
	char name[64];
	NOTY("Testing partitioning function and disk initialization");
	if(argc < 4){
		NOTY("Not enough argument to begin the testing sequence: foramt <virtual_disk> <disk_size> <base_partition_size>");
		return 1;
	}
	char* disk_name = strdup(argv[1]);
	uint32_t disk_size = (uint32_t)atoi(argv[2]);
	uint32_t size = (uint32_t)atoi(argv[3]);
	redFs_open_static_virtual_memory(disk_name);
	int ret = 0;
	
	uint32_t partition_id = 0;
	char* partition_name = "partition_test";

	if(!redFs_partition_defined(NULL)){
		NOTY("Initializing virtual disk");
		ret = redFs_init_disk(disk_size);
		if(ret) return ret;
		NOTY("Done");
		NOTY("Creating base partition");
		ret = redFs_create_partition(partition_name, size);
		if(ret){
			redFs_strerror(ret);
			return ret;
		}
		NOTY("Partition created");
	}else if(!redFs_partition_defined(partition_name)){
		NOTY("Creating base partition");
		ret = redFs_create_partition(partition_name, size);
		if(ret) return ret;
		NOTY("Partition created");
	}
	NOTY("Obtaining partition id");
	partition_id = redFs_get_partition_id_from_name(partition_name);
	if(!partition_id) return PARTITION_NOT_FOUND_ERROR;
	
	NOTY("Fetching partition header");
	Red_Header rh = {0};
	redFs_get_partition_header(partition_id, &rh);
	ret = redFs_partition_header_sanity_check(&rh);
	if(ret) return ret;
	NOTY("Validation completed");
	
	NOTY("Printing partition table");
	redFs_print_ptable();

	NOTY("Printing fstab from the disk location, not from the header");
	redFs_print_fstab(partition_id);

	NOTY("Printing partition header");
	redFs_print_partition_header(&rh);
	
	NOTY("Printing fragmentation report");
	redFs_print_fragmentation_report(&rh.fstab);
	
	NOTY("Synching partition");
	rh.cache_timing = 1; // forcing synch by setting a random cache timing value
	ret = redFs_sync_partition(&rh);
	if(ret) return ret;

	NOTY("Deleting partition");
	ret = redFs_delete_partition(partition_name,partition_id);
	if(ret) return ret;
	
	NOTY("Creating random partitions");
	for(int i=0;i<10; i++){
		strcpy(name, "t");
		sprintf(buffer, "%d", i);
		strcat(name, buffer);
		NOTYF("-> creating %s", name);
		ret = redFs_create_partition(name, 0x1FFFFF);
		if(ret) return ret;
	}
	NOTY("Printing partition table");
	redFs_print_ptable();

	NOTY("Deleting random partitions");
	srand(time(NULL));
	for(int i=0;i<5; i++){
		strcpy(name, "t");
		sprintf(buffer, "%d", rand()%10);
		strcat(name, buffer);
		NOTYF("-> deleting %s", name);
		ret = redFs_delete_partition(name, redFs_get_partition_id_from_name(name));
	}
	NOTY("Printing partition table");
	redFs_print_ptable();

	NOTY("Creating a bigger partition and testing the autofill");
	ret = redFs_create_partition(partition_name, size);
	if(ret) return ret;
	
	NOTY("Printing partition table");
	redFs_print_ptable();
		
	NOTY("Try to read a non existing partition");
	redFs_get_partition_header(partition_id, &rh);
	ret = redFs_partition_header_sanity_check(&rh);
	if(!ret) return ret;
	
	NOTY("Try deleting unexisting partition");
	for(int i=0;i<5; i++){
		strcpy(name, "dt_");
		sprintf(buffer, "%d", rand()%10);
		strcat(name, buffer);
		ret = redFs_delete_partition(name, redFs_get_partition_id_from_name(name));
		if(!ret) return ret;
	}
	NOTY("Printing partition table");
	redFs_print_ptable();
	partition_id = redFs_get_partition_id_from_name(partition_name);
	redFs_get_partition_header(partition_id, &rh);
	NOTYF("Formatting partition '%u' and fetch the header back", partition_id);
	ret = redFs_erase_partition(partition_id);
	if(ret) return ret;
	// after each erase you must always fetch the latest partition header, see redFs.h next to redFs_erase_partition
	partition_id = redFs_get_partition_id_from_name(partition_name);
	redFs_get_partition_header(partition_id, &rh);
	ret = redFs_partition_header_sanity_check(&rh);
	if(ret) {
		return ret;
	}
	NOTY("Printing partition table");
	redFs_print_ptable();
	NOTY("Printing partition header");
	redFs_print_partition_header(&rh);
	
	NOTY("Synching partition");
	rh.cache_timing = 1; // forcing synch by setting a random cache timing value
	ret = redFs_sync_partition(&rh);
	if(ret) return ret;

	NOTY("Testing completed");
	redFs_close_static_virtual_memory();
	return 0;
}
