#ifndef REDFS_H
#define REDFS_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>


/* ======================================================================================================== */
/*
 *
 *		 /$$$$$$$                  /$$ /$$$$$$$$ /$$$$$$ 
 *		| $$__  $$                | $$| $$_____//$$__  $$
 *		| $$  \ $$  /$$$$$$   /$$$$$$$| $$     | $$  \__/
 *		| $$$$$$$/ /$$__  $$ /$$__  $$| $$$$$  |  $$$$$$ 
 *		| $$__  $$| $$$$$$$$| $$  | $$| $$__/   \____  $$
 *		| $$  \ $$| $$_____/| $$  | $$| $$      /$$  \ $$
 *		| $$  | $$|  $$$$$$$|  $$$$$$$| $$     |  $$$$$$/
 *		|__/  |__/ \_______/ \_______/|__/      \______/ 
 *                                                 
 *	a simple filesystem implementation for low power and simple systems
 *
 *	This is a filesystem built with the intention to be used on small devices like microcomputer 
 *	or microcontroller, or generally in a limited computational environment. 
 *	To do so the redFs embed different functions for not only managing the disk but also for managing 
 *	partition tables, boot sector and so on. 
 *	Due to the fact that it's desinged for small and simple system the structure for the disk partitioning 
 *	and how it's built could be different from the modern way of doing things, but be compatible it's not 
 *	one of redFs requirement, it's designed to be as simple as possible without wasting resources while still 
 *	providing an easy-to-understand structure with accomodating functions to interact with the filesystem.
 *
 *	The boot sector is 512 byte wide.
 *	
 *	Max addressable space ( 32bit pointer limit ): 4gb
 *	
 *	For more informations check out the informations on top of function declaration inside headers file.
 */
/* ======================================================================================================== */

#define REDAPI					// search for REDAPI for a quick reference to the library's API functions

#define REDFS_ID_PREFIX			94694209
#define REDFS_ID				('R'<<24 | 'D'<<16 | 'F'<<8 | 'S')
#define REDFS_SUFFIX			96499042

#define REDFS_VERSION			0x010000 /* redfs version, from least significant byte ( patch ) going up till major version  */
#define PARTITION_LIMIT			256     /* number of partition that any redFs partition table can handle */
#define BOOT_SECTOR_SIZE		512		/* bytes */
#define PARTITION_BLANK_OFFSET	1024	/* byte offset that separate two partitions */
#define NODE_SIZE				1024	/* size for a single node */
#define CACHE_TIME_LIMIT		1024*20 /* number of WRITE access before the fstab inside the Red_Header is synched on the drive */
#define RED_PTR					uint32_t

/* Enum to define different errors, it's used by redFs_strerror() to check and print the associated message */

typedef enum{
	NOERROR = 0,
	PARTITION_TABLE_FORMAT_ERROR,
	BOOT_SECTOR_WRITING_ERROR,
	PARTITION_TABLE_WRITE_ERROR,
	PARTITION_TABLE_READ_ERROR,
	PARTITION_NOT_FOUND_ERROR,
	PARTITION_TABLE_EMPTY_ERROR,
	NOT_ENOUGH_DISK_SPACE_ERROR,
	FSTAB_READ_ERROR,
	FSTAB_WRITE_ERROR,
	FSTAB_PAGE_WRITE_ERROR,
	PARTITION_FORMAT_DISK_ERROR,
	PARTITION_SIZE_NOT_SUFFICIENT_ERROR,
	PARTITION_ACTION_UNKNOWN_ERROR,
	PARTITION_NODE_WRITING_ERROR,
	PARTITION_NODE_READING_ERROR,
	REDFS_UNSUPPORTED_FUNCTION_ERROR,
	REDFS_BLOCK_FRAGMENT_ERROR,
	REDFS_FRAGMENT_OFFSET_ERROR,
	NODE_ALLOCATION_ERROR,
	NODE_DEALLOCATION_ERROR,
	NODE_NOT_FOUND_ERROR,
	NODE_IS_NOT_A_FOLDER_ERROR,
	NODE_RECURSIVE_DEALLOCATION_ERROR,
	FOLDER_NOT_FOUND_ERROR,
	FILE_ALLOCATION_ERROR,
	FILE_NOT_FOUND_ERROR,
	FILE_POINTER_ERROR,
	FILE_TOO_SMALL_ERROR,
	FILE_DEALLOCATION_ERROR,
	FILE_ALREADY_EXIST_ERROR,
	GENERAL_INVALID_POINTER_ERROR,
	PARTITION_INVALID_ID_ERROR,
	PARTITION_POINTER_LOCATION_MISMATCH_ERROR,
	PARTITION_VERSION_INCOMPATIBLE_ERROR,
	PARTITION_MAGIC_ID_IS_INVALID_ERROR,
	RED_INVALID_ERROR,
	RED_ERROR_LIMIT
}Red_State;

