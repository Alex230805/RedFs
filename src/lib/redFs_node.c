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

bool redFs_node_is_folder(RED_PTR address){
	return false;
}

bool redFs_node_is_file(RED_PTR address){
	return false;	
}

bool redFs_node_has_next(RED_PTR address){
	return false;
}

int redFs_node_alloc(Red_Header* header, char* name, uint8_t permissions, uint8_t type){
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
	uint32_t frag = redFs_get_free_fragment_offset(header->fstab.raw_block_ptr[i].fragment_map);
	if(frag == 0) return 1;

	// calculating node offset and sync node
	RED_PTR node_adr = header->fstab.raw_block_ptr[i].base_ptr + frag*sizeof(Red_Node);
	ret = redFs_node_write(node_adr, &node);
	if(ret) return ret;

	// updating block parameters
	header->fstab.raw_block_ptr[i].fragment_map = header->fstab.raw_block_ptr[i].fragment_map | (1 << frag);
	header->fstab.raw_block_ptr[i].node_count+=1;
	if(header->fstab.raw_block_ptr[i].node_count >=32 && header->fstab.raw_block_ptr[i].fragment_map == 0xFFFFFFFF){
		header->fstab.block_state[i] = FULL_BLOCK;
	}
	
	// grep father node,u pdating content list and sync it back
	ret = redFs_node_read(header->current_node, &node);
	if(ret) return ret;

	if(node.content_count < (NODE_ARRAY_LIMIT/sizeof(RED_PTR))){
		node.content[node.content_count] = node_adr;
		node.content_count += 1;
		ret = redFs_node_write(header->current_node, &node);
		if(ret) return ret;
	}else{
		printf("TODO: node concat\n");
		exit(1);
	}

	// updating caching counter
	header->cache_timing += 1;
	if(header->cache_timing > header->cache_limit){
		 ret = redFs_sync_partition(header);
	}

	return ret;
}

void redFs_node_show_content(RED_PTR address){
	Red_Node node = {0};
	int ret = redFs_node_read(address, &node);
	if(ret){
		redFs_strerror(ret);
		return;
	}
	redFs_node_debug_show_content(&node);
}

void redFs_node_debug_show_content(Red_Node* node){
	printf("Node name: %s\n", node->name);
	printf("Father node: 0x%x\n", node->f_node);
	printf("Permissions: %d\n", node->permissions);
	printf("Content count: %d\n", node->content_count);
	for(uint32_t i=0;i<node->content_count;i++){
		printf("-> content [%d]: 0x%x\n", i, node->content[i]);
	}
}
