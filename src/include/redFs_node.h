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

#define STR_BUFFER_LEN 16
#define WORD_BUFFER_LEN 16


#define NODE_CONTENT_OFFSET ((sizeof(uint8_t)*2)+(sizeof(uint32_t))+(sizeof(char)*STRING_LIMIT)+(sizeof(bool))+(sizeof(RED_PTR)*2))
#define NODE_CONTENT_COUNT_OFFSET ((sizeof(uint8_t)*2)+(sizeof(char)*STRING_LIMIT)+sizeof(bool))
#define NODE_CHAINED_OFFSET ((sizeof(uint8_t)*2)+(sizeof(char)*STRING_LIMIT))
#define NODE_NEXT_PAGE_ADDRESS_OFFSET ((sizeof(uint8_t)*2)+(sizeof(uint32_t))+(sizeof(char)*STRING_LIMIT)+(sizeof(bool))+(sizeof(RED_PTR)))


int redFs_node_append_content(RED_PTR address, RED_PTR content_adr);
int redFs_node_revert_latest(RED_PTR address);
int redFs_node_writedown(Red_Node* node, RED_PTR address);
int redFs_node_grep(RED_PTR address, Red_Node* node);
bool redFs_node_has_next(RED_PTR address);
bool redFs_node_is_file(RED_PTR address);
bool redFs_node_is_folder(RED_PTR address);

void redFs_node_debug_show_content(RED_PTR address);
/*
 * 
 * Node partition internal functions. 
 *
 * */

int redFs_node_alloc(Red_Header* header, char* name, uint8_t permissions);


#ifndef REDFS_NODE_IMP
#define REDFS_NODE_IMP


#endif //REDFS_NODE_IMP
#endif // REDFS_NODE_H
