#define REDFS_IMP

#include "redFs.h"

int redFs_format_partition_table(uint32_t max_disk_size){
	Red_ptable ptable = {0};
	ptable.max_disk_size = max_disk_size;
	uint32_t size = sizeof(Red_ptable);
	uint32_t offset = BOOT_SECTOR_SIZE;
	for(uint32_t i=0;i<size;i++){
		if(redFs_disk_action_write(offset+i, *((uint8_t*)&ptable+i))){
			return (int)PARTITION_TABLE_FORMAT_ERROR;
		}
	}
	return 0;
}

int redFs_write_boot_sector(uint8_t*content, uint32_t len){
	for(uint32_t i=0;i<len && len < BOOT_SECTOR_SIZE;i++){
		if(redFs_disk_action_write(i, content[i])){
			return (int)BOOT_SECTOR_WRITING_ERROR;
		}
	}
	return 0;
}

int redFs_update_partition_table(uint32_t p_fstab_adr, uint32_t size,uint32_t partition_id, uint8_t partition_number){
	Red_ptable ptable = redFs_get_partition_table();
	if(partition_number > ptable.partition_count){
		return (int)PARTITION_NOT_FOUND_ERROR;
	}
	ptable.partition_list[partition_number] = p_fstab_adr;
	ptable.partition_size[partition_number] = size;
	ptable.partition_id[partition_number] = partition_id;
	if(redFs_rewrite_partition_table(ptable)){
		return (int)PARTITION_TABLE_WRITE_ERROR;
	}
	return 0;
}
int redFs_update_last_on_partition_table(uint32_t p_fstab_adr, uint32_t size,uint32_t partition_id){
	Red_ptable ptable = redFs_get_partition_table();
	ptable.partition_list[ptable.partition_count-1] = p_fstab_adr;
	ptable.partition_size[ptable.partition_count-1] = size;
	ptable.partition_id[ptable.partition_count-1] = partition_id;
	if(redFs_rewrite_partition_table(ptable)){
		return (int)PARTITION_TABLE_WRITE_ERROR;
	}
	return 0;
}


int redFs_push_on_partition_table(uint32_t p_fstab_adr, uint32_t size, uint32_t id){
	Red_ptable ptable = redFs_get_partition_table();
	ptable.partition_list[ptable.partition_count] = p_fstab_adr;
	ptable.partition_size[ptable.partition_count] = size;
	ptable.partition_size[ptable.partition_count] = id;
	ptable.partition_count+=1;
	if(redFs_rewrite_partition_table(ptable)){
		return (int)PARTITION_TABLE_WRITE_ERROR;
	}
	return 0;
}

int redFs_pop_off_partition_table(){
	Red_ptable ptable = redFs_get_partition_table();
	ptable.partition_list[ptable.partition_count-1] = 0;
	ptable.partition_size[ptable.partition_count-1] = 0;
	ptable.partition_id[ptable.partition_count-1] = 0;
	ptable.partition_count-=1;
	if(redFs_rewrite_partition_table(ptable)){
		return (int)PARTITION_TABLE_WRITE_ERROR;
	}
	return 0;
}


Red_ptable redFs_get_partition_table(){
	Red_ptable p = {0};
	uint32_t size = sizeof(Red_ptable);
	int offset = BOOT_SECTOR_SIZE;
	for(uint32_t i=0;i<size;i++){
		uint8_t buffer;
		if(redFs_disk_action_read(offset+i, &buffer)){
			Red_ptable dp = {0};
			return dp;
		}
		*((uint8_t*)&p+i) = buffer;
	}
	return p;
}

int redFs_rewrite_partition_table(Red_ptable new_ptable){
	int size = sizeof(Red_ptable);
	int offset = BOOT_SECTOR_SIZE;
	for(int i=0;i<size;i++){
		if(redFs_disk_action_write(offset+i, *((uint8_t*)&new_ptable+i))){
			return (int)PARTITION_TABLE_FORMAT_ERROR;
		}
	}
	return 0;
}

