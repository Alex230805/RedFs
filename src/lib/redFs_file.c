#define REDFS_FILE_IMP
#include "redFs_file.h"


int redFs_touch_file_in_current_location(Red_Header* header, char* name, uint8_t permissions){
	int ret = redFs_cache_update(header);
	if(ret) return ret;
	if(redFs_get_file_from_current_folder(header, name) != 0){
		return 	FILE_ALREADY_EXIST;
	}
	return redFs_node_create_child_node(header, name, permissions, PAGE_IS_FILE, header->current_node);
}


int redFs_touch_file(Red_Header* header, char* path, uint8_t permissions){
	RED_PTR current_node = header->current_node;
	if(path[0] == '/') header->current_node = header->root;
	char** cpath = redFs_chop_path(path);
	char* file_name = redFs_path_pop_last(cpath);
	int ret = redFs_change_path_already_chopped(header, cpath);
	if(ret) return ret;
	ret = redFs_touch_file_in_current_location(header, file_name, permissions);
	if(ret) return ret;
	header->current_node = current_node;
	return 0;
}


RED_PTR redFs_get_file_from_current_folder(Red_Header* header, char*name){
	Red_Node node = {0};
	int ret = redFs_node_read(header->current_node, &node);
	if(ret) return ret;
	RED_PTR adr = 0;
	bool end = false;
	while(!end){
		for(uint32_t i=0;i<node.content_count && adr == 0; i++){
			Red_Node n = {0};
			ret = redFs_node_read(node.content[i], &n);
			if(ret) return ret;
			//printf("Comparing %s with %s\n", name, n.name);
			if(strcmp(name, n.name) == 0 && n.type == PAGE_IS_FILE){
				adr = node.content[i];
				end = true;
			}
		}
		if(!end){
			if(node.chained && node.next_page != 0){
				ret = redFs_node_read(header->current_node, &node);
				if(ret) return 0;
			}else{
				return 0;
			}
		}
	}
	return adr;
}

RED_PTR redFs_get_file(Red_Header* header, char*path){
	RED_PTR current_node = header->current_node;
	if(path[0] == '/') header->current_node = header->root;
	char** cpath = redFs_chop_path(path);
	char* file_name = redFs_path_pop_last(cpath);
	RED_PTR node_adr = 0;
	int ret = redFs_change_path_already_chopped(header, cpath);
	if(ret) return ret;
	node_adr = redFs_get_file_from_current_folder(header, file_name);
	if(node_adr == 0) return FILE_NOT_FOUND_ERROR;
	header->current_node = current_node;
	return node_adr;
}

int __redFs_recursive_remove_file(Red_Header* header, RED_PTR ptr, Red_File* f){
	int ret = redFs_node_read(ptr, (Red_Node*)f);
	if(ret) return ret;
	if(f->chained && f->next_page != 0){
		ret =  __redFs_recursive_remove_file(header, f->next_page, f);
		if(ret) return ret;
	}
	ret = redFs_node_dealloc(header, ptr);
	if(ret) return ret;
	return 0;
}

int redFs_remove_file_in_current_location(Red_Header* header, char*name){
	RED_PTR ptr = redFs_get_file_from_current_folder(header, name);
	if(ptr == 0){
		return FILE_NOT_FOUND_ERROR;
	}
	static Red_File f = {0};
	int ret = __redFs_recursive_remove_file(header, ptr, &f);
	if(ret){
		redFs_strerror(ret);
		return FILE_DEALLOCATION_ERROR;
	}
	return redFs_node_pop_child_node_with_ptr(header, ptr, header->current_node);
}

int redFs_remove_file(Red_Header* header, char* path){
	RED_PTR current_node = header->current_node;
	if(path[0] == '/') header->current_node = header->root;
	char** cpath = redFs_chop_path(path);
	char* file_name = redFs_path_pop_last(cpath);
	int ret = redFs_change_path_already_chopped(header, cpath);
	if(ret) return ret;
	ret = redFs_remove_file_in_current_location(header, file_name);
	if(ret) return ret;
	header->current_node = current_node;
	return 0;

}

