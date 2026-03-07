#include <stdio.h>
#include <stdlib.h>


#include "redFs.h"


static char virtual_disk[0xFFFF];

int main(){
	printf("Size of redfstab: %zu\n", sizeof(Red_Fstab));
	printf("Size of a single node: %zu\n", sizeof(Red_Node));
	printf("Size of a single file node: %zu\n", sizeof(Red_File));

	printf("Size of redfs partition table: %zu\n", sizeof(Red_ptable));

	return 0;
}
