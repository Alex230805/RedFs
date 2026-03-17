#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#define VIRTIO
#include "redFs.h"


#define VIRTUAL_MEMORY "virtual_memory"
#define DISK_SIZE 0xFFFFFFFF

#define SEP()\
	printf("\n============================================================\n");

int main(){
	srand(time(NULL));
	printf("NOTE: file stream on '%s' is open for the entire virtual test\n", VIRTUAL_MEMORY);

	redFs_open_static_virtual_memory(VIRTUAL_MEMORY);

	printf("General stats:\n");
	printf("Size of redfstab: %.2f Mb\n", (double)sizeof(Red_Fstab)/1000000);
	printf("Memory block size: %d\n", BLOCK_SIZE);
	printf("Total maximum memory block count: %d\n",BLOCK_COUNT);
	printf("Segment per memory block: %d\n", BLOCK_NODE_COUNT);
	printf("Size of a single node: %zu\n", sizeof(Red_Node));
	printf("Size of a single file node: %zu\n", sizeof(Red_File));
	printf("Size of redfs partition table: %zu\n", sizeof(Red_ptable));
	printf("Node array limit for node: %zu\n", NODE_ARRAY_LIMIT/sizeof(RED_PTR));

	SEP();
	printf("Initializing disk of size: %.2f Mb\n",(double)DISK_SIZE/1000000);
	redFs_init_disk(DISK_SIZE);
	printf("Generating partitions: \n");

	int ret = 0;
	const uint32_t partition_0_size = 0xF0FFFF;
	char * partition_0_name = "part_0"; 
	printf("-> Creating partition '%s' of size: %.2f Mb\n", partition_0_name,(double)partition_0_size/1000000);
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

	SEP();
	printf("Testing sync function:\n");
	header.cache_timing = 1;
	ret = redFs_sync_partition(&header);
	if(ret){
		printf("aborting '%s' creation\n",partition_2_name);
	}
	printf("Header synched to the drive, now fetching it back to see if data maches\n");
	redFs_print_fstab(1003); // print fstab directly from the drive
	
	SEP();
	printf("Generating fragmentation report on a new partition 'frag_test'\n");
	ret = redFs_create_partition("frag_test", 0x1FFFFF);
	if(ret){
		printf("aborting '%s' creation\n",partition_1_name);
		redFs_strerror(ret);
	}
	redFs_print_ptable();
	Red_Header f_header = {0};
	redFs_get_partition_header(1005, &f_header);
	redFs_print_fragmentation_report(&f_header.fstab);

	SEP();
	printf("Testing auto fill space to optimize partition creation\n");
	for(int i=0;i<8;i++){
		char buffer[8];
		char name[8];
		strcpy(name, "t");
		sprintf(buffer, "%d", i);
		strcat(name, buffer);
		ret = redFs_create_partition(name, 0x1FFFFF);
		if(ret){
			printf("aborting '%s' creation\n",partition_1_name);
			redFs_strerror(ret);
		}
		printf("Creating partition '%s'\n", name);
	}
	redFs_print_ptable();

	SEP();
	printf("Testing folder creation\n");
	printf("\nCreating folder list, wait a few seconds ..\n");
	for(int i=0;i<340;i++){
		char buffer[8];
		char name[16];
		strcpy(name, "folder_");
		sprintf(buffer, "%d", i);
		strcat(name, buffer);
		ret = redFs_node_create_child_node(&f_header, name, 0, PAGE_IS_FOLDER, f_header.current_node);
		if(ret){
			redFs_strerror(ret);
			return ret;
		}
		//printf("Creating folder %s\n", name);
	}
	redFs_node_show(f_header.current_node);

	SEP()
	printf("\nPrinting fragment map after folder allocation: \n");
	redFs_print_fragmentation_report(&f_header.fstab);
	SEP();
	printf("Trying to delete random folders from the current node\n");
	for(uint32_t i=0;i<200 && ret == 0;i++){
		char buffer[8];
		char name[16];
		strcpy(name, "folder_");
		sprintf(buffer, "%d", (int)(rand()%340));
		strcat(name, buffer);
		printf("Deleting folder %s\n", name);
		ret = redFs_node_remove_child_node(&f_header, name, f_header.current_node);
	}
	redFs_print_fragmentation_report(&f_header.fstab);
	SEP();
	printf("Testing recursive node remove on a root subdir\n");
	printf("TODO: to properly do that we need the cd function\n");

	redFs_close_static_virtual_memory();
	return 0;
}