static const char* red_state_lit[RED_ERROR_LIMIT] = {
	[NOERROR]						=	"No error reported during operation",  			
	[PARTITION_TABLE_FORMAT_ERROR]	=	"Unable to format the partition table",  			
	[BOOT_SECTOR_WRITING_ERROR]		=	"Unable to write the boot sector",  			
	[PARTITION_TABLE_WRITE_ERROR]	=	"Partition table writing error, cannot update partition table",  			
	[PARTITION_TABLE_READ_ERROR]	=	"Partition table reading error, cannot get partition table",  			
	[PARTITION_NOT_FOUND_ERROR]		=	"Unable to find the specified partition",  			
	[NOT_ENOUGH_DISK_SPACE_ERROR]	=	"Not enough disk space available",  			
	[FSTAB_READ_ERROR]				=	"Unable to read fstab",  			
	[FSTAB_WRITE_ERROR]				=   "Unable to write fstab",  			
	[FSTAB_PAGE_WRITE_ERROR]		=	"Cannot write partition page due to a write error",  			
	[PARTITION_FORMAT_DISK_ERROR]	=	"Unable to format partition due to a disk error",  			
	[PARTITION_SIZE_NOT_SUFFICIENT_ERROR] =	"The specified partition size is not sufficient to store even the fstab",  			
	[PARTITION_ACTION_UNKNOWN_ERROR]		=	"The required action could not be performed since it doesn't exist or it's still under development",  			
	[PARTITION_NODE_WRITING_ERROR]	=   "Unable to allocate new node for this partition",  			
	[PARTITION_NODE_READING_ERROR]	=	"Unable to read node or node content from this partition",  			
	[REDFS_UNSUPPORTED_FUNCTION_ERROR]	=	"Function not supported",  			
	[REDFS_BLOCK_FRAGMENT_ERROR]	=	"Cannot read the block fragment map",  			
	[NODE_ALLOCATION_ERROR]			=	"Could not allocate node due to a disk error",  			
	[NODE_DEALLOCATION_ERROR]		=	"Could not deallocate node due to a disk error",  			
	[NODE_NOT_FOUND_ERROR]				=	"Could not locate the specified node",  			
	[NODE_IS_NOT_A_FOLDER_ERROR]	=	"The specified node is not a folder, cannot search for folders inside this node",  			
	[NODE_RECURSIVE_DEALLOCATION_ERROR] =	"Recursive deallocation failed",  			
	[FOLDER_NOT_FOUND_ERROR]		=	"No such folder",  			
	[FILE_ALLOCATION_ERROR]			=	"Unable to create file in the current directory due to a partition error",  			
	[FILE_NOT_FOUND_ERROR]			=	"File not found",  			
	[FILE_POINTER_ERROR]			=	"File pointer error: unable to read the complete file from the filesystem",  			
	[FILE_TOO_SMALL_ERROR]			=	"Cannot read the specified size: file is smaller",  			
	[FILE_DEALLOCATION_ERROR]		=	"Cannot remove/deallocate file",  			
	[FILE_ALREADY_EXIST_ERROR]			=	"File already exist",  			
	[PARTITION_TABLE_EMPTY_ERROR]			=	"The partition table is empty for the selected disk",  			
	[GENERAL_INVALID_POINTER_ERROR]		=	"Invalid pointer provided",  			
	[PARTITION_INVALID_ID_ERROR]			=	"Sanity check failed, partition id did not match the reference id inside the partition table",  			
	[PARTITION_POINTER_LOCATION_MISMATCH_ERROR] =	"Sanity check failed, pointer to where the partition begin is different from what's on the partition table",  			
	[PARTITION_VERSION_INCOMPATIBLE_ERROR] =	"Cannot operate inside the current partition, found an incompatible redFs major version used to create this partition",  			
	[PARTITION_MAGIC_ID_IS_INVALID_ERROR]	=	"Sanity check failed on magic number check, the redFs identifier for the cloned memory block is different from the identifier in the current fstab. This indicate a wrong pointer offset or a possible memory corruption, in any case DO NOT operate on this partition.",
	[REDFS_FRAGMENT_OFFSET_ERROR] = "Unable to obtain a valid fragment mapping offset from the fstab",
	[RED_INVALID_ERROR]				=	"Unknown error"
};


