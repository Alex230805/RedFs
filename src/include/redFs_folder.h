#ifndef REDFS_FOLDER_H
#define REDFS_FOLDER_H

#include "redFs.h"
#include "redFs_file.h"

/*
 *	redFs_folder.c and redFs_folder.h provide the abstraction layer on top of redFs_node to 
 *	interact with folders. Everything related to creating folders, deleting folders, navigating 
 *	the filesystem and so on are implemented here. 
 *
 *	Each partition header has an entry point address and the current node address which keep track 
 *	of the entire filesystem root file and the current opened folder inside the filesystem. 
 *	By having those two addresses redFs_folder can implement a simplified tree navigation system that 
 *	can work directly with path like "/this/is/a/folder" or "./current/folder" or "test/folder". 
 *
 */

/*
 *	redFs_local_buffer and related variable/macro are used as a static circular buffer inside redFs_folder 
 *	to momentarily write and read string literals used by the printing functions. 
 *	If needed it's possible to increase this size by scaling up BUFFER_SIZE.
 *
 */

#define BUFFER_SIZE 1024*16
static char redFs_local_buffer[BUFFER_SIZE];
static int redFs_local_buffer_tracker = 0;

/* ======================================================================================================== */

/*  RedFS Folder API functions */

/* 
 *	This is a wrapper for redFs_change_directory. It allow a simplified tree navigation system that accept 
 *	absolute path and/or relative paths. 
 *	It return 0 if no error occurred, otherwise it return an error code compatible with redFs_strerror.
 *
 */

REDAPI int redFs_change_path(Red_Header*header, char* path);

/*
 *	It return the name of the current directory pointed by the current_node address of Red_Header*.
 *	The name is allocated inside the static buffer of redFs, if needed it's suggested to duplicate this 
 *	string inside another location.
 *
 */

REDAPI char* redFs_get_current_dir_name(Red_Header* header);

/*
 *	Node fetch function to get a clone of the current_node folder. 
 *	Upon successful operation it return 0, otherwise an error code is returned. 
 *
 */

REDAPI int redFs_get_current_directory(Red_Header* header, Red_Node* node);

/*
 *	Embedded functions to get a simplified output of the content inside the 
 *	current directory.
 *	Upon succesful operation it return 0, otherwise an error code is returned. 
 *
 */

REDAPI int redFs_print_current_dir_content(Red_Header* header);

/*
 *	Wrapper of redFs_print_current_dir_content(Red_Header* header). It can take 
 *	a complete path and it will navigate to the specified folder, printing it 
 *	to the standard output. 
 *	Upon succesful operation it return 0, otherwise an error code is returned. 
 *
 */

REDAPI int redFs_print_dir_content(Red_Header* header, char* path);

/*
 *	Similar to redFs_print_current_dir_content, it will dive inside the 
 *	current folder to fetch the content's name and return it as an array 
 *	or strings allocated in the redFs circular buffer. 
 *
 *	It's not necessary to perform a deallocation after fetching and using 
 *	the data. It's suggested to strdup the content if you need a persistent 
 *	information about the current content for more than an immediate usage. 
 *
 */

REDAPI char** redFs_get_current_dir_content(Red_Header* header);

/*
 *	Wrapper of redFs_get_current_dir_content, by providing a path it will fetch the 
 *	folder and collect the content's name, it will return an array of strings allocated 
 *	inside the internal redFs circular buffer, there is no need to deallocate the 
 *	array after utilize this functions. 
 *
 *	It's suggested to use this function where the result is immediately used and not stored. 
 *	If there's a need to store the result of this function it's suggested to take the 
 *	output and strdup-it. 
 *
 */

REDAPI char** redFs_get_dir_content(Red_Header* header, char* path);

/*
 *	Create directory operate on relative and absolute path. It can perform tree 
 *	navigation and create the directory where needed.
 *	Upon succesful operation it return 0, otherwise an error code is returned. 
 *
 */

REDAPI int redFs_create_directory(Red_Header* header, char* full_path, int permissions);

/*
 *	Function that return the amount of elements of the current folder by providing the 
 *	partition header and the pointer where the value will be stored. 
 *
 *	Upon succesfull operation a zero exit code will be returned, otherwise a non zero exit code 
 *	compatible with redFs_strerror will be returned.
 */

REDAPI int redFs_get_current_dir_content_count(Red_Header* header, uint32_t* dest_count);

/*
 *	Based on redFs_get_current_dir_content_count, it accept a path and a variable pointer where 
 *	the count of the elements will be stored. 
 *
 *	If an error occur it will return a non-zero exit code, otherwise a zero exit code will be returned 
 *	compatible with redFs_strerror.
 */

REDAPI int redFs_get_dir_content_count(Red_Header* header, char* path, uint32_t* dest_count);

/*
 *	Complement of redFs_create_directory(). It provide a simple function to delete 
 *	a directory given a specified path that point to it. It can also perform tree navigation 
 *	like redFs_create_directory()
 *	Upon succesful operation it return 0, otherwise an error code is returned. 
 *
 */

REDAPI int redFs_remove_directory(Red_Header* header, char*full_path);

/* ======================================================================================================== */

/*
 *	Internal filesystem functions used by redFs.  
 *	Use it with caution. It's suggested to read the source code for each one of them before proceed 
 *	using it.
 */

char* redFs_malloc(size_t size);
char** redFs_chop_path(char* path);
char* redFs_path_pop_last(char** chopped_path);
uint32_t redFs_get_path_dir_count(char** chopped_path);
int redFs_change_path_already_chopped(Red_Header* header, char** chopped_path);
int redFs_change_directory(Red_Header* header, char* dir_name); // '.' and '..' are supported


#ifndef REDFS_FOLDER_IMP
#define REDFS_FOLDER_IMP

#endif
#endif // REDFS_FOLDER_H
