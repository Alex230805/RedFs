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

bool redFs_node_has_next(RED_PTR address);
bool redFs_node_is_file(RED_PTR address);
bool redFs_node_is_folder(RED_PTR address);


void redFs_node_show_content(RED_PTR address);
void redFs_node_debug_show_content(Red_Node* node);
/*
 * 
 * Node partition internal functions. 
 *
 * */

int redFs_node_alloc(Red_Header* header, char* name, uint8_t permissions, uint8_t type);


#ifndef REDFS_NODE_IMP
#define REDFS_NODE_IMP


#endif //REDFS_NODE_IMP
#endif // REDFS_NODE_H