#define STRING_LIMIT	  16							/* String len limit, it's used for the node/folder/file naming */
#define PTR_TABLE_TYPE	  uint32_t	
#define BLOCK_SIZE		  (1024*32)						/* 32k of space per memory block, which is log2(max(PTR_TABLE_TYPE))*NODE_SIZE */
#define BLOCK_COUNT		  ( 0xFFFFFFFF / BLOCK_SIZE )
#define BLOCK_NODE_COUNT  ( BLOCK_SIZE / NODE_SIZE  )

#define NODE_ARRAY_LIMIT	(NODE_SIZE-((sizeof(uint8_t)*2)+(sizeof(char)*STRING_LIMIT)+(sizeof(bool))+(sizeof(uint32_t)+(sizeof(RED_PTR)*4))))
#define NODE_FILE_LIMIT		(NODE_SIZE-((sizeof(uint8_t)*2)+(sizeof(char)*STRING_LIMIT)+(sizeof(bool))+(sizeof(RED_PTR)*3)+sizeof(uint32_t))-8)

#define PAGE_STATE_TYPE	  uint8_t
#define PAGE_STATE_LEN	  PTR_TABLE_LEN

/* Block state */

#define FREE_BLOCK			0x00
#define ACTIVE_BLOCK		0x0A
#define FULL_BLOCK			0x1A
#define RESERVED_BLOCK		0xAE

#define FREE_SEGMENT		0x00
#define SEGMENT_ALLOCATED	0xFF

#define PAGE_IS_FILE		0x01
#define PAGE_IS_FOLDER		0x02
#define PAGE_IS_CHAIN		0x30

/* convention used to identify a node who it's a coninuation of another one*/

#define CHAINED_NAME "___c"

/* node  default permission */

#define PAGE_DEF_PERMISSION 0x00

/* RedFs partition table */

typedef struct{
	uint32_t max_disk_size;
	uint8_t  partition_count;
	RED_PTR	 partition_list[PARTITION_LIMIT]; /* list pointer for each partition */
	uint32_t partition_size[PARTITION_LIMIT]; /* partition size specified in bytes */
	uint32_t partition_id[PARTITION_LIMIT];
	char	 partition_name[PARTITION_LIMIT][STRING_LIMIT];
}Red_ptable;

/* Main redFs nodes to handle folder and file creation  */

typedef struct Red_Node{
	uint8_t  type;
	RED_PTR  f_node;
	char	 name[STRING_LIMIT];
	uint8_t  permissions;
	bool	 chained;
	uint32_t content_count;
	RED_PTR	 prev_page;
	RED_PTR  next_page;
	RED_PTR  content[(NODE_ARRAY_LIMIT/sizeof(RED_PTR))];
}Red_Node;

typedef struct Red_File{
	uint8_t  type;
	RED_PTR  f_node;
	char	 name[STRING_LIMIT];
	uint8_t  permissions;
	bool	 chained;
	uint32_t file_size;
	RED_PTR	 prev_page;
	RED_PTR  next_page;
	uint8_t  content[NODE_FILE_LIMIT];
}Red_File;

/* 
 *	Fstab based on a memory block segmentation.
 *	To reduce fstab size, each node is 512 bytes wide and the 
 *	fstab store a list of memory blocks, each 32Kbyte wide. 
 *	During the  allocation the first available block is selected 
 *	only if it can handle (any) allocation.
 *
 */

typedef struct{
	RED_PTR  base_ptr;
	uint8_t  node_count; 
	uint32_t fragment_map; /* 32bit bitmap to map the memory block offset */
}Red_MBlock;


typedef struct{
	uint32_t   redfs_id[3];
	char	   partition_name[STRING_LIMIT];
	uint32_t    version;
	Red_MBlock raw_block_ptr[BLOCK_COUNT]; /* memory block list */
	uint8_t	   block_state[BLOCK_COUNT];
	uint32_t   free_blocks;
	uint32_t   block_limit;
	uint32_t   partition_id;
	RED_PTR    entry_point;
}Red_Fstab;


/*
 *	RedFS partition header. It's a struct that store 
 *	major information about one partition selected by 
 *	the system or a process,  ready to be used.
 *
 */

