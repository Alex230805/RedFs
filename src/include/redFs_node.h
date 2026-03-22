#ifndef REDFS_NODE_H
#define REDFS_NODE_H

#include "redFs.h"
#include "redFs_io.h"


/*
 * redFs node manager. Here the standard node-related functions are 
 * defined while more specific action such folder or file action will 
 * be handled directly by the relative C file. 
 *
 * */

int  redFs_node_write(RED_PTR address, Red_Node* node);
int  redFs_node_read(RED_PTR address, Red_Node* node);

int redFs_node_update_content_list(Red_Header* header, RED_PTR current_node, RED_PTR node_adr);
RED_PTR redFs_node_alloc(Red_Header* header, char* name, uint8_t permissions, uint8_t type);
int redFs_node_dealloc(Red_Header* header, RED_PTR ptr);

void redFs_node_show(RED_PTR address);
void redFs_node_debug_show_content(Red_Node* node);
int redFs_node_get_content_count(Red_Node* node);
void redFs_node_debug_show_content_array(Red_Node* node, int c_base);

/* NOTE: this function does not deallocate the child ptr from the father node*/
int redFs_node_pop_child_node_with_ptr(Red_Header* header, RED_PTR child, RED_PTR father_node);

int redFs_node_create_child_node(Red_Header* header, char* name, uint8_t permissions, uint8_t type, RED_PTR father_node);
int redFs_node_remove_child_node(Red_Header* header, char*name, RED_PTR father_node);


#ifndef REDFS_NODE_IMP
#define REDFS_NODE_IMP


#endif //REDFS_NODE_IMP
#endif // REDFS_NODE_H
