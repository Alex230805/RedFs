#ifndef REDFS_H
#define REDFS_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

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
 *	a simple file system implementation for low power and simple systems
 *
 *	This is a filesystem built with the intention to be used on small devices like microcomputer 
 *	or microcontroller, generally speaking in a limited computational environment. To do so the 
 *	redFs embed different functions for not only managing the disk but also for managing partition 
 *	tables, boot sector and so on. Due to the fact that it's desinged for small system the structure 
 *	for the disk partitioning and how it's built could be different from the modern way of doing things, 
 *	it's designed to be as simple as possible without wasting resources.
 *
 *	The boot sector is 512 byte wide.
 *
 *	The partition table is defined by the Red_ptable.
 *	
 *	Max addressable space ( 32bit pointer limit ): 4gb
 *
 *	Each partition is flagged with the redFs id ( which is composed by the prefix+id+suffix), during 
 *	the partition's finding task this is used as the main verification code for the partition integrity. 
 *	
 *	To be able to work with partitions easily redFs provide a function to manage the main partition table.
 *	It's essential to have a pointer table to each partition on the disk to avoid a manual search as 
 *	much as possible, and for that reason redFs embedd a partitioning system which create a small table 
 *	with an offset larger than the boot sector size ( which is 512 bytes by default in redFs ) and from 
 *	there a special struct is placed to address each fstab entry point.
 *
 */
/* ======================================================================================================== */

#define REDFS_ID_PREFIX			94694209
#define REDFS_ID				('R'<<24 | 'D'<<16 | 'F'<<8 | 'S')
#define REDFS_SUFFIX			96499042

#define REDFS_VERSION			0x010000 /* redfs version  */
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
	PARTITION_TABLE_EMPTY,
	NOT_ENOUGH_DISK_SPACE_ERROR,
	FSTAB_READ_ERROR,
	FSTAB_WRITE_ERROR,
	FSTAB_PAGE_WRITE_ERROR,
	PARTITION_FORMAT_DISK_ERROR,
	PARTITION_SIZE_NOT_SUFFICIENT,
	PARTITION_ACTION_UNKNOWN,
	PARTITION_NODE_WRITING_ERROR,
	PARTITION_NODE_READING_ERROR,
	REDFS_UNSUPPORTED_FUNCTION,
	REDFS_BLOCK_FRAGMENT_ERROR,
	NODE_ALLOCATION_ERROR,
	NODE_DEALLOCATION_ERROR,
	NODE_NOT_FOUND,
	NODE_IS_NOT_A_FOLDER_ERROR,
	NODE_RECURSIVE_DEALLOCATION_ERROR,
	FOLDER_NOT_FOUND_ERROR,
	FILE_ALLOCATION_ERROR,
	FILE_NOT_FOUND_ERROR,
	FILE_POINTER_ERROR,
	FILE_TOO_SMALL_ERROR,
	FILE_DEALLOCATION_ERROR,
	FILE_ALREADY_EXIST,
	GENERAL_INVALID_POINTER,
	PARTITION_INVALID_ID,
	PARTITION_POINTER_LOCATION_MISMATCH,
	PARTITION_VERSION_INCOMPATIBLE,
	PARTITION_MAGIC_ID_IS_INVALID
}Red_State;

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
 *	RedFS partition header. It's a struct that store the 
 *	major information about one partition selected by 
 *	the system ready to be used by one or more process.
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
 *	space will be allocated, leaving a free spot inside the partition table. 
 *	
 */

int redFs_init_disk(uint32_t disk_size);

/* 
 *	A new partition will be created inside the partition table of the disk with a char* name associated and 
 *	the size specified. This function create a new fstab and maps the pages of the partition, initialize a 
 *	root node and synch the partition table with the unique id of the partition, the associated size and 
 *	the pointer to the fstab of the partition.
 *
 */

int redFs_create_partition(char* name, uint32_t size);

/*
 *	In order to delete a partition it's necessary to provide the partition id associated with the partition 
 *	and the name of the partition. This is done to provide a minimum of protection layer ti avoid deleting 
 *	unwanted partition. 
 *
 *	Each partition cannot have a smaller size than the sizeof(Red_Fstab). The redFs library allow an allocation 
 *	of a partition with equal or slightly bigger size, but of course you wouldn't be able to create any folder 
 *	or file. 
 *
 */

