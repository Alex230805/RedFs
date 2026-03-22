#define  REDFS_NODE_IMP

#include "redFs_node.h"


int redFs_node_write(RED_PTR address, Red_Node* node){
	for(uint32_t i=0;i<sizeof(Red_Node);i++){
		if(redFs_disk_action_write(address+i, *((uint8_t*)node+i))){
			return 	PARTITION_NODE_WRITING_ERROR;
		}
	}
	return 0;
}

int redFs_node_read(RED_PTR address, Red_Node* node){
	for(uint32_t i=0;i<sizeof(Red_Node);i++){
		if(redFs_disk_action_read(address+i, (uint8_t*)node+i)){
			return 	PARTITION_NODE_READING_ERROR;
		}
	}
	return 0;
}

int redFs_node_update_content_list(Red_Header* header, RED_PTR current_node, RED_PTR node_adr){
	int ret = 0;
	Red_Node node = {0};
	ret = redFs_node_read(current_node, &node);
	if(ret) return ret;

	if(node.content_count < (NODE_ARRAY_LIMIT/sizeof(RED_PTR))){
		node.content[node.content_count] = node_adr;
		node.content_count += 1;
		ret = redFs_node_write(current_node, &node);	
	}else{
		if(node.chained){
			ret = redFs_node_update_content_list(header, node.next_page, node_adr);
		}else{

			// allocating new chain node and writing the prev_page address
			RED_PTR ptr = redFs_node_alloc(header, CHAINED_NAME, node.permissions,PAGE_IS_CHAIN | node.type);
			if(ptr == 0) return NODE_ALLOCATION_ERROR;
			Red_Node c_node = {0};
			ret = redFs_node_read(ptr, &c_node);
			if(ret) return ret;
			c_node.prev_page = current_node;
			ret = redFs_node_write(ptr, &c_node);
			if(ret) return ret;

			// updating the current node chained flag, writing down the next_page address and sync it back to the drive
			node.chained = true;
			node.next_page = ptr;
			ret = redFs_node_write(current_node, &node);	
			if(ret) return ret;

			ret = redFs_node_update_content_list(header, node.next_page, node_adr);

		}
	}
	return 0;
}


RED_PTR redFs_node_alloc(Red_Header* header, char* name, uint8_t permissions, uint8_t type){
	int ret = 0;
	static Red_Node node = {0};
	node.type = type;
	memcpy(node.name, name, sizeof(char)*STRING_LIMIT);
	node.name[STRING_LIMIT-1] = '\0';
	node.permissions = permissions;
	node.chained = false;

	// searching for an active block
	uint32_t i=0;
	while((header->fstab.block_state[i] == FULL_BLOCK ||  header->fstab.block_state[i] == RESERVED_BLOCK) && i < header->fstab.block_limit){i+=1;}
	int frag = redFs_get_free_fragment_offset(header->fstab.raw_block_ptr[i].fragment_map);
	if(frag == -1) {
		redFs_strerror(REDFS_BLOCK_FRAGMENT_ERROR);
		redFs_strerror(NOT_ENOUGH_DISK_SPACE_ERROR);
		return 0;
	}

	// calculating node offset and sync node
	RED_PTR node_adr = header->fstab.raw_block_ptr[i].base_ptr + frag*sizeof(Red_Node);
	ret = redFs_node_write(node_adr, &node);
	if(ret){
		redFs_strerror(ret);
		return 0;
	}

	// updating block parameters
	header->fstab.raw_block_ptr[i].fragment_map = header->fstab.raw_block_ptr[i].fragment_map | (1 << frag);
	header->fstab.raw_block_ptr[i].node_count+=1;
	if(header->fstab.raw_block_ptr[i].node_count >=32 && header->fstab.raw_block_ptr[i].fragment_map == 0xFFFFFFFF){
		header->fstab.block_state[i] = FULL_BLOCK;
	}
	
	ret = redFs_cache_update(header);
	if(ret) return 0;
	
	return node_adr;
}

