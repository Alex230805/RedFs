#define VIRTIO
#include "redFs.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NOTY(content) printf("[FOLDER TEST]: "content"\n")
#define NOTYF(content, ...) printf("[FOLDER TEST]: "content"\n", __VA_ARGS__);


typedef struct{
	char* ptr;
	int len;
}String;


typedef struct{
	String* strings;
	size_t  tracker;
	size_t	size;
}String_List;

#define LOCAL_SIZE 1024*8
static char local_mem[LOCAL_SIZE] =  {0};
static size_t local_tracker =	0;
static size_t local_size =		LOCAL_SIZE;

void* local_alloc(size_t size){
	if(size+local_tracker >= local_size) local_tracker = 0;
	void* ptr = &local_mem[local_tracker];
	local_tracker += size;
	if(local_tracker >= local_size) local_tracker = 0;
	return ptr;
}

char* local_strdup(char* name){
	int size = strlen(name);
	char* buff = local_alloc(sizeof(char)*size+1);
	strcpy(buff, name);
	buff[size] = '\0';
	char* pos = strchr(buff, '\n');
	if(pos) *pos = '\0';
	return buff;
}

void string_list_push(String_List* list, char* str){
	if(list->strings == NULL){
		list->strings = (String*)local_alloc(sizeof(String)*64);
		list->tracker = 0;
		list->size = 64;
	}
	list->strings[list->tracker].ptr = local_strdup(str);
	list->strings[list->tracker].len = strlen(list->strings[list->tracker].ptr);
	list->tracker += 1;
	return;
}

int main(int argc, char** argv){
	char buffer[64];
	char name[64];
	NOTY("Testing folder creation and directory navigation");
	if(argc < 3) {
		NOTY("Not enough argument to begin the testing sequence: file <virtual_disk> <disk_size>");
		return 1;
	}
	const char* disk_name = argv[1];
	int size = atoi(argv[2]);
	redFs_open_static_virtual_memory(disk_name);
	redFs_init_disk(size);
	
	// TODO: add check to search for an available partition 
	NOTY("Initializing test ....");
	int ret = 0;
	char* partition_name = "dir_test";
	ret = redFs_create_partition(partition_name, 0xA0FFFF);
	if(ret) return ret;
	
	// TODO: add redFs_get_partition_id 
	int id = 1001;
	Red_Header header = {0};
	redFs_get_partition_header(id, &header);

	NOTY("Creating folder pool and deleting random elements");
	for(int i=0;i<110;i++){
		strcpy(name, "folder_");
		sprintf(buffer, "%d", i);
		strcat(name, buffer);
		NOTYF("Creating %s", name);
		ret = redFs_create_directory(&header, name, 0);
		if(ret){ return ret; }
	}
	for(uint32_t i=0;i<30;i++){
		strcpy(name, "folder_");
		sprintf(buffer, "%d", (int)(rand()%110));
		strcat(name, buffer);
		if(redFs_remove_directory(&header,name)){
			if(redFs_errno == NODE_NOT_FOUND_ERROR){
				i=-1;
			}else{
				break;
			}
		}else{
			NOTYF("Deleting folder %s", name);
		}
	}
	ret = 0;
	NOTY("Synching base changes to the disk");
	ret = redFs_sync_partition(&header);
	NOTY("Print fragmentation report");
	redFs_print_fragmentation_report(&header.fstab);
	NOTY("Generating base filesystem tree");
	#define BASE_TREE_SIZE 10
	int mkdir_num = 8;

	String_List sl = {0};
	for(int i=0;i<BASE_TREE_SIZE; i++){
		usleep(20000);
		strcpy(name, "base_root_dir_");	
		sprintf(buffer, "%d\n", i);
		strcat(name, buffer);
		char* dir = local_strdup(name);
		string_list_push(&sl, dir);
		NOTYF("Creating folder '%s'", dir);
		redFs_create_directory(&header, dir, 0);
		NOTYF("Going inside '%s'", dir);
		ret = redFs_change_path(&header, dir);
		if(ret) return ret;
	
		for(int j=0; j<mkdir_num; j++){
			usleep(10000);
			strcpy(name, "f_");
			sprintf(buffer, "%d", j);
			strcat(name, buffer);
			NOTYF("Creating subdir '%s'", name);
			// NOTE: create subdir is path depentend, if you specify a path structure like /this/path it will unwrap the tree and search for 
			// a folder named "this" inside the root folder, and then create "path" inside "this". If you specify a folder with just a name 
			// like "this" or "./this" then the folder will be created on the current opened node folder.
			ret = redFs_create_directory(&header, local_strdup(name), 0); 
			if(ret) return ret;
		}
		ret = redFs_print_current_dir_content(&header);
		if(ret) return ret;
		ret = redFs_change_path(&header, "/");
		if(ret) return ret;
	}

	NOTY("Synching base changes to the disk");
	ret = redFs_sync_partition(&header);
	NOTY("Print fragmentation report");
	redFs_print_fragmentation_report(&header.fstab);
	
	NOTY("Recursive tree population, creating subdir");	
	for(int i=0;i<BASE_TREE_SIZE - 5;i++){
		usleep(10000);
		NOTY("change dir to root");
		ret = redFs_change_directory(&header, "/");
		if(ret) return ret;
		NOTYF("Going inside %s", sl.strings[i].ptr);
		ret = redFs_change_directory(&header, sl.strings[i].ptr);
		if(ret) return ret;
		for(int j=0;j<mkdir_num;j++){
			strcpy(name, "f_");
			sprintf(buffer, "%d", j);
			strcat(name, buffer);
			NOTYF("Changing directory to subdir of %s: %s", sl.strings[i].ptr,name);
			ret = redFs_change_directory(&header, name);
			if(ret) return ret;
			for(int k=0;k<mkdir_num; k++){
				usleep(10000);
				strcpy(name, "dd_ps_");	
				sprintf(buffer, "%d", k);
				strcat(name, buffer);
				NOTYF("Creating subdir %s", name);
				ret = redFs_create_directory(&header, name, 0);
				if(ret) return ret;
			}
			ret = redFs_print_current_dir_content(&header);
			if(ret) return ret;
			NOTYF("Returning inside %s", sl.strings[i].ptr);
			ret = redFs_change_directory(&header, "..");
			if(ret) return ret;
		}
		usleep(220000);
		NOTY("Print fragmentation report");
		redFs_print_fragmentation_report(&header.fstab);
	}
	NOTY("Synching base changes to the disk");
	ret = redFs_sync_partition(&header);
	NOTY("Fetching content from folder, content of '/'");

	char** dir_content = redFs_get_dir_content(&header, "/");
	if(!dir_content) return 1;
	uint32_t dir_content_size = *((uint32_t*)(dir_content-sizeof(uint32_t)));
	for(uint32_t i=0;i<dir_content_size; i++){
		printf("%s\n", dir_content[i]);
	}
	
	NOTY("Fetching content from folder, content of '/base_root_dir_0'");
	dir_content = redFs_get_dir_content(&header, "base_root_dir_0");
	if(!dir_content) return redFs_errno;
	dir_content_size = *((uint32_t*)(dir_content-sizeof(uint32_t)));
	for(uint32_t i=0;i<dir_content_size; i++){
		printf("%s\n", dir_content[i]);
	}
	NOTY("Content count of '/base_root_dir_0'");
	uint32_t content_count = 0;
	if(redFs_get_dir_content_count(&header,"base_root_dir_0", &content_count)){
		return redFs_errno;
	}
	NOTYF("Content count: %d\n", content_count);
	redFs_close_static_virtual_memory();
	NOTY("Test completed");
	return ret;
}
