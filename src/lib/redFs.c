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

RED_PTR redFs_caclulate_new_partition_offset(){
	Red_ptable ptable = redFs_get_partition_table();
	if(ptable.partition_count < 1) return sizeof(Red_ptable)+BOOT_SECTOR_SIZE+PARTITION_BLANK_OFFSET;
	RED_PTR offset = ptable.partition_size[ptable.partition_count-1] + ptable.partition_list[ptable.partition_count-1];
	offset += PARTITION_BLANK_OFFSET;
	return offset;
}

uint32_t redFs_generate_partition_id(){
	Red_ptable ptable = redFs_get_partition_table();
	return ptable.partition_count+1000;
}

int redFs_define_fstab(char* partition_name, uint32_t partition_size, uint32_t starting_point, Red_Fstab* fstab){
	fstab->redfs_id[0] = REDFS_ID_PREFIX;
	fstab->redfs_id[1] = REDFS_ID;
	fstab->redfs_id[2] = REDFS_SUFFIX;
	memcpy(fstab->partition_name, partition_name, sizeof(char)*STRING_LIMIT);
	fstab->version = REDFS_VERSION;
	fstab->partition_id = redFs_generate_partition_id();
	Red_ptable ptable = redFs_get_partition_table();
	if(starting_point+partition_size > ptable.max_disk_size){
		return 1;
	}
	
	for(uint32_t i=0;i<(uint32_t)(partition_size/NODE_SIZE);i++){
		fstab->raw_ptr_table[i] = starting_point + i*NODE_SIZE;
	}
	for(uint32_t i=0;i<(uint32_t)(partition_size/NODE_SIZE);i++){
		fstab->ptr_table_state[i] = PAGE_FREE;
	}
	
	for(uint32_t i=0;i<(uint32_t)(sizeof(Red_Fstab)/NODE_SIZE);i++){
		fstab->ptr_table_state[i] = PAGE_ALLOCATED;
	}

	fstab->free_pages = (uint32_t)(partition_size/NODE_SIZE) - (uint32_t)(sizeof(Red_Fstab)/NODE_SIZE);
	fstab->page_limit = (uint32_t)(partition_size/NODE_SIZE);
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
	memset(entry_point.content, 0x00, NODE_ARRAY_LIMIT/sizeof(RED_PTR));
	// locate and write entry point node 
	uint32_t i=0;
	while(fstab->ptr_table_state[i] != PAGE_FREE && i < fstab->page_limit){i+=1;}
	RED_PTR node_adr = fstab->raw_ptr_table[i];

	for(i=0;i<sizeof(Red_Node);i++){
		if(redFs_disk_action_write(node_adr+i, *((uint8_t*)&entry_point+i))){
			return (int)FSTAB_PAGE_WRITE_ERROR;
		}
	}


	fstab->ptr_table_state[i] = PAGE_ALLOCATED;
	fstab->free_pages -= 1;
	fstab->entry_point = node_adr;

	// save fstab->inside the drive
	for(i=0;i<sizeof(Red_Fstab);i++){
		if(redFs_disk_action_write(starting_point+i, *((uint8_t*)fstab+i))){
			return (int)FSTAB_WRITE_ERROR;
		}
	}

	//redFs_debug_print_fstab(fstab);
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
	RED_PTR offset = redFs_caclulate_new_partition_offset();
	
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

	return 0;
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
	printf("Free pages count: %d\n", fstab->free_pages);
	printf("Page limit: %d\n", fstab->page_limit);
	printf("Partiton id (partition table related): %d\n", fstab->partition_id);
	printf("Entry point node address: 0x%x\n", fstab->entry_point);
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

void redFs_print_ptable(){
	Red_ptable ptable = redFs_get_partition_table();
	printf("Max disk size: %u\n", ptable.max_disk_size);
	printf("Number of partition: %d\n", ptable.partition_count);
	printf("Pointer table: \n");
	for(uint32_t i=0;i<ptable.partition_count;i++){
		printf("-> block [%d]: partition id %d, located at  0x%x, of size %d\n",i, ptable.partition_id[i], ptable.partition_list[i], ptable.partition_size[i]);
	}
}


void redFs_strerror(int return_state){
	switch(return_state){
		case NOERROR:
			printf("No error reported during operation\n");
			break;
		case PARTITION_TABLE_FORMAT_ERROR: 
			printf("Error: Unable to format the partition table\n");
			break;
		case BOOT_SECTOR_WRITING_ERROR: 
			printf("Error: Unable to write the boot sector\n");
			break;
		case PARTITION_TABLE_WRITE_ERROR: 
			printf("Error: Partition table writing error, cannot update partition table\n");
			break;
		case PARTITION_TABLE_READ_ERROR: 
			printf("Error: Partition table writing error, cannot get partition table\n");
			break;
		case PARTITION_NOT_FOUND_ERROR: 
			printf("Error: Unable to find the specified partition\n");
			break;
		case NOT_ENOUGH_DISK_SPACE_ERROR: 
			printf("Error: Not enough disk space available\n");
			break;
		case FSTAB_READ_ERROR: 
			printf("Error: Unable to read fstab\n");
			break;
		case FSTAB_WRITE_ERROR: 
			printf("Error: Unable to write fstab\n");
			break;
		case FSTAB_PAGE_WRITE_ERROR: 
			printf("Error: Cannot write partition page due to a write error\n");
			break;
		case PARTITION_FORMAT_DISK_ERROR: 
			printf("Error: Unable to format partition due to a disk error\n");
			break;
		case PARTITION_SIZE_NOT_SUFFICIENT:
			printf("Error: The specified partition size is not sufficient to store even the fstab\n");
			break;
		default: 
			printf("Error: Unknown error\n");
			break;
	}
}
