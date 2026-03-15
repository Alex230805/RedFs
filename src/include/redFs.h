#ifndef REDFS_H
#define REDFS_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/*
 * RedFS: a simple file system for low power architectures.
 *
 * This is a filesystem built with the intention to be used on small devices like microcomputer 
 * or microcontroller, generally speaking in a limited computational environment. To do so the 
 * redFs embed different functions for not only managing the disk but also for managing partition 
 * tables, boot sector and so on. Due to the fact that it's desinged for small system the structure 
 * for the disk partitioning and how it's built could be different from the modern way of doing things, 
 * it's designed to be as simple as possible without wasting resources.
 *
 * The boot sector is 512 byte wide.
 *
 * The partition table is defined by the Red_ptable.
 *	
 * Max addressable space ( 32bit pointer limit ): 4gb
 *
 * Each partition is flagged with the redFs id ( which is composed by the prefix+id+suffix), during 
 * the partition finding task this is used as the main verification code for the partition integrity. 
 *	
 * To be able to work with partitions easily redFs provide a function to manage the main partition table.
 * It's essential to have a pointer table to each partition on the disk to avoid a manual search as 
 * much as possible, and for that reason redFs embedd a partitioning system which create a small table 
 * with an offset larger than the boot sector size ( which is 512 bytes by default in redFs ) and from 
 * there a special struct is placed to address each fstab entry point.
 *
 * */

#define REDFS_ID_PREFIX 94694209
#define REDFS_ID		'R'<<24 | 'D'<<16 | 'F'<<8 | 'S'
#define REDFS_SUFFIX	96499042

#define REDFS_VERSION 1
#define PARTITION_LIMIT 256
#define BOOT_SECTOR_SIZE 512 // bytes
#define PARTITION_BLANK_OFFSET 1024 // byte offset that separate two partitions
#define NODE_SIZE 1024
//#define NODE_SIZE 512
#define CACHE_TIME_LIMIT 1024*20 // number of access before the fstab inside the Red_Header is synched to the drive

#define RED_PTR uint32_t

typedef enum{
	NOERROR = 0,
	PARTITION_TABLE_FORMAT_ERROR,
	BOOT_SECTOR_WRITING_ERROR,
	PARTITION_TABLE_WRITE_ERROR,
	PARTITION_TABLE_READ_ERROR,
	PARTITION_NOT_FOUND_ERROR,
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
	NODE_ALLOCATION_ERROR
}Red_State;


/*
 * RedFs partition table
 *
 * */


typedef struct{
	uint32_t max_disk_size;
	uint8_t partition_count;
	RED_PTR partition_list[PARTITION_LIMIT]; // list pointer for each partition
	uint32_t partition_size[PARTITION_LIMIT]; // in bytes
	uint32_t partition_id[PARTITION_LIMIT];
}Red_ptable;


/*
 * Main redFs nodes to handle folder and file creation
 *
 * */


#define STRING_LIMIT 16

#define PTR_TABLE_TYPE uint32_t

#define BLOCK_SIZE (1024*32) // 32k of space per memory block
#define BLOCK_COUNT ( 0xFFFFFFFF / BLOCK_SIZE )
#define BLOCK_NODE_COUNT ( BLOCK_SIZE / NODE_SIZE )

#define NODE_ARRAY_LIMIT (NODE_SIZE-((sizeof(uint8_t)*2)+(sizeof(char)*STRING_LIMIT)+(sizeof(bool))+(sizeof(uint32_t)+(sizeof(RED_PTR)*4))))
#define NODE_FILE_LIMIT (NODE_SIZE-((sizeof(uint8_t)*2)+(sizeof(char)*STRING_LIMIT)+(sizeof(bool))+(sizeof(RED_PTR)*4)))


#define PAGE_STATE_TYPE uint8_t
#define PAGE_STATE_LEN PTR_TABLE_LEN

// Block state
//
#define FREE_BLOCK		0x00
#define ACTIVE_BLOCK	0x0A
#define FULL_BLOCK		0x1A
#define RESERVED_BLOCK  0xAE

// One segment is a part of the memory block, 
// each memory block is composed of BLOCK_NODE_COUNT fragment
//
#define FREE_SEGMENT		0x00
#define SEGMENT_ALLOCATED	0xFF

#define PAGE_IS_FILE	0x01
#define PAGE_IS_FOLDER	0x02
#define PAGE_IS_CHAIN	0x30

#define CHAINED_NAME "___c"

#define PAGE_DEF_PERMISSION 0x00

typedef struct Red_Node{
	uint8_t type;
	RED_PTR f_node;
	char name[STRING_LIMIT];
	uint8_t permissions;
	bool chained;
	uint32_t content_count;
	RED_PTR	 prev_page;
	RED_PTR  next_page;
	RED_PTR  content[(NODE_ARRAY_LIMIT/sizeof(RED_PTR))];
}Red_Node;

