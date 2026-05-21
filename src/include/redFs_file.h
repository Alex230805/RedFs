#ifndef REDFS_FILE_H
#define REDFS_FILE_H

#include "redFs.h"

/* ======================================================================================================== */

/*  RedFS Folder API functions */

/*
 *	This function can create a node of type FILE in the specified path. It accept file permissions, 
 *	the path + the name of the file, and the Red_Header* struct pointer as argument. 
 *	This action has as a side effect the increment of the cache timing variable inside Red_Header, 
 *	this must be kept in mind since the file creation and file write action are two separate process 
 *	in redFs and both will touch the cache timing variable.
 *
 *	This function upon any type of error it will return an error code compatible with redFs_strerror, or 
 *	otherwise it will return 0 on successful completion.
 *
 *	NOTE: the path argument must be a compatible path format ( "/this/folder", "./here", "this/folder".. )
 *	used also by the folder implementation of redFs, and it must include as a destination the file name, as 
 *	an example "/position/of/new/file.txt" will create a empty node called "file.txt" inside "/position/of/new/".
 */

REDAPI int redFs_touch_file(Red_Header* header, char* path, uint8_t permissions);

/*
 *	Complement of redFs_touch_file, it accept the path indicating the exact position of the file 
 *	and it will remove it. It accept Red_Header* to update the cached fstab, this means that it 
 *	will perform a side effect, increasing the cache timing variable.
 *
 *	Upon successful completion it will return 0, otherwise it return an error compatible with 
 *	redFs_strerror.
 */

REDAPI int redFs_remove_file(Red_Header* header, char* path);

/*
 *	This function can wrine an **already existing** file with a new content. It accept the 
 *	path indicating the exact position of the destination file for the write operation, an 
 *	array of bytes which is the content to be written on the selected file and the size of the 
 *	array. 
 *
 *	Upon successful completion it will return 0, otherwise it return an error compatible with 
 *	redFs_strerror.
 *
 *	NOTE: 
 *
 *	This function cannot perform a "touch" operation, it can only write inside an alreay 
 *	existing node of type FILE, it means that before writing down a new file it's necessary 
 *	to perform a redFs_touch_file() operation.
 *
 */

REDAPI int redFs_write_file(Red_Header* header, char*path, uint8_t* buffer, uint32_t size);

/*
 *	Complement of redFs_write_file, it can read a certain amount of bytes from the file 
 *	pointed by the path argument; it accept a pointer to a destination buffer that will be 
 *	used to write the content fetched from the file. If the specified size exeed the maximum 
 *	file size, then only the maximum amount of content will be returned to the caller, breaking 
 *	the reading loop.
 *
 *	Upon successful completion it will return 0, otherwise it return an error compatible with 
 *	redFs_strerror.
 *
 *	NOTE:
 *
 *	This function cannot read from unexisting files and cannot perform a touch operation, it can 
 *	only read from an existing node of type FILE.
 *
 */

REDAPI int redFs_read_file(Red_Header* header, char*path, uint8_t* buffer, uint32_t size);

/*
 *	This function return the size of a specified file. If the file exist and no error are encounter 
 *	then the file size will be returned, otherwise an error compatible with redFs_strerror will be returned.
 */

REDAPI int redFs_get_file_size(Red_Header* header, char* path);

/* ======================================================================================================== */

/*
 *	Internal filesystem functions used by redFs.  
 *	Use it with caution. It's suggested to read the source code for each one of them before proceed 
 *	using it.
 */

RED_PTR redFs_get_file(Red_Header* header, char*path);
int redFs_touch_file_in_current_location(Red_Header* header, char* name, uint8_t permissions);
int redFs_write_file_in_current_location(Red_Header* header, char*name, uint8_t* buffer, uint32_t size);
int redFs_read_file_in_current_location(Red_Header* header, char*name, uint8_t* buffer, uint32_t size);
RED_PTR redFs_get_file_from_current_folder(Red_Header* header, char*name);
int redFs_get_current_file_size(Red_Header* header, char*name);
int redFs_remove_file_in_current_location(Red_Header* header, char*name);


#ifndef REDFS_FILE_IMP
#define REDFS_FILE_IMP

#endif
#endif// REDFS_FILE_H