int redFs_node_dealloc(Red_Header* header, RED_PTR ptr){
	uint32_t block_tracker = 0;
	Red_Node node = {0};
	int ret = redFs_node_read(ptr, &node);
	if(ret) return ret;
	if(node.chained){
		ret =  redFs_node_dealloc(header, node.next_page);
		if(ret) return ret;	
	}
	while(block_tracker < header->fstab.block_limit-1 && ( ptr < header->fstab.raw_block_ptr[block_tracker].base_ptr || ptr > header->fstab.raw_block_ptr[block_tracker+1].base_ptr)){
		block_tracker+=1;
	}
	uint32_t frag_offset = (ptr - header->fstab.raw_block_ptr[block_tracker].base_ptr) / sizeof(Red_Node);
	uint32_t new_fragment_map = header->fstab.raw_block_ptr[block_tracker].fragment_map;
	new_fragment_map = new_fragment_map ^ ((1<<frag_offset) & 0xFFFFFFFF);
	header->fstab.raw_block_ptr[block_tracker].fragment_map = new_fragment_map;
	header->fstab.raw_block_ptr[block_tracker].node_count -= 1;
	if(header->fstab.raw_block_ptr[block_tracker].node_count == 0){
		header->fstab.block_state[block_tracker] = FREE_BLOCK;
	}else{
		header->fstab.block_state[block_tracker] = ACTIVE_BLOCK;
	}

	ret = redFs_cache_update(header);
	return 0;
}

/* NOTE: this function does not deallocate the child ptr from the father node*/
int redFs_node_pop_child_node_with_ptr(Red_Header* header, RED_PTR child, RED_PTR father_node){
	int ret = 0;
	Red_Node f_node = {0};
	ret = redFs_node_read(father_node, &f_node);
	if(ret) return ret;
	if(f_node.type != PAGE_IS_FOLDER) return (int)NODE_IS_NOT_A_FOLDER_ERROR;
	bool end = false;
	RED_PTR ptr = 0;

	while(!end){
		for(uint32_t i=0;i<f_node.content_count && ptr == 0; i++){
			Red_Node node = {0};
			ret = redFs_node_read(f_node.content[i], &node);
			if(ret) return ret;
			if(child == f_node.content[i]){
				end = true;
				ptr = f_node.content[i];
				f_node.content[i] = 0;
				f_node.content_count -= 1;
				ret = redFs_node_write(father_node, &f_node);
				if(ret) return ret;
			}
		}
		if(ptr == 0){ 
			if(f_node.chained){	
				ret = redFs_node_read(f_node.next_page, &f_node);
				if(ret) return ret;
			}else{
				return (int)NODE_NOT_FOUND;
			}
		}
	}
	ret = redFs_cache_update(header);
	if(ret) return ret;
	return 0;
}


int redFs_node_remove_child_node(Red_Header* header, char*name, RED_PTR father_node){
	int ret = 0;
	Red_Node f_node = {0};
	ret = redFs_node_read(father_node, &f_node);
	if(ret) return ret;
	if(f_node.type != PAGE_IS_FOLDER) return (int)NODE_IS_NOT_A_FOLDER_ERROR;
	bool end = false;
	RED_PTR ptr = 0;

	while(!end){
		for(uint32_t i=0;i<f_node.content_count && ptr == 0; i++){
			Red_Node node = {0};
			ret = redFs_node_read(f_node.content[i], &node);
			if(ret) return ret;
			if(strcmp(node.name, name) == 0){
				end = true;
				ptr = f_node.content[i];
				f_node.content[i] = 0;
				f_node.content_count -= 1;
				ret = redFs_node_write(father_node, &f_node);
				if(ret) return ret;
			}
		}
		if(ptr == 0){ 
			if(f_node.chained){	
				ret = redFs_node_read(f_node.next_page, &f_node);
				if(ret) return ret;
			}else{
				return (int)NODE_NOT_FOUND;
			}
		}
	}
	ret = redFs_node_dealloc(header, ptr);
	if(ret) {
		redFs_strerror(ret);
		return NODE_DEALLOCATION_ERROR;
	}
	
	ret = redFs_cache_update(header);
	if(ret) return ret;
	return 0;

}


