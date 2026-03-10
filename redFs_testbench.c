#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define VIRTIO
#include "redFs.h"


#define VIRTUAL_MEMORY "virtual_memory"
#define DISK_SIZE 0xFFFFFFFF


#define SEP()\
	printf("\n============================================================\n");

int main(){
	printf("NOTE: file stream on '%s' is open for the entire virtual test\n", VIRTUAL_MEMORY);

	redFs_open_static_virtual_memory(VIRTUAL_MEMORY);

	printf("General stats:\n");
	printf("Size of redfstab: %.2f Mb\n", (double)sizeof(Red_Fstab)/1000000);
	printf("Size of a single node: %zu\n", sizeof(Red_Node));
	printf("Size of a single file node: %zu\n", sizeof(Red_File));
	printf("Size of redfs partition table: %zu\n", sizeof(Red_ptable));

	SEP();
	printf("Initializing disk of size: %.2f Mb\n",(double)DISK_SIZE/1000000);
	redFs_init_disk(DISK_SIZE);
	printf("Generating partitions: \n");

	int ret = 0;
	const uint32_t partition_0_size = 0xF0FFFF;
	char * partition_0_name = "part_0"; 
	printf("-> Creating partition '%s' of size: %.2f\n", partition_0_name,(double)partition_0_size/1000000);
	ret = redFs_create_partition(partition_0_name, partition_0_size);
	redFs_strerror(ret);
	if(ret){
		printf("aborting '%s' creation\n",partition_0_name);
	}


	const uint32_t partition_1_size = 0xBAFFFFF;
	char * partition_1_name = "part_1"; 
	printf("-> Creating partition '%s' of size: %.2f Mb\n", partition_1_name, (double)partition_1_size/1000000);
	ret = redFs_create_partition(partition_1_name, partition_1_size);
	redFs_strerror(ret);
	if(ret){
		printf("aborting '%s' creation\n",partition_1_name);
	}
	
	const uint32_t partition_2_size = 0x3FFFFFFF;
	char * partition_2_name = "data"; 
	printf("-> Creating partition '%s' of size: %.2f Mb\n", partition_2_name,(double)partition_2_size/1000000);
	ret = redFs_create_partition(partition_2_name, partition_2_size);
	redFs_strerror(ret);
	if(ret){
		printf("aborting '%s' creation\n",partition_2_name);
	}
	
	const uint32_t partition_3_size = 0xF0FFFF;
	char * partition_3_name = "AUX"; 
	printf("-> Creating partition '%s' of size: %.2f Mb\n", partition_3_name,(double)partition_3_size/1000000);
	ret = redFs_create_partition(partition_3_name, partition_3_size);
	redFs_strerror(ret);
	if(ret){
		printf("aborting '%s' creation\n",partition_3_name);
	}
	redFs_print_ptable();
	SEP()
	printf("Showing fstab about each partition:\n\n");
	redFs_print_fstab(1001);
	printf("\n");
	redFs_print_fstab(1002);
	printf("\n");
	redFs_print_fstab(1003);
	printf("\n");
	redFs_print_fstab(1004);
	SEP();
	printf("Trying to delete '%s' with ID '%d'\n", partition_0_name, 1001);
	ret = redFs_delete_partition(partition_0_name,1001);
	redFs_strerror(ret);
	redFs_print_ptable();
	SEP();
	printf("Getting partition header for partition %d\n", 1003);
	Red_Header header = {0};
	redFs_get_partition_header(1003, &header);
	printf("Partition header content:\n");
	redFs_print_partition_header(&header);

	redFs_close_static_virtual_memory();
	return 0;
}
