#include "redFs.h"
#include <stdio.h>
#include <stdlib.h>


#define NOTY(content) printf("[FOLDER TEST]: "content"\n")

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

	NOTY("Creating folder pool and deleting random elements");
	for(int i=0;i<340;i++){
		strcpy(name, "folder_");
		sprintf(buffer, "%d", i);
		strcat(name, buffer);
		ret = redFs_create_directory(&f_header, name, 0);
		if(ret){
			redFs_strerror(ret);
			return ret;
		}
	}
	for(uint32_t i=0;i<200 && ret == 0;i++){
		strcpy(name, "folder_");
		sprintf(buffer, "%d", (int)(rand()%340));
		strcat(name, buffer);
		printf("Deleting folder %s\n", name);
		ret = redFs_remove_directory(&f_header,name);
	}

	NOTY("Generating base filesystem tree\n");
	int base_tree_size = 11;
	int mkdir_num = 20;
	char** folder_tree = (char**)malloc(sizeof(char*)*base_tree_size);
	
	for(int i=0;i<base_tree_size;i++){
		strcat(name, "base_root_dir_");	
		sprintf(buffer, "%d", j);
		strcat(name, buffer);
		folder_tree[i] = strdup(name);
	}

	for(int i=0;i<base_tree_size; i++){
		redFs_create_directory(&header, folder_tree[i], 0);
		for(int j=0; j<mkdir_num; j++){
			strcpy(name, folder_tree[i]);
			strcat(name, "/f_");	
			sprintf(buffer, "%d", j);
			strcat(name, buffer);
			ret = redFs_create_directory(&header, name, 0);
			if(ret) return ret;
		}
	}
	NOTY("Synching base changes to the disk\n");
	ret = redFs_sync_partition(&header);

	NOTY("Recursive tree population, creating subdir");	
	for(int i=0;i<base_tree_size;i++){
		ret = redFs_change_directory(&header, "/");
		if(ret) return ret;
		ret = redFs_change_directory(&header, folder_tree[i]);
		if(ret) return ret;
		for(int j=0;j<mkdir_num;j++){
			strcpy(name, "f_");
			sprintf(buffer, "%d", j);
			strcat(name, buffer);
			ret = redFs_change_directory(&header, folder_tree[i]);
			if(ret) return ret;
			for(int k=0;k<mkdir_count; k++){
				strcat(name, "f_");	
				sprintf(buffer, "%d", k);
				strcat(name, buffer);
				ret = redFs_create_directory(&header, name, 0);
				if(ret) return ret;
			}
			ret = redFs_change_directory(&header, "..");
			if(ret) return ret;
		}
	}
	NOTY("Synching base changes to the disk\n");
	ret = redFs_sync_partition(&header);

	NOTY("Testing recursive remove");
	
	printf("Testing program incomplete\n");
	abort();

	redFs_close_static_virtual_memory();
	NOTY("Testing completed");
	return ret;
}
