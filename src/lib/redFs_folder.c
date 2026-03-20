#define REDFS_FOLDER_IMP

#include "redFs_folder.h"

char* redFs_malloc(size_t size){
	if(redFs_local_buffer_tracker+size > BUFFER_SIZE){
		redFs_local_buffer_tracker = 0;
	}
	char* ptr = &redFs_local_buffer[redFs_local_buffer_tracker];
	redFs_local_buffer_tracker += size;
	return ptr;
}

char** redFs_chop_path(char* path){
	uint32_t len = 0;
	if(strlen(path) < 1) return NULL;
	if(path[0] == '/'){ path+=sizeof(char);}
	
	for(uint32_t i=0;i<strlen(path);i++){
		if(path[i] == '/') len += 1;	
	}

	char** folder_list = (char**)redFs_malloc(sizeof(char*)*(len+2));
	folder_list = &folder_list[1];

	char* ptr = strchr(path, '/');
	bool halt = false;
	uint32_t subdir_count = 0;

	if(ptr == NULL) {
		ptr = strchr(path, '\0');
		halt = true;
	}
	while(true){
		char* buffer = (char*)redFs_malloc(sizeof(char)*STRING_LIMIT);
		if(ptr-path > 0){
			memcpy(buffer, path, ptr-path);
			buffer[(ptr-path)] = '\0';
			folder_list[subdir_count] = buffer;
			subdir_count += 1;
		}
		if(halt) break;
		path = ptr+sizeof(char);
		ptr = strchr(path, '/');
		if(ptr == NULL){
			ptr = strchr(path, '\0');
			halt = true;
		}
	}
	char* len_buffer = (char*)redFs_malloc(sizeof(uint32_t));
	memcpy(len_buffer, &subdir_count, sizeof(uint32_t));
	folder_list[-1] = len_buffer;

	return folder_list;
}

char* redFs_path_pop_last(char** chopped_path){
	uint32_t len;
	memcpy(&len,chopped_path[-1], sizeof(uint32_t));
	if(len == 0) return NULL;
	char* name = chopped_path[len-1];
	len -= 1;
	memcpy(chopped_path[-1],&len, sizeof(uint32_t));
	return name;
}

uint32_t redFs_get_path_dir_count(char** chopped_path){
	uint32_t len;
	memcpy(&len, chopped_path[-1], sizeof(uint32_t));
	return len;
}


int redFs_get_current_directory(Red_Header* header, Red_Node* node){
	int ret = redFs_cache_update(header);
	if(ret) return ret;
	return redFs_node_read(header->current_node, node);
}


int redFs_change_directory(Red_Header* header, char* dir_name){ // '.' and '..' are supported
	int ret = 0;
	Red_Node node = {0};
	Red_Node n = {0};
	ret = redFs_node_read(header->current_node, &node);
	if(ret) return ret;
	bool end = false;
	RED_PTR ptr = 0;

	if(strcmp("..", dir_name) == 0){
		if(node.f_node != 0) header->current_node = node.f_node;
		return 0;
	}
	if(strcmp(".", dir_name) == 0){
		header->current_node = header->current_node;
		return 0;
	}

	while(!end){
		for(uint32_t i=0;i<node.content_count && ptr == 0; i++){
			ret = redFs_node_read(node.content[i], &n);
			if(ret) return ret;
			if(n.type == PAGE_IS_FOLDER && strcmp(dir_name, n.name) == 0){
				ptr = node.content[i];
				end = true;
			}
		}
		if(!end){
			if(node.chained){
				ret = redFs_node_read(node.next_page, &node);
				if(ret) return ret;
			}else{
				return (int)FOLDER_NOT_FOUND_ERROR;
			}
		}
	}
	header->current_node = ptr;
	ret = redFs_cache_update(header);
	return ret;
}

/* "full_path" means the path and the name of the folder included. It could be 
 * a static or a relative path
 * */

int redFs_create_directory(Red_Header* header, char* full_path, int permissions){
	char** cp = redFs_chop_path(full_path);
	char* name = redFs_path_pop_last(cp);
	int ret = 0;
	RED_PTR cache = header->current_node;
	if(full_path[0] == '/') header->current_node = header->root;
	for(uint32_t i=0; i < redFs_get_path_dir_count(cp); i++){
		ret = redFs_change_directory(header, cp[i]);
		if(ret) return ret;
	}
	ret = redFs_node_create_child_node(header, name, permissions, PAGE_IS_FOLDER, header->current_node);
	if(ret) return ret;
	header->current_node = cache;
	ret = redFs_cache_update(header);
	return ret;
}

int redFs_remove_directory(Red_Header* header, char*full_path){
	char** cp = redFs_chop_path(full_path);
	char* name = redFs_path_pop_last(cp);
	RED_PTR cache = header->current_node;
	int ret = 0;
	if(full_path[0] == '/') header->current_node = header->root;
	for(uint32_t i=0; i < redFs_get_path_dir_count(cp); i++){
		ret = redFs_change_directory(header, cp[i]);
		if(ret) return ret;
	}
	ret = redFs_node_remove_child_node(header, name, header->current_node);
	if(ret) return ret;
	header->current_node = cache;
	ret = redFs_cache_update(header);
	return ret;
}

char* redFs_get_current_dir_name(Red_Header* header){
	Red_Node node = {0};
	int ret = redFs_get_current_directory(header, &node);
	if(ret) return NULL;
	char* name = redFs_malloc(sizeof(char)*STRING_LIMIT);
	strcpy(name, node.name);
	ret = redFs_cache_update(header);
	if(ret) return NULL;
	return name;
}

int redFs_get_current_dir_content(Red_Header* header){
	Red_Node node = {0};
	int ret = redFs_get_current_directory(header, &node);
	if(ret) return ret;
	uint32_t count = redFs_node_get_content_count(&node);
	uint32_t i=0;
	printf("Content of %s:\n\n", node.name);
	printf("perm	type	name\n\n");
	printf(".  0x%x\tfolder\t./\n", PAGE_DEF_PERMISSION);
	printf(".  0x%x\tfolder\t../\n", PAGE_DEF_PERMISSION);
	while(i < count){
		for(uint32_t j=0;j<node.content_count;j++, i++){
			Red_Node n = {0};
			ret = redFs_node_read(node.content[j], &n);
			if(ret) return ret;
			printf(".  0x%x", n.permissions);
			if(n.type == PAGE_IS_FOLDER){
				printf("\tfolder\t");
				printf("%s/\n", n.name);
			}else{
				printf("\tfile\t");
				printf("%s\n", n.name);
			}
		}
		if(node.chained){
			if(node.next_page != 0){
				ret = redFs_node_read(node.next_page, &node);
			}else{
				return PARTITION_NODE_READING_ERROR;
			}
		}
	}
	ret = redFs_cache_update(header);
	return ret;
}

int redFs_change_absolute_path(Red_Header* header, char* path){
	header->current_node = header->root;
	int ret = redFs_cache_update(header);
	if(ret) return ret;
	return redFs_change_path(header, path);
}

int redFs_change_path(Red_Header*header, char* path){
	char** chopped_path = redFs_chop_path(path);
	for(uint32_t i=0; i < redFs_get_path_dir_count(chopped_path); i++){
		int ret = redFs_change_directory(header, chopped_path[i]);
		if(ret) return ret;
	}
	return 0;
}
