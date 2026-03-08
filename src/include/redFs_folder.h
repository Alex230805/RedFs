#ifndef REDFS_FOLDER_H
#define REDFS_FOLDER_H


Red_Node redFs_get_root_folder(uint8_t partition_number);
void redFs_change_directory(Red_Node current_node, char* dir_name); // '.' and '..' are supported
void redFs_create_directory(Red_Node current_node, char* name);

#ifndef REDFS_FOLDER_IMP
#define REDFS_FOLDER_IMP

#endif

#endif // REDFS_FOLDER_H