int redFs_delete_partition(char*name,uint32_t partition_id);

/*
 *	This function print the partition table associated with a specific partition. 
 *
 */

void redFs_print_fstab(uint32_t partition_id);

/*
 *	This function print the partition table struct located in the base of the disk. If the disk is not 
 *	initialized then this will print garbage. 
 *
 */

void redFs_print_ptable();

/* 
 *	Each redFs function that operate with the disk directly may return different errors depending on what's 
 *	happened during the operation. This function take the error code and prints out a stringified format 
 *	to show the error. 
 *
 */

void redFs_strerror(int return_state);

/*	
 *	The simple design of redFs allow instance of a partition to be obtained in order to operate with anything 
 *	related to node manipulation. This was a design choice dictated to simplify the integration with simple 
 *	operative sistem or runtime environment. The system can get an instance of one partition header to operate 
 *	on one partition while maintaining the access to other partitions header that different processes may have 
 *	requested to redFs. 
 *
 *	NOTE: The synch between two instances of the same partition cannot be handled directly by redFs due to the 
 *	high complexity of different scenarios, thus the system on top redFs must be capable of handling disk access 
 *	and data coherence to avoid fragmentation or partial data loss. 
 *
 */

void redFs_get_partition_header(uint32_t partition_id, Red_Header* rh);

/*
 *	Reading the partition header may fail if there is a problem with the disk interaction, and to prevent 
 *	operating on a failed partition it's possible to use this sanity check function to evaluate the header 
 *	content. 
 *	This function return 0 if the partition is sane, or it will return an error code compatible with redFs_strerror()
 *	indicating the failure point.
 *
 */

int redFs_partition_header_sanity_check(Red_Header* rh);

/*
 *	Navigate the partition table and check for a specified filesystem. If the filesystem is present it return true, 
 *	if not then it will return false. 
 *	By providing a null pointer as an argument it will return true if any partition is defined, and it will return 
 *	false if the disk is initialized but with no partition allocated.
 *
 */

bool redFs_partition_defined(char* partition_name);

/*
 *	Search inside the partition table for the specified partition. If a match is found then the dedicated id of
 *	the partition is returned, else a 0 will be returned.
 *
 */

uint32_t redFs_get_partition_id_from_name(char* partition_name);

/*
 *	Similar to redFs_get_partition_id_from_name, if a match is found inside the partition table, then the 
 *	name is copied inside the destination buffer provided as an argument ( note that the maximum size for 
 *	a partition name is STRING_LIMIT, ensure to have a char* dest buffer with at least this size), if nothing 
 *	is found then the dest buffer a string termination will be set as the first character. 
 *
 */

int redFs_get_partition_name_from_id(char* dest, uint32_t partition_id);

/*
 *	This function can print those stats and show them to the standard output. 
 *
 */

void redFs_print_partition_header(Red_Header* rh);

/*
 *	Each disk access and node manipulation increase a counter that keep tracks of the total operation for 
 *	time instance. Those operation change inevitably the fstab associated with a partition, wheter it's a 
 *	folder creation or a file deletion. It's not possible and convinient to continuousy access the disk to 
 *	synch back those little updates for speed reason and disk general healt. 
 *	RedFs manage that by having a function that synch back the latest fstab state on one header, this is 
 *	also done automatically if the if the number disk write access is greater than the default threshold 
 *	which is defined by CACHE_TIME_LIMIT. 
 *	Each header has his own counter that will be reset each time the write access surpass this limit. 
 *
 *	IMPORTANT: If a partition is closed you must call this function manually to synch the latest changes 
 *	inside the fstab that may have not be catched by the automatic synch system.
 *
 */

int redFs_sync_partition(Red_Header* header);

/*
 *	This function print a fragmentation report for a generic Red_Fstab that may be associated with a partition 
 *	or be created separately. Each page has his own fragment map that's used to both allocate and provide an 
 *	offset for a new node, which simplify keeping track of the fragmentation allowing also a pretty graphics 
 *	representation on what's going on inside your partition. 
 *
 *	This take account also the disk memory mapped to store the partition's fstab itself. 
 *
 */

void redFs_print_fragmentation_report(Red_Fstab* fstab);

/* ======================================================================================================== */

/*
 *	Internal filesystem functions used by redFs.  
 *	Use it with caution.
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
RED_PTR redFs_caclulate_new_partition_offset(uint32_t size);
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
