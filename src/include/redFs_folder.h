#ifndef REDFS_FOLDER_H
#define REDFS_FOLDER_H

#include "redFs.h"
#define BUFFER_SIZE 4096*2

static char redFs_local_buffer[BUFFER_SIZE];
static int redFs_local_buffer_tracker = 0;

char* redFs_malloc(size_t size);
char** redFs_chop_path(char* path);
char* redFs_path_pop_last(char** chopped_path);
uint32_t redFs_get_path_dir_count(char** chopped_path);

int redFs_create_directory(Red_Header* header, char* full_path, int permissions);
int redFs_remove_directory(Red_Header* header, char*full_path);
int redFs_get_current_directory(Red_Header* header, Red_Node* node);
int redFs_change_directory(Red_Header* header, char* dir_name); // '.' and '..' are supported

char* redFs_get_current_dir_name(Red_Header* header);
int redFs_get_current_dir_content(Red_Header* header);
int redFs_get_dir_content(Red_Header* header, char* path);

/* wrapper for redFs_change_directory to allow a complete tree navigation*/
int redFs_change_path(Red_Header*header, char* path);


#ifndef REDFS_FOLDER_IMP
#define REDFS_FOLDER_IMP

#endif

#endif // REDFS_FOLDER_H