int redFs_sort_sync_partition_table(){
	Red_ptable ptable = redFs_get_partition_table();
	
	RED_PTR swap_ptr = 0;
	uint32_t swap_size = 0;
	uint32_t swap_id = 0;

	for(uint32_t j=0;j<ptable.partition_count;j++){
		for(uint32_t i=0;i<ptable.partition_count-1;i++){
			if(ptable.partition_list[i] > ptable.partition_list[i+1]){
				swap_ptr = ptable.partition_list[i+1];
				swap_size = ptable.partition_size[i+1];
				swap_id = ptable.partition_id[i+1];

				ptable.partition_list[i+1] = ptable.partition_list[i];
				ptable.partition_id[i+1] =	 ptable.partition_id[i];
				ptable.partition_size[i+1] = ptable.partition_size[i];

				ptable.partition_list[i] = swap_ptr; 
				ptable.partition_id[i]   = swap_id;
				ptable.partition_size[i] = swap_size;
			}
		}
	}
	
	int err = redFs_rewrite_partition_table(ptable);
	return err;
}


RED_PTR redFs_caclulate_new_partition_offset(uint32_t size){
	// note: this is based on a ordered partition table
	Red_ptable ptable = redFs_get_partition_table();
	if(ptable.partition_count < 1) return sizeof(Red_ptable)+BOOT_SECTOR_SIZE+PARTITION_BLANK_OFFSET;
	
	// search for usable space between the base and the first partition of the disk
	if(ptable.partition_list[0] - sizeof(Red_ptable)+BOOT_SECTOR_SIZE+PARTITION_BLANK_OFFSET >= size){
		return sizeof(Red_ptable)+BOOT_SECTOR_SIZE+PARTITION_BLANK_OFFSET;
	}

	// search for usable space between existing partitions
	for(uint32_t i=0;i<ptable.partition_count-1;i++){
		uint32_t start = ptable.partition_size[i] + ptable.partition_list[i]+PARTITION_BLANK_OFFSET;
		uint32_t end =  ptable.partition_list[i+1];
		if(end-start >= size){ return start; }
	}

	// if there's no space between existing partition, a new one after the last one is created
	uint32_t start = ptable.partition_size[ptable.partition_count-1] + ptable.partition_list[ptable.partition_count-1]+PARTITION_BLANK_OFFSET;
	return start;
}

uint32_t redFs_generate_partition_id(){
	Red_ptable ptable = redFs_get_partition_table();
	uint32_t bid = 1000;
	for(int i=0;i<PARTITION_LIMIT ;i++){
		if(ptable.partition_id[i] > bid){
			bid = ptable.partition_id[i];
		}
	}
	return bid+1;
}

int redFs_define_fstab(char* partition_name, uint32_t partition_size, uint32_t starting_point, Red_Fstab* fstab){
	fstab->redfs_id[0] = REDFS_ID_PREFIX;
	fstab->redfs_id[1] = REDFS_ID;
	fstab->redfs_id[2] = REDFS_SUFFIX;
	memcpy(fstab->partition_name, partition_name, sizeof(char)*STRING_LIMIT);
	fstab->partition_name[STRING_LIMIT-1] = '\0';
	fstab->version = REDFS_VERSION;
	fstab->partition_id = redFs_generate_partition_id();
	Red_ptable ptable = redFs_get_partition_table();
	if(starting_point+partition_size > ptable.max_disk_size){
		return 1;
	}
	if(partition_size < sizeof(Red_Fstab)) return 1;
	
	// TODO: add not-contiguous allocation and mapping 
	uint32_t i=0;
	for(i=0;i<(uint32_t)(partition_size/BLOCK_SIZE);i++){
		fstab->raw_block_ptr[i].base_ptr = starting_point + i*BLOCK_SIZE;
		fstab->block_state[i] = FREE_BLOCK;
		fstab->raw_block_ptr[i].node_count = 0;
		fstab->raw_block_ptr[i].fragment_map = 0;
	}

	uint32_t block_per_fstab = (sizeof(Red_Fstab) / BLOCK_SIZE)+1;
	for(i=0;i<block_per_fstab;i++){
		fstab->block_state[i] = RESERVED_BLOCK;
		fstab->raw_block_ptr[i].node_count = BLOCK_NODE_COUNT;
		fstab->raw_block_ptr[i].fragment_map = 0xFFFFFFFF;
	}

	fstab->free_blocks = (uint32_t)(partition_size/BLOCK_SIZE) - block_per_fstab;
	fstab->block_limit = (uint32_t)(partition_size/BLOCK_SIZE);
	fstab->entry_point = 0;
	return 0;
}