int redFs_write_file_in_current_location(Red_Header* header, char*name, uint8_t* buffer, uint32_t size){
	RED_PTR file_ptr = redFs_get_file_from_current_folder(header, name);
	int ret = 0;
	if(file_ptr == 0){
		ret = redFs_node_create_child_node(header, name, PAGE_DEF_PERMISSION, PAGE_IS_FILE, header->current_node);
		if(ret) return ret;
		file_ptr = redFs_get_file_from_current_folder(header, name);
	}
	uint32_t required_space = (size/(NODE_FILE_LIMIT))+1;
	uint32_t tracker = 0;
	uint32_t current_ptr = file_ptr;
	static Red_File f = {0};
	for(uint32_t i=0;i<required_space; i++){
		ret = redFs_node_read(current_ptr, (Red_Node*)&f);
		f.file_size = size;
		for(uint32_t j=0;j<NODE_FILE_LIMIT;j++){
			if(tracker < size){
				f.content[j] = buffer[tracker];
				tracker += 1;
			}
		}
		if(f.chained){
			if(f.next_page != 0){
				ret = redFs_node_write(file_ptr, (Red_Node*)&f);
				if(ret)	return ret;
				current_ptr = f.next_page;
			}else{
				return FILE_POINTER_ERROR;
			}
		}else{
			RED_PTR c_adr = redFs_node_alloc(header, CHAINED_NAME, f.permissions, PAGE_IS_FILE);
			if(c_adr == 0) return FILE_ALLOCATION_ERROR;
			f.next_page = c_adr;
			f.chained = true;
			ret = redFs_node_write(current_ptr, (Red_Node*)&f);
			if(ret)	return ret;
			current_ptr = c_adr;
			ret = redFs_cache_update(header);
			if(ret) return ret;
		}
	}
	ret = redFs_cache_update(header);
	if(ret) return ret;

	if(f.chained){
		if(f.next_page != 0){
			ret =  __redFs_recursive_remove_file(header, f.next_page, &f);
			if(ret) return ret;
			f.next_page = 0;
			f.chained = false;
			ret = redFs_node_write(current_ptr, (Red_Node*)&f);
			if(ret) return ret;
		}
	}
	return 0;
}

int redFs_write_file(Red_Header* header, char*path, uint8_t* buffer, uint32_t size){
	RED_PTR current_node = header->current_node;
	if(path[0] == '/') header->current_node = header->root;
	char** cpath = redFs_chop_path(path);
	char* file_name = redFs_path_pop_last(cpath);
	int ret = redFs_change_path_already_chopped(header, cpath);
	if(ret) return ret;
	ret = redFs_write_file_in_current_location(header, file_name, buffer, size);
	if(ret) return ret;
	header->current_node = current_node;
	return 0;
}

int redFs_read_file_in_current_location(Red_Header* header, char*name, uint8_t* buffer, uint32_t size){
	RED_PTR file_ptr = redFs_get_file_from_current_folder(header, name);
	int ret = 0;
	if(file_ptr == 0){
		return FILE_NOT_FOUND_ERROR;
	}
	uint32_t tracker = 0;
	RED_PTR current_ptr = file_ptr;
	while(true){
		Red_File f = {0};
		ret = redFs_node_read(current_ptr, (Red_Node*)&f);
		if(ret) return ret;
		bool end = false;
		for(uint32_t j=0;j<NODE_FILE_LIMIT && !end;j++){
			if(tracker < size){
				buffer[tracker] = f.content[j];
				tracker += 1;
			}else{
				end = true;
			}
		}
		if(f.chained && !end){
			if(f.next_page != 0){
				current_ptr = f.next_page;
			}else{
				return FILE_POINTER_ERROR;
			}
		}else{
			return FILE_TOO_SMALL_ERROR;
		}
	}
	return 0;
}

int redFs_read_file(Red_Header* header, char*path, uint8_t* buffer, uint32_t size){
	RED_PTR current_node = header->current_node;
	if(path[0] == '/') header->current_node = header->root;
	char** cpath = redFs_chop_path(path);
	char* file_name = redFs_path_pop_last(cpath);
	int ret = redFs_change_path_already_chopped(header, cpath);
	if(ret) return ret;
	ret = redFs_read_file_in_current_location(header, file_name, buffer, size);
	if(ret) return ret;
	header->current_node = current_node;
	return 0;
}

int redFs_get_current_file_size(Red_Header* header, char*name){
	RED_PTR file_ptr = redFs_get_file_from_current_folder(header, name);
	if(file_ptr == 0){
		return -1;
	}
	Red_File f = {0};
	if(redFs_node_read(file_ptr, (Red_Node*)&f)) return -1;
	return f.file_size;
}

int redFs_get_file_size(Red_Header* header, char* path){
	RED_PTR current_node = header->current_node;
	if(path[0] == '/') header->current_node = header->root;
	char** cpath = redFs_chop_path(path);
	char* file_name = redFs_path_pop_last(cpath);
	int ret = redFs_change_path_already_chopped(header, cpath);
	if(ret) return ret;
	int size = redFs_get_current_file_size(header, file_name);
	header->current_node = current_node;
	return size;
}