typedef struct{
	uint32_t  used_space;
	uint32_t  reserved_space;
	RED_PTR   partition_address;
	RED_PTR   root;
	RED_PTR   current_node;
	Red_Fstab fstab;
	uint32_t  cache_timing;
	uint32_t  cache_limit;
}Red_Header;


/* ======================================================================================================== */

/*  RedFS Main API functions */

/*
 *	Header associated with the I/O library of redFs. You MUST customize the implementation of 
 *	the standard functions of redFs used by the library to integrate it with your system.
 *	For more information please check out "redFs_io.h" header file for more informations.
 *
 */

#include "redFs_io.h"

/*
 *	Header of the standard node implementation. This include functions used by the folder implementation 
 *	and the file implementation.
 *	
 */

#include "redFs_node.h"

/*
 *	File and Folder implementation based on the generalized node structure.
 *
 */

#include "redFs_folder.h"
#include "redFs_file.h"

/*
 *	RedFs main errno variable. Every function that perform an interaction with the filesystem 
 *	can generate and error since they are based on the generic functions of the filesystem, which  
 *	can return an error code. 
 *	Every function then can update the redFs_errno to flag the caller about the internal state, even 
 *	if the function is of type void, like print functions and so on. 
 *	It's suggested to use the redFs_errno and/or relative runtime function to get the error code 
 *	only if a function return a non-zero exit code instead of saving the return value directly 
 *	from each call:
 *
 *	Example: 
 *		
 *		int pid = 1003; 
 *		redFs_print_fstab(pid); // this is a void function, you can use redFs_errno to check the exit code 
 *		if(redFs_errno){
 *			redFs_print_strerror(redFs_errno);
 *		}
 *		
 *	Example: 
 *
 *		...
 *		...		// those functions can return directly the exit code, but we are going to use 
 *				// the errno value
 *   	
 *   	file_buffer[read_size] = '\0';
 *   	if(redFs_write_file(&header, "./buffer", (uint8_t*)file_buffer, sizeof(char)*read_size)){
 *			redFs_print_strerror(redFs_errno);
 *   	}
 *   	NOTYF("Written file size: %d", redFs_get_file_size(&header, "./buffer"));
 *   	NOTY("Current dir content");
 *   	if(redFs_print_dir_content(&header, "./")){
 *			redFs_print_strerror(redFs_errno);
 *   	}
 *   	usleep(220000);
 *   	NOTY("Printing fragmentation report");
 *   	redFs_print_fragmentation_report(&header.fstab);
 *
 */

static uint32_t redFs_errno = 0;

/*
 *	Runtime function to get the current errno value.
 *
 */

uint32_t redFs_get_errno();

/*
 *	Accessible software function to get redFs version during runtime.
 *
 */

REDAPI void redFs_get_version(int* major, int* minor, int* patch);

/*
 *	This function is designed to create a new partition table inside the drive with 
 *	a base offset of BOOT_SECTOR_SIZE byte. A Red_ptable structure will be allocated 
 *	and used to store the pointer to the partition, the associated id and the partition 
 *	size. 
 *	Calling this function is necessary ONLY if the selected disk is virgin, without any 
 *	interaction with any instance of redFs in the past. If this function is called with an 
 *	already initialize disk then the partition table will be lost and the reference to 
 *	each partition will be invalidated. 
 *
 *	The partition table store partition reference with a fixed offset, if too many partition 
 *	are created and deleted there's a risk of fragmentation inside the drive. If a new partition 
 *	cannot fit in a deallocated space between two already allocated partition then a new contiguous 
 *	space will be allocated, leaving a free spot inside the partition table's mapping. 
 *	
 */

REDAPI int redFs_init_disk(uint32_t disk_size);

/* 
 *	A new partition will be created inside the partition table of the disk with a char* name associated and 
 *	the size specified. This function create a new fstab and maps the pages of the partition, initialize a 
 *	root node and synch the partition table with the unique id of the partition, the associated size and 
 *	the pointer to the fstab of the partition.
 *
 */

REDAPI int redFs_create_partition(char* name, uint32_t size);

/*
 *	In order to delete a partition it's necessary to provide the partition id and the name of the partition. 
 *
 *	Each partition cannot have a smaller size than sizeof(Red_Fstab). The redFs library allow an allocation 
 *	of a partition with equal or slightly bigger size, but of course you wouldn't be able to create any folder 
 *	or file. 
 *
 */

REDAPI int redFs_delete_partition(char*name,uint32_t partition_id);