Red_Fstab* redFs_get_fstab(uint8_t partition_number){
	static Red_Fstab fstab = {0};
	Red_ptable ptable = redFs_get_partition_table();
	if(partition_number > ptable.partition_count){
		return &fstab;
	}
	RED_PTR address = ptable.partition_list[partition_number];

	for(uint32_t i=0;i<sizeof(Red_Fstab);i++){
		uint8_t buffer = 0;
		if(redFs_disk_action_read(address+i, &buffer)){
			static Red_Fstab dfstab = {0};
			return &dfstab;
		}
		*((uint8_t*)&fstab+i) = buffer;
	}
	return &fstab;
}


int redFs_update_fstab(Red_Fstab fstab, uint8_t partition_number){
	Red_ptable ptable = redFs_get_partition_table();
	if(partition_number > ptable.partition_count){
		return (int)PARTITION_NOT_FOUND_ERROR;
	}
	RED_PTR address = ptable.partition_list[partition_number];

	for(uint32_t i=0;i<sizeof(Red_Fstab);i++){
		if(redFs_disk_action_write(address+i, *((uint8_t*)&fstab+i))){
			return (int)FSTAB_WRITE_ERROR;
		}
	}
	return 0;
}


int redFs_get_free_fragment_offset(uint32_t fragment_map){
	uint32_t i;
	for(i=0; i < sizeof(uint32_t)*8;i++){
		int f = (fragment_map >> i) & 0x01;
		if(f == 0){
			return i;
		}
	}
	return -1;
}


int redFs_format_partition(char* partition_name, uint32_t partition_size, uint32_t starting_point, Red_Fstab* fstab){

	/* 
	 * a preallocated address inside the partition table is expected to be found at the time 
	 * of calling this function, this is the role of "starting_point" address which is the 
	 * new allocated partition. If that's not the case you must first calculate the offset for 
	 * the partition with the help of the "redFs_caclulate_new_partition_offset()" function and 
	 * manually update the partition table with "redFs_push_on_partition_table(uint32_t p_fstab_adr, uint32_t size)" 
	 * or similar functions provided by the API. It's suggested to use the wrapper "redFs_create_partition()"
	 * which handle all the necessary task, including this function call
	 *
	 * */

	if(redFs_define_fstab(partition_name, partition_size, starting_point, fstab)){
		return (int)NOT_ENOUGH_DISK_SPACE_ERROR;
	}
	
	// define entry point node
	Red_Node entry_point = {0};
	entry_point.type = PAGE_IS_FOLDER;
	strcpy(entry_point.name, "root");
	entry_point.permissions = PAGE_DEF_PERMISSION;
	entry_point.chained = false;
	entry_point.prev_page = 0;
	entry_point.next_page = 0;
	entry_point.content_count = 0;
	memset(entry_point.content, 0x00, NODE_ARRAY_LIMIT/sizeof(RED_PTR));
	// locate and write entry point node 
	uint32_t i=0;
	while((fstab->block_state[i] == RESERVED_BLOCK || fstab->block_state[i] == FULL_BLOCK) && i < fstab->block_limit){i+=1;}
	int frag_offset = redFs_get_free_fragment_offset(fstab->raw_block_ptr[i].fragment_map);
	if(frag_offset == -1) return 1;

	RED_PTR node_adr = fstab->raw_block_ptr[i].base_ptr + frag_offset*sizeof(Red_Node);
	for(uint32_t k=0;k<sizeof(Red_Node);k++){
		if(redFs_disk_action_write(node_adr+k, *((uint8_t*)&entry_point+k))){
			return (int)FSTAB_PAGE_WRITE_ERROR;
		}
	}
	fstab->raw_block_ptr[i].fragment_map = fstab->raw_block_ptr[i].fragment_map | (1 << frag_offset);

	if(fstab->block_state[i] == FREE_BLOCK){
		fstab->block_state[i] = ACTIVE_BLOCK;
	}

	if(fstab->raw_block_ptr[i].node_count+1 >= BLOCK_NODE_COUNT){
		fstab->free_blocks -= 1;
		fstab->block_state[i] = FULL_BLOCK;
	}else{
		fstab->raw_block_ptr[i].node_count += 1;
	}

	fstab->entry_point = node_adr;

	// save fstab->inside the drive
	for(i=0;i<sizeof(Red_Fstab);i++){
		if(redFs_disk_action_write(starting_point+i, *((uint8_t*)fstab+i))){
			return (int)FSTAB_WRITE_ERROR;
		}
	}

	return 0;
}


