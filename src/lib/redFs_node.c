#define  REDFS_NODE_IMP

#include "redFs_node.h"

int redFs_node_writedown(Red_Node* node, RED_PTR address){
	return 0;
}

int redFs_node_grep(RED_PTR address, Red_Node* node){
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

int redFs_node_revert_latest(RED_PTR address){
	return 0;
}

int redFs_node_append_content(RED_PTR address, RED_PTR content_adr){
	return 0;
}

int redFs_node_alloc(Red_Header* header, char* name, uint8_t permissions){
	return 0;
}

void redFs_node_debug_show_content(RED_PTR address){
}