/*
 *	redFs_erase_partition() is a user function that serve as a wrapper for the internal redFs_format_partition
 *	function. It will reset the fstab from the disk, initializing it back to the original state. 
 *	The formatting process does not erase all data from the partition, but it reset the fstab; if you have a 
 *	"saving" of the latest fstab before the formatting process, it's possible to still access the file throughout the 
 *	partition with the functions that take a Red_Header* argument until it deallocation is required. 
 *	It's suggested to trigger a delete signal for every instance of a single partition to then proceed with the formatting 
 *	process, you must avoid different processes to access one disk with an old instance of Red_Header* (which include an instance 
 *	of a cached Red_Fstab* ) after the partition is formatted that may cause invalidation of further updates after a reboot or a new 
 *	fetch of Red_Header directly from the disk.
 *
 *	IMPORTANT: if you call functions that use Red_Header and you perform different write action till triggering the auto caching 
 *	system, the cached Red_Header provided as argument will overwrite the cleaned fstab associated with the partition, invalidating the 
 *	previous format. See "format.c" inside "src/test" to see how the formatting process is performed.
 *
 *	IMPORTANT: the previous assigned partition id will be invalidated after a format process. It's suggested to first 
 *	erase the partition, then acquiring the latest id with the dedicated function, then fetching the latest header with the 
 *	obtained id. See "format.c" inside "src/test" to see how the formatting process is performed.
 */

REDAPI int redFs_erase_partition(uint32_t partition_id);

/*
 *	This function print the partition table associated with a specific partition. 
 *
 */

REDAPI void redFs_print_fstab(uint32_t partition_id);

/*
 *	This function print the partition table struct located in the base of the disk. If the disk is not 
 *	initialized then this will print garbage. 
 *
 */

REDAPI void redFs_print_ptable();

/* 
 *	Each redFs function that operate with the disk directly may return different errors depending on what's 
 *	happened during the operation. This function take the error code and returns an string description 
 *	to the caller.
 *
 */

REDAPI const char* redFs_strerror(int return_state);

/*
 *	Similar to redFs_strerror, it accept the error code from any redFs functions and it will print the 
 *	error messages inside the specified output stream.
 *	If it can print into the specified output stream it will return 0, otherwise it will return a non-zero 
 *	exit code. 
 *	
 *	NOTE: the file stream must be opened by the caller. 
 */

REDAPI int redFs_print_strerror(int return_state, FILE* stream);

/*
 *	Shortcut of redFs_print_strerror, it accept the error code from any redFs functions and it will print 
 *	the error message into stdout file stream.
 *
 */

REDAPI int redFs_print_strerror_to_stdout(int return_state);

/*
 *	Similar to redFs_print_strerror_to_stdout, it accept the error code from any redFs functions and it will 
 *	print the error message into stderr filestream.
 *
 */

REDAPI int redFs_print_strerror_to_stderr(int return_state);

/*	
 *	The simple design of redFs allow instance of a partition to be obtained in order to operate with anything 
 *	related to node manipulation. This was a design choice dictated to simplify the integration with simple 
 *	operative sistem or runtime environment. The system can get an instance of one partition header to operate 
 *	on it while maintaining the access to other partitions header that different processes may have 
 *	requested to the system. 
 *
 *	NOTE: The synch between two instances of the same partition cannot be handled directly by redFs due to the 
 *	high complexity of different scenarios, thus the system on top redFs must be capable of handling disk access 
 *	and data coherence to avoid fragmentation or partial data loss. 
 *
 */

REDAPI void redFs_get_partition_header(uint32_t partition_id, Red_Header* rh);

/*
 *	Reading the partition header may fail if there is a problem with the disk interaction, and to prevent 
 *	operating on a failed partition it's possible to use this sanity check function to evaluate the header 
 *	content. 
 *	This function return 0 if the partition is sane, or it will return an error code compatible with redFs_strerror()
 *	indicating the failure point.
 *
 *	NOTE: Sanity check may fail due to different reason, it's suggested to repeat the sanity process more that one time 
 *	if it failed. 
 *	
 *	USAGE EXAMPLE:
 *
 *	To proceed with an accurate check you must fetch Red_Header* and repeat the sanity check process more 
 *	than one time, if it fail after N times you should refetch Red_Header* associated with the partition and repeat the 
 *	process another time to exclude any possible reading error from the disk. 
 *	If it does fail again then it's possible that a disk error or a partition error was indeed present and the partition 
 *	integrity cannot be ensured; if you still use the partition instead of aborting the process then data loss may occur 
 *	which are not responsibility of redFs.
 *
 */

