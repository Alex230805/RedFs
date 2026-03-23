#ifndef REDFS_FILE_H
#define REDFS_FILE_H

#include "redFs.h"


int redFs_touch_file_in_current_location(Red_Header* header, char* name, uint8_t permissions);
int redFs_write_file_in_current_location(Red_Header* header, char*name, uint8_t* buffer, uint32_t size);
int redFs_read_file_in_current_location(Red_Header* header, char*name, uint8_t* buffer, uint32_t size);
RED_PTR redFs_get_file_from_current_folder(Red_Header* header, char*name);
int redFs_get_current_file_size(Red_Header* header, char*name);
int redFs_remove_file_in_current_location(Red_Header* header, char*name);

int redFs_touch_file(Red_Header* header, char* path, uint8_t permissions);
RED_PTR redFs_get_file(Red_Header* header, char*path);
int redFs_remove_file(Red_Header* header, char* path);
int redFs_write_file(Red_Header* header, char*path, uint8_t* buffer, uint32_t size);
int redFs_read_file(Red_Header* header, char*path, uint8_t* buffer, uint32_t size);
int redFs_get_file_size(Red_Header* header, char* path);

#ifndef REDFS_FILE_IMP
#define REDFS_FILE_IMP


#endif
#endif// REDFS_FILE_H
