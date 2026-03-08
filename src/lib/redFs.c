#define REDFS_IMP

#include "redFs.h"


int redfs_disk_action_write(RED_PTR address, uint8_t data){
	/* include implementation */
	(void)address;
	(void)data;
	return 0;
}

int redfs_disk_action_read(RED_PTR address, uint8_t* data){
	/* include implementation */
	(void)address;
	(void)data;
	return 0;
}

int redFs_format_partition_table(uint32_t max_disk_size){
	Red_ptable ptable = {0};
	ptable.max_disk_size = max_disk_size;
	int size = sizeof(Red_ptable);
	int offset = BOOT_SECTOR_SIZE;
	for(int i=0;i<size;i++){
		if(redfs_disk_action_write(offset+i, *((uint8_t*)&ptable+i))){
			return (int)PARTITION_TABLE_FORMAT_ERROR;
		}
	}
	return 0;
}

int redFs_write_boot_sector(uint8_t*content, uint32_t len){
	for(uint32_t i=0;i<len && len < BOOT_SECTOR_SIZE;i++){
		if(redfs_disk_action_write(i, content[i])){
			return (int)BOOT_SECTOR_WRITING_ERROR;
		}
	}
	return 0;
}

int redFs_update_partition_table(uint32_t p_fstab_adr, uint8_t partition_number){
	Red_ptable ptable = redFs_get_partition_table();
	if(partition_number > ptable.partition_count){
		return (int)PARTITION_NOT_FOUND_ERROR;
	}
	ptable.partition_list[partition_number] = p_fstab_adr;
	if(redFs_rewrite_partition_table(ptable)){
		return (int)PARTITION_TABLE_WRITING_ERROR;
	}
	return 0;
}


int redFs_push_on_partition_table(uint32_t p_fstab_adr){
	Red_ptable ptable = redFs_get_partition_table();
	ptable.partition_list[ptable.partition_count] = p_fstab_adr;
	ptable.partition_count+=1;
	if(redFs_rewrite_partition_table(ptable)){
		return (int)PARTITION_TABLE_WRITING_ERROR;
	}
	return 0;
}

Red_ptable redFs_get_partition_table(){
	Red_ptable p = {0};
	uint32_t size = sizeof(Red_ptable);
	int offset = BOOT_SECTOR_SIZE;
	for(uint32_t i=0;i<size;i++){
		uint8_t buffer;
		redfs_disk_action_read(offset+i, &buffer);
		*((uint8_t*)&p+i) = buffer;
	}
	return p;
}

int redFs_rewrite_partition_table(Red_ptable new_ptable){
	int size = sizeof(Red_ptable);
	int offset = BOOT_SECTOR_SIZE;
	for(int i=0;i<size;i++){
		if(redfs_disk_action_write(offset+i, *((uint8_t*)&new_ptable+i))){
			return (int)PARTITION_TABLE_FORMAT_ERROR;
		}
	}
	return 0;
}

RED_PTR redFs_caclulate_new_partition_offset(){
	Red_ptable ptable = redFs_get_partition_table();
	if(ptable.partition_count < 1) return 0;
	RED_PTR offset = ptable.partition_size[ptable.partition_count] + ptable.partition_list[ptable.partition_count];
	offset += PARTITION_BLANK_OFFSET;
	return offset;
}

uint32_t redFs_get_new_partition_id(){
	Red_ptable ptable = redFs_get_partition_table();
	return ptable.partition_count+1000;
}