REDAPI int redFs_partition_header_sanity_check(Red_Header* rh);

/*
 *	Navigate the partition table and check for a specified filesystem. If the filesystem is present it return true, 
 *	if not then it will return false. 
 *	By providing a null pointer as an argument it will return true if any partition is defined, and it will return 
 *	false if the disk is initialized but with no partition allocated.
 *
 */

REDAPI bool redFs_partition_defined(char* partition_name);

/*
 *	Search inside the partition table for the specified partition. If a match is found then the dedicated id of
 *	the partition is returned, else a 0 will be returned.
 *
 */

REDAPI uint32_t redFs_get_partition_id_from_name(char* partition_name);

/*
 *	Similar to redFs_get_partition_id_from_name, if a match is found inside the partition table, then the 
 *	name is copied inside the destination buffer provided as an argument ( note that the maximum size for 
 *	a partition name is STRING_LIMIT, ensure to have a char* dest buffer with at least this size), if nothing 
 *	is found then dest[0] will be set to Hex 0x00 ( C string termination ). 
 *
 */

REDAPI int redFs_get_partition_name_from_id(char* dest, uint32_t partition_id);

/*
 *	This function can print partition header's stats to the standard output. 
 *
 */

REDAPI void redFs_print_partition_header(Red_Header* rh);

/*
 *	Each disk access and node manipulation increase a counter that keep tracks of the total operation for 
 *	time instance. Those operation change inevitably the fstab associated with a partition, wheter it's a 
 *	folder creation or a file deletion. It's not possible and convinient to continuousy access the disk to 
 *	synch back those little updates for speed reason and disk general health. 
 *	RedFs manage that with a function that synch back the latest fstab state on one header, this is 
 *	also done automatically if the number of disk write access is greater than the default threshold 
 *	which is defined by CACHE_TIME_LIMIT. ( each write access increase this counter by 1. )
 *	Each header has his own counter that will be reset each time the write access surpass this limit. 
 *
 *	IMPORTANT: If a partition is closed you MUST call this function manually to synch the latest changes 
 *	inside the fstab that may have not be catched by the automatic synch system.
 *
 */

REDAPI int redFs_sync_partition(Red_Header* header);

/*
 *	This function print a fragmentation report for a generic Red_Fstab that may be associated with a partition 
 *	or be created separately. Each page has his own fragment map that's used to both allocate and provide an 
 *	offset for a new node starting from the page block pointer, which also simplify keeping track of the fragmentation 
 *	allowing a pretty graphics representation on what's going on inside your partition. 
 *
 *	This take account also the disk memory mapped to store the partition's fstab itself. 
 *
 */

REDAPI void redFs_print_fragmentation_report(Red_Fstab* fstab);

/* ======================================================================================================== */

/*
 *	Internal filesystem functions used by redFs.  
 *	Use it with caution. It's suggested to read the source code for each one of them before proceed 
 *	using it.
 */

int redFs_format_partition_table(uint32_t max_disk_size);
int redFs_write_boot_sector(uint8_t*content, uint32_t len);
int redFs_update_partition_table(uint32_t p_fstab_adr,uint32_t size, uint32_t partition_id, uint8_t partition_number, char* name);
int redFs_update_last_on_partition_table(uint32_t p_fstab_adr, uint32_t size,uint32_t partition_id, char* name);
int redFs_push_on_partition_table(uint32_t p_fstab_adr, uint32_t size, uint32_t partition_id, char* name);
int redFs_pop_off_partition_table();
Red_ptable redFs_get_partition_table();
int redFs_rewrite_partition_table(Red_ptable new_ptable);
int redFs_sort_sync_partition_table();
RED_PTR redFs_calculate_new_partition_offset(uint32_t size);
uint32_t redFs_generate_partition_id();
int redFs_define_fstab(char* partition_name, uint32_t partition_size, uint32_t starting_point, Red_Fstab* fstab);
Red_Fstab* redFs_get_fstab(uint8_t partition_number);
int redFs_update_fstab(Red_Fstab fstab, uint8_t partition_number);
int redFs_get_free_fragment_offset(uint32_t fragment_map);
int redFs_format_partition(char* partition_name, uint32_t partition_size, uint32_t starting_address, Red_Fstab* fstab);
void redFs_debug_print_fstab(Red_Fstab* fstab);
int redFs_cache_update(Red_Header *header);


#ifndef REDFS_IMP
#define REDFS_IMP


#endif // REDFS_IMP
#endif // REDFS_H