typedef struct Red_File{
	uint8_t type;
	RED_PTR f_node;
	char name[STRING_LIMIT];
	uint8_t permissions;
	bool chained;
	RED_PTR	 prev_page;
	RED_PTR  next_page;
	uint8_t content[NODE_FILE_LIMIT-4];
}Red_File;


/* 
 * Simple fstab based on a memory block segmentation.
 * To reduce fstab size, each node is 512 bytes wide and the 
 * fstab store a list of memory blocks each 32Kbyte wide. During the 
 * allocation the first available block is selected only if it can 
 * handle an allocation, it means that if a block still have free 
 * space it will be selected as the target for the node allocation,
 * and to determine this each Red_MBlock have a "node_state" array, 
 * the width is the size of the memory block divided by the 
 * size of each node ( 512byte ). It store not only the node state, 
 * but it's also used to offset the base pointer of the memory 
 * block to a free space to enable the allocation. 
 *
 * This improve the size of the fstab since it will store 
 * fewer pointer and state for each page and with a smaller node 
 * the fragmentation of the drive due to the normal usage will be 
 * reduced drastically compared to the previous version, which to 
 * reduce fstab size down to 2Mb each node was 8Kb wide.
 *
 * */

typedef struct{
	RED_PTR base_ptr;
	uint8_t node_count; 
	uint32_t fragment_map; // bitmap to keep track of allocated node inside the memory map 
}Red_MBlock;

typedef struct{
	uint32_t redfs_id[3];
	char partition_name[STRING_LIMIT];
	uint8_t version;
	
	Red_MBlock raw_block_ptr[BLOCK_COUNT];
	uint8_t	block_state[BLOCK_COUNT];

	uint32_t free_blocks;
	uint32_t block_limit;
	uint32_t partition_id;
	RED_PTR entry_point;
}Red_Fstab;


/*
 *	RedFS partition header. It's a struct that store the 
 *	major information about one partition selected by 
 *	the system ready to be used by one or more process.
 *
 * */

typedef struct{
	uint32_t used_space;
	uint32_t reserved_space;
	RED_PTR partition_address;

	RED_PTR root;
	RED_PTR current_node;

	Red_Fstab fstab;
	uint32_t cache_timing;
	uint32_t cache_limit;
}Red_Header;



/*
 * Internal filesystem functions, used by each main public function 
 * of redFs. Usable to get more control over your disk. 
 *
 * Use it with caution.
 *
 * */

#include "redFs_io.h"
#include "redFs_node.h"

int redFs_format_partition_table(uint32_t max_disk_size);
int redFs_write_boot_sector(uint8_t*content, uint32_t len);

int redFs_update_partition_table(uint32_t p_fstab_adr,uint32_t size, uint32_t partition_id, uint8_t partition_number);
int redFs_update_last_on_partition_table(uint32_t p_fstab_adr, uint32_t size,uint32_t partition_id);
int redFs_push_on_partition_table(uint32_t p_fstab_adr, uint32_t size, uint32_t partition_id);
int redFs_pop_off_partition_table();
Red_ptable redFs_get_partition_table();
int redFs_rewrite_partition_table(Red_ptable new_ptable);
int redFs_sort_sync_partition_table();
RED_PTR redFs_caclulate_new_partition_offset(uint32_t size);
uint32_t redFs_generate_partition_id();

int redFs_define_fstab(char* partition_name, uint32_t partition_size, uint32_t starting_point, Red_Fstab* fstab);
Red_Fstab* redFs_get_fstab(uint8_t partition_number);
int redFs_update_fstab(Red_Fstab fstab, uint8_t partition_number);
void redFs_print_fragmentation_report(Red_Fstab* fstab);
int redFs_get_free_fragment_offset(uint32_t fragment_map);
int redFs_format_partition(char* partition_name, uint32_t partition_size, uint32_t starting_address, Red_Fstab* fstab);
void redFs_debug_print_fstab(Red_Fstab* fstab);

/*
 *
 * redFs standard API functions 
 *
 * */

int redFs_init_disk(uint32_t disk_size);
int redFs_create_partition(char* name, uint32_t size);
int redFs_delete_partition(char*name,uint32_t partition_id);
void redFs_print_fstab(uint32_t partition_id);
void redFs_print_ptable();
void redFs_strerror(int return_state);
void redFs_get_partition_header(uint32_t partition_id, Red_Header* rh);
void redFs_print_partition_header(Red_Header* rh);

/*
 * 
 * Before closing the partition the system must synch changing inside the fstab used to 
 * operate with the filesystem, if this is not done all modification about folders, files 
 * and so on will be reverted to the last auto synch. Before changing header it's a good 
 * practice to always synch the header to the drive to avoid data fragmentation problem.
 *
 * */

int redFs_sync_partition(Red_Header* header);


#ifndef REDFS_IMP
#define REDFS_IMP


#endif // REDFS_IMP

#endif // REDFS_H