int redFs_init_disk(uint32_t disk_size){
	if(redFs_format_partition_table(disk_size)){
		return (int)PARTITION_TABLE_FORMAT_ERROR;
	}
	for(int i=0;i<BOOT_SECTOR_SIZE;i++){
		if(redFs_disk_action_write(i, 0)){
			return (int)BOOT_SECTOR_WRITING_ERROR;
		}
	}
	return 0;
}

int redFs_create_partition(char* name, uint32_t size){
	if(size < sizeof(Red_Fstab)) return (int)PARTITION_SIZE_NOT_SUFFICIENT;
	static Red_Fstab fstab = {0};
	RED_PTR offset = redFs_caclulate_new_partition_offset(size);
	if(offset == 0){
		return NOT_ENOUGH_DISK_SPACE_ERROR;
	}
	
	int ret = redFs_push_on_partition_table(offset, size, 0);
	if(ret){
		int pop_err = redFs_pop_off_partition_table();
		if(pop_err) return pop_err;
		return ret;
	}
	ret = redFs_format_partition(name, size, offset, &fstab);
	if(ret) return ret;
	ret = redFs_update_last_on_partition_table(offset, size, fstab.partition_id);
	if(ret) return ret;
	ret = redFs_sort_sync_partition_table();
	return ret;
}

/* 
 * soft delete: the partition and the relative fstab is still 
 * present inside the disk. The entry address is just removed 
 * from the partition table 
 * 
 * */

int redFs_delete_partition(char*name,uint32_t partition_id){
	bool end = false;
	Red_ptable ptable = redFs_get_partition_table();
	if(ptable.max_disk_size == 0){
		return (int)PARTITION_TABLE_READ_ERROR;
	}
	for(int i=0;i<ptable.partition_count && !end;i++){
		if(ptable.partition_id[i] == partition_id){
			Red_Fstab* fstab = redFs_get_fstab(i);
			if(fstab->redfs_id[0] == 0){
				return (int)FSTAB_READ_ERROR;
			}
			if(strcmp(fstab->partition_name, name) == 0){
				/* deleting the partition entry and shifting back the remaining pointer */
				for(int j=i;j<(ptable.partition_count-i)-1;j++){
					ptable.partition_list[j] = ptable.partition_list[j+1];
					ptable.partition_size[j] = ptable.partition_size[j+1];
					ptable.partition_id[j] = ptable.partition_id[j+1];
				}
				ptable.partition_count -= 1;
				int err = redFs_rewrite_partition_table(ptable);
				if(err) return err;
				end = true;
				err = redFs_sort_sync_partition_table();
				return err;
			}
		}
	}
	if(!end){
		return (int)PARTITION_NOT_FOUND_ERROR;
	}
	return 0;
}


void redFs_debug_print_fstab(Red_Fstab* fstab){
	printf("RedFs id: %d %d %d\n", fstab->redfs_id[0], fstab->redfs_id[1],fstab->redfs_id[2]);
	printf("Partition name: %s\n", fstab->partition_name);
	printf("RedFs version: %d\n", fstab->version);
	printf("Free memory blocks: %d\n", fstab->free_blocks);
	printf("Max block available: %d\n", fstab->block_limit);
	printf("Partiton id (partition table related): %d\n", fstab->partition_id);
	printf("Entry point node address: 0x%x\n", fstab->entry_point);
	printf("Block states:\n");
	for(uint32_t i=0;i<BLOCK_COUNT;i++){
		if(fstab->block_state[i] == ACTIVE_BLOCK){
			printf("-> Active block [%d], node allocated: %d, fragment map: 0x%x\n", i, fstab->raw_block_ptr[i].node_count, fstab->raw_block_ptr[i].fragment_map);
		}
		if(fstab->block_state[i] == FULL_BLOCK){
			printf("-> Full block [%d], node allocated: %d, fragment map: 0x%x\n", i, fstab->raw_block_ptr[i].node_count, fstab->raw_block_ptr[i].fragment_map);
		}
		if(fstab->block_state[i] == RESERVED_BLOCK){
			printf("-> Reserved block [%d], node allocated: %d, fragment map: 0x%x\n", i, fstab->raw_block_ptr[i].node_count, fstab->raw_block_ptr[i].fragment_map);
		}
	}
}


