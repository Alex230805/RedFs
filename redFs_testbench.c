#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define VIRTIO
#include "redFs.h"


#define VIRTUAL_MEMORY "virtual_memory"

int main(){
	printf("NOTE: file stream on '%s' is open for the entire virtual test\n", VIRTUAL_MEMORY);

	redFs_open_static_virtual_memory(VIRTUAL_MEMORY);

	printf("Size of redfstab: %zu\n", sizeof(Red_Fstab));
	printf("Size of a single node: %zu\n", sizeof(Red_Node));
	printf("Size of a single file node: %zu\n", sizeof(Red_File));
	printf("Size of redfs partition table: %zu\n", sizeof(Red_ptable));

	//redFs_init_disk(0x0FFFFFFF);
	//redFs_create_partition("partition_0", 0xFFFF);
	
	redFs_close_static_virtual_memory();
	return 0;
}