int __redFs_node_recursive_remove(Red_Header* header, RED_PTR father_node){
	int ret = 0;
	Red_Node node = {0};
	ret = redFs_node_read(father_node, &node);
	if(ret) return ret;
	if(node.type != PAGE_IS_FOLDER){
		return -1;
	}else{
		// if(node.type == PAGE_IS_FOLDER){
		for(uint32_t i=0; i < node.content_count; i++){
			RED_PTR c = node.content[i];
			int status = __redFs_node_recursive_remove(header, c);
			if(status > 0){
				return NODE_RECURSIVE_DEALLOCATION_ERROR;
			}
			ret = redFs_node_dealloc(header, c);
			header->cache_timing += 1;
			if(header->cache_timing > header->cache_limit){
				 ret = redFs_sync_partition(header);
			}
			if(ret) return NODE_DEALLOCATION_ERROR;
		}
		ret = redFs_node_dealloc(header, father_node);
		ret = redFs_cache_update(header);
		if(ret) return ret;
		// }
	}
	return 0;
}

int redFs_node_recursive_remove_child_node(Red_Header* header, char* name, RED_PTR father_node){
	int ret = 0;
	Red_Node f_node = {0};
	ret = redFs_node_read(father_node, &f_node);
	if(ret) return ret;
	if(f_node.type != PAGE_IS_FOLDER) return (int)NODE_IS_NOT_A_FOLDER_ERROR;
	bool end = false;
	RED_PTR ptr = 0;

	while(!end){
		for(uint32_t i=0;i<f_node.content_count && ptr == 0; i++){
			Red_Node node = {0};
			ret = redFs_node_read(f_node.content[i], &node);
			if(ret) return ret;
			if(strcmp(node.name, name) == 0){
				end = true;
				ptr = f_node.content[i];
				f_node.content[i] = 0;
				f_node.content_count -= 1;
				ret = redFs_node_write(father_node, &f_node);
				if(ret) return ret;
			}
		}
		if(ptr == 0){ 
			if(f_node.chained){	
				ret = redFs_node_read(f_node.next_page, &f_node);
				if(ret) return ret;
			}else{
				return (int)NODE_NOT_FOUND;
			}
		}
	}

	ret = __redFs_node_recursive_remove(header, ptr);
	if(ret) return ret;
	ret = redFs_cache_update(header);
	if(ret) return ret;

	return 0;
}

int redFs_node_create_child_node(Red_Header* header, char* name, uint8_t permissions, uint8_t type, RED_PTR father_node){
	int ret = 0;
	
	Red_Node n = {0};
	ret = redFs_node_read(father_node, &n);
	if(ret) return ret;
	if(n.type != PAGE_IS_FOLDER) return (int)NODE_IS_NOT_A_FOLDER_ERROR;

	
	RED_PTR ptr = redFs_node_alloc(header, name, permissions, type);
	if(ptr == 0) return NODE_ALLOCATION_ERROR;
	ret = redFs_node_read(ptr, &n);
	if(ret) return ret;
	n.f_node = father_node;
	ret = redFs_node_write(ptr, &n);
	if(ret) return ret;

	ret = redFs_node_update_content_list(header, father_node, ptr);	
	ret = redFs_cache_update(header);
	return ret;
}

void redFs_node_show(RED_PTR address){
	Red_Node node = {0};
	int ret = redFs_node_read(address, &node);
	if(ret){
		redFs_strerror(ret);
		return;
	}
	redFs_node_debug_show_content(&node);
}


void redFs_node_debug_show_content_array(Red_Node* node, int c_base){
	uint32_t i=0;
	for(i=0;i<node->content_count;i++){
		printf("-> content [%d]: 0x%x\n", i+c_base, node->content[i]);
	}
	if(node->chained){
		Red_Node n = {0};
		int ret = redFs_node_read(node->next_page, &n);
		if(ret){
			redFs_strerror(PARTITION_NODE_READING_ERROR);
			return;
		}
		redFs_node_debug_show_content_array(&n,i+c_base);
	}
}

int redFs_node_get_content_count(Red_Node* node){
	int content_count = 0;
	bool end = false;
	while(node->chained && !end){
		Red_Node n = {0};
		int ret = redFs_node_read(node->next_page, &n);
		if(ret){
			redFs_strerror(PARTITION_NODE_READING_ERROR);
		}
		content_count += redFs_node_get_content_count(&n);
		end = true;
	}
	content_count += node->content_count;
	return content_count;
}


void redFs_node_debug_show_content(Red_Node* node){
	int content_count = redFs_node_get_content_count(node);
	printf("Node name: %s\n", node->name);
	printf("Father node: 0x%x\n", node->f_node);
	printf("Permissions: %d\n", node->permissions);
	printf("Content count: %d\n", content_count);
}