Red_Fstab redFs_define_fstab(char* partition_name, uint32_t partition_size, uint32_t starting_point){
	Red_Fstab fstab = {0};
	fstab.redfs_id[0] = REDFS_ID_PREFIX;
	fstab.redfs_id[1] = REDFS_ID;
	fstab.redfs_id[2] = REDFS_SUFFIX;
	strcpy(fstab.partition_name, partition_name);
	fstab.version = REDFS_VERSION;
	fstab.partition_id = redFs_get_new_partition_id();
	Red_ptable ptable = redFs_get_partition_table();
	if(starting_point+partition_size > ptable.max_disk_size){
		Red_Fstab abort_fstab = {0};
		return abort_fstab;
	}
	for(uint32_t i=0;i<(uint32_t)(partition_size/NODE_SIZE);i++){
		fstab.raw_ptr_table[i] = starting_point + i*NODE_SIZE;
	}
	for(uint32_t i=0;i<(uint32_t)(partition_size/NODE_SIZE);i++){
		fstab.ptr_table_state[i] = PAGE_FREE;
	}
	fstab.free_pages = (uint32_t)(partition_size/NODE_SIZE);
	fstab.file_count = 0;
	fstab.entry_point = 0;
	return fstab;
}

Red_Fstab redFs_get_fstab(uint8_t partition_number){
	Red_Fstab fstab = {0};
	Red_ptable ptable = redFs_get_partition_table();
	if(partition_number > ptable.partition_count){
		return fstab;
	}
	RED_PTR address = ptable.partition_list[partition_number];

	for(uint32_t i=0;i<sizeof(Red_Fstab);i++){
		uint8_t buffer = 0;
		if(redfs_disk_action_read(address+i, &buffer)){
			Red_Fstab dfstab = {0};
			return dfstab;
		}
		*((uint8_t*)&fstab+i) = buffer;
	}
	return fstab;
}


int redFs_update_fdtab(Red_Fstab fstab, uint8_t partition_number){
	Red_ptable ptable = redFs_get_partition_table();
	if(partition_number > ptable.partition_count){
		return (int)PARTITION_NOT_FOUND_ERROR;
	}
	RED_PTR address = ptable.partition_list[partition_number];

	for(uint32_t i=0;i<sizeof(Red_Fstab);i++){
		if(redfs_disk_action_write(address+i, *((uint8_t*)&fstab+i))){
			return (int)FSTAB_WRITE_ERROR;
		}
	}
	return 0;
}


int redFs_format_partition(char* partition_name, uint32_t partition_size, uint32_t starting_point){

	/* 
	 * a preallocated address inside the partition table is expected to be found at the time 
	 * of calling this function, if that's not the case you must first calculate the offset for 
	 * the partition with the help of the "redFs_caclulate_new_partition_offset()" function and 
	 * manually update the partition table with "redFs_push_on_partition_table(uint32_t p_fstab_adr)" 
	 * or similar functions provided by the API. It's suggested to use the wrapper "redFs_create_partition()"
	 * which handle all the necessary task, including this function call
	 *
	 * */

	Red_Fstab fstab = redFs_define_fstab(partition_name, partition_size, starting_point);
	if(fstab.redfs_id[0] == 0){
		return (int)NOT_ENOUGH_DISK_SPACE_ERROR;
	}

	Red_Node entry_point = {0};
	entry_point.type = PAGE_IS_FOLDER;
	strcpy(entry_point.name, "root");
	entry_point.permissions = PAGE_DEF_PERMISSION;
	entry_point.chained = false;
	entry_point.prev_page = 0;
	entry_point.next_page = 0;
	memset(entry_point.content, 0x00, NODE_ARRAY_LIMIT/sizeof(RED_PTR));
	
	uint32_t i=0;
	while(fstab.ptr_table_state[i] != PAGE_FREE){i++;}
	RED_PTR node_adr = fstab.raw_ptr_table[i];
	for(i=0;i<sizeof(Red_Node);i++){
		if(redfs_disk_action_write(node_adr+i, *((uint8_t*)&entry_point+i))){
			return (int)FSTAB_PAGE_WRITE_ERROR;
		}
	}
	fstab.ptr_table_state[i] = PAGE_ALLOCATED;
	fstab.free_pages -= 1;
	fstab.entry_point = node_adr;
	
	for(i=0;i<sizeof(Red_Fstab);i++){
		if(redfs_disk_action_write(node_adr+i, *((uint8_t*)&fstab+i))){
			return (int)FSTAB_WRITE_ERROR;
		}	
	}

	return 0;
}

int redFs_create_partition(char* name, uint32_t size){

	return 0;
}


