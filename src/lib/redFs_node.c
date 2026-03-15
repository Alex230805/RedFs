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
	
	// updating caching counter
	header->cache_timing += 1;
	if(header->cache_timing > header->cache_limit){
		 ret = redFs_sync_partition(header);
	}
	
	return node_adr;
}

int redFs_node_create_child_node(Red_Header* header, char* name, uint8_t permissions, uint8_t type, RED_PTR father_node){
	int ret = 0;
	RED_PTR ptr = redFs_node_alloc(header, name, permissions, type);
	if(ptr == 0) return NODE_ALLOCATION_ERROR;
	Red_Node n = {0};
	ret = redFs_node_read(ptr, &n);
	if(ret) return ret;
	n.f_node = father_node;
	ret = redFs_node_write(ptr, &n);
	if(ret) return ret;

	ret = redFs_node_update_content_list(header, father_node, ptr);
	
	header->cache_timing += 1;
	if(header->cache_timing > header->cache_limit){
		 ret = redFs_sync_partition(header);
	}
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