void redFs_print_fstab(uint32_t partition_id){
	Red_ptable ptable = redFs_get_partition_table();
	for(uint32_t i=0;i<ptable.partition_count;i++){
		if(ptable.partition_id[i] == partition_id){
			Red_Fstab* fstab = redFs_get_fstab(i);
			redFs_debug_print_fstab(fstab);
			return;
		}
	}
	redFs_strerror(PARTITION_NOT_FOUND_ERROR);
}

void redFs_get_partition_header(uint32_t partition_id, Red_Header* rh){
	Red_ptable ptable = redFs_get_partition_table();
	bool end = false;
	for(uint32_t i=0;i<ptable.partition_count && !end;i++){
		if(ptable.partition_id[i] == partition_id){
			for(uint32_t j=0;j<sizeof(Red_Fstab); j++){
				if(redFs_disk_action_read(ptable.partition_list[i]+j, ((uint8_t*)&rh->fstab+j))){
					redFs_strerror(FSTAB_READ_ERROR);
					return;
				}
			}
			rh->partition_address = ptable.partition_list[i];
			rh->root = rh->fstab.entry_point;
			rh->current_node = rh->fstab.entry_point;
			for(uint32_t j=0;j<rh->fstab.block_limit;j++){
				if(rh->fstab.block_state[j] == FULL_BLOCK){
					rh->used_space += 1;
				}
			}
			rh->used_space *= BLOCK_SIZE;
			for(uint32_t j=0;j<rh->fstab.block_limit;j++){
				if(rh->fstab.block_state[j] == RESERVED_BLOCK){
					rh->reserved_space += 1;
				}
			}
			rh->reserved_space *= BLOCK_SIZE;
			rh->cache_timing = 0;
			rh->cache_limit = CACHE_TIME_LIMIT;
			end = true;
		}
	}
	return;
}

void redFs_print_partition_header(Red_Header* rh){
	printf("Used space: %d bytes / %.2f Mb\n", rh->used_space, (double)rh->used_space/1000000);
	printf("Reserved space: %d bytes / %.2f Mb\n", rh->reserved_space, (double)rh->reserved_space/1000000);
	printf("Partition address: 0x%x\n",rh->partition_address);
	printf("Root node: 0x%x\n", rh->root);
	printf("Current node: 0x%x\n\n", rh->current_node);
	printf("Partition table: \n");
	printf("- RedFs id: %d %d %d\n", rh->fstab.redfs_id[0], rh->fstab.redfs_id[1],rh->fstab.redfs_id[2]);
	printf("- Partition name: %s\n", rh->fstab.partition_name);
	printf("- RedFs version: %d\n", rh->fstab.version);
	printf("- Free pages count: %d\n", rh->fstab.free_blocks);
	printf("- Page limit: %d\n", rh->fstab.block_limit);
	printf("- Partiton id (partition table related): %d\n", rh->fstab.partition_id);
	printf("- Entry point node address: 0x%x\n", rh->fstab.entry_point);
}

void redFs_print_ptable(){
	Red_ptable ptable = redFs_get_partition_table();
	printf("Max disk size: %u\n", ptable.max_disk_size);
	printf("Number of partition: %d\n", ptable.partition_count);
	printf("Pointer table: \n");
	for(uint32_t i=0;i<ptable.partition_count;i++){
		printf("-> block [%d]: partition id %d, located at  0x%x, of size %d / %.2f Mb\n",i, ptable.partition_id[i], ptable.partition_list[i], ptable.partition_size[i], (double)ptable.partition_size[i]/1000000);
	}
}

void redFs_print_fragmentation_report(Red_Fstab* fstab){
	printf("RedFs fragmentation report\n");
	for(uint32_t i=0;i<fstab->block_limit;i++){
		printf("Memory block %d at 0x%x:  ->  ", i, fstab->raw_block_ptr[i].base_ptr);
		for(uint32_t j=0;j<32;j++){
			int fstate = fstab->raw_block_ptr[i].fragment_map >> j & 0x01;
			if(fstate == 1){
				printf("\e[0m[\e[40m\e[1;91m#\e[0m]");
			}else{
				printf("\e[0m[\e[40m\e[1;92m#\e[0m]");
			}
			printf("\e[0m");
		}
		printf("\n");
	}
}



void redFs_strerror(int return_state){
	switch(return_state){
		case NOERROR:
			fprintf(stderr,"No error reported during operation\n");
			break;
		case PARTITION_TABLE_FORMAT_ERROR: 
			fprintf(stderr,"Error: Unable to format the partition table\n");
			break;
		case BOOT_SECTOR_WRITING_ERROR: 
			fprintf(stderr,"Error: Unable to write the boot sector\n");
			break;
		case PARTITION_TABLE_WRITE_ERROR: 
			fprintf(stderr,"Error: Partition table writing error, cannot update partition table\n");
			break;
		case PARTITION_TABLE_READ_ERROR: 
			fprintf(stderr,"Error: Partition table writing error, cannot get partition table\n");
			break;
		case PARTITION_NOT_FOUND_ERROR: 
			fprintf(stderr,"Error: Unable to find the specified partition\n");
			break;
		case NOT_ENOUGH_DISK_SPACE_ERROR: 
			fprintf(stderr,"Error: Not enough disk space available\n");
			break;
		case FSTAB_READ_ERROR: 
			fprintf(stderr,"Error: Unable to read fstab\n");
			break;
		case FSTAB_WRITE_ERROR: 
			fprintf(stderr,"Error: Unable to write fstab\n");
			break;
		case FSTAB_PAGE_WRITE_ERROR: 
			fprintf(stderr,"Error: Cannot write partition page due to a write error\n");
			break;
		case PARTITION_FORMAT_DISK_ERROR: 
			fprintf(stderr,"Error: Unable to format partition due to a disk error\n");
			break;
		case PARTITION_SIZE_NOT_SUFFICIENT:
			fprintf(stderr,"Error: The specified partition size is not sufficient to store even the fstab\n");
			break;
		case PARTITION_ACTION_UNKNOWN: 
			fprintf(stderr,"Partition action error: the required action could not be performed since it doesn't exist or it's still under development\n");
			break;
		case PARTITION_NODE_WRITING_ERROR:
			fprintf(stderr,"Error: unable to allocate new node for this partition\n");
			break;
		case PARTITION_NODE_READING_ERROR:
			fprintf(stderr,"Error: unable to read node or node content from this partition\n");
			break;
		case REDFS_UNSUPPORTED_FUNCTION:
			fprintf(stderr,"Error: function not supported\n");
			break;
		case REDFS_BLOCK_FRAGMENT_ERROR:
			fprintf(stderr,"Error while trying to read the block fragment map\n");
			break;
		case NODE_ALLOCATION_ERROR: 
			fprintf(stderr,"Could not allocate node due to a disk error\n");
			break;
		case NODE_DEALLOCATION_ERROR:
			fprintf(stderr, "Could not deallocate node due to a disk error\n");
			break;
		case NODE_NOT_FOUND:
			fprintf(stderr,"Could not locate the specified node\n");
			break;
		case NODE_IS_NOT_A_FOLDER_ERROR:
			fprintf(stderr, "The specified node is not a folder, cannot search for folders inside this node\n");
			break;
		case NODE_RECURSIVE_DEALLOCATION_ERROR:
			fprintf(stderr, "Recursive deallocation failed\n");
			break;
		case FOLDER_NOT_FOUND_ERROR:
			fprintf(stderr, "Error: no such folder\n");
			break;
		case FILE_ALLOCATION_ERROR:
			fprintf(stderr, "Unable to create file in the current directory due to a partition error\n");
			break;
		case FILE_NOT_FOUND_ERROR:
			fprintf(stderr, "File not found\n");
			break;
		case FILE_POINTER_ERROR:
			fprintf(stderr, "File pointer error: unable to read the complete file from the filesystem\n");
			break;
		case FILE_TOO_SMALL_ERROR:
			fprintf(stderr, "Cannot read the specified size: file is smaller\n");
			break;
		case FILE_DEALLOCATION_ERROR:
			fprintf(stderr, "Cannot remove/deallocate file\n");
			break;
		default: 
			fprintf(stderr,"Error: Unknown error\n");
			break;
	}
}

int redFs_sync_partition(Red_Header* header){
	if(header->cache_timing == 0) return 0;
	for(uint32_t i=0;i<sizeof(Red_Fstab);i++){
		if(redFs_disk_action_write(header->partition_address+i, *((uint8_t*)&header->fstab+i))){
			return (int)FSTAB_WRITE_ERROR;
		}
	}
	header->cache_timing = 0;
	return 0;
}

int redFs_cache_update(Red_Header *header){
	header->cache_timing += 1;
	int ret = 0;
	if(header->cache_timing > header->cache_limit){
		 ret = redFs_sync_partition(header);
	}
	return ret;
}
