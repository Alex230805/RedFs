#define REDFS_IMP

#include "redFs.h"


int redFs_disk_action_write(RED_PTR address, uint8_t data){
	/* include implementation */
	(void)address;
	(void)data;
	return 0;
}

int redFs_disk_action_read(RED_PTR address, uint8_t* data){
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

int redFs_update_partition_table(uint32_t p_fstab_adr, uint32_t size, uint8_t partition_number){
	Red_ptable ptable = redFs_get_partition_table();
	if(partition_number > ptable.partition_count){
		return (int)PARTITION_NOT_FOUND_ERROR;
	}
	ptable.partition_list[partition_number] = p_fstab_adr;
	ptable.partition_size[partition_number] = size;
	if(redFs_rewrite_partition_table(ptable)){
		return (int)PARTITION_TABLE_WRITE_ERROR;
	}
	return 0;
}


int redFs_push_on_partition_table(uint32_t p_fstab_adr, uint32_t size){
	Red_ptable ptable = redFs_get_partition_table();
	ptable.partition_list[ptable.partition_count] = p_fstab_adr;
	ptable.partition_size[ptable.partition_count] = size;
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

Red_Fstab redFs_define_fstab(char* partition_name, uint32_t partition_size, uint32_t starting_point){
	Red_Fstab fstab = {0};
	fstab.redfs_id[0] = REDFS_ID_PREFIX;
	fstab.redfs_id[1] = REDFS_ID;
	fstab.redfs_id[2] = REDFS_SUFFIX;
	strcpy(fstab.partition_name, partition_name);
	fstab.version = REDFS_VERSION;
	fstab.partition_id = redFs_generate_partition_id();
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
		if(redFs_disk_action_read(address+i, &buffer)){
			Red_Fstab dfstab = {0};
			return dfstab;
		}
		*((uint8_t*)&fstab+i) = buffer;
	}
	return fstab;
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


int redFs_format_partition(char* partition_name, uint32_t partition_size, uint32_t starting_point){

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
		if(redFs_disk_action_write(node_adr+i, *((uint8_t*)&entry_point+i))){
			return (int)FSTAB_PAGE_WRITE_ERROR;
		}
	}
	fstab.ptr_table_state[i] = PAGE_ALLOCATED;
	fstab.free_pages -= 1;
	fstab.entry_point = node_adr;
	
	for(i=0;i<sizeof(Red_Fstab);i++){
		if(redFs_disk_action_write(node_adr+i, *((uint8_t*)&fstab+i))){
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
	RED_PTR offset = redFs_caclulate_new_partition_offset();
	if(redFs_push_on_partition_table(offset, size)){
		return (int)PARTITION_TABLE_WRITE_ERROR;
	}
	if(redFs_format_partition(name, size, offset)){
		if(redFs_pop_off_partition_table()){
			return (int)PARTITION_FORMAT_DISK_ERROR;
		}
	}
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
		Red_Fstab fstab = redFs_get_fstab(i);
		if(fstab.redfs_id[0] == 0){
			return (int)FSTAB_READ_ERROR;
		}
		if(fstab.partition_id == partition_id && strcmp(fstab.partition_name, name) == 0){
			/* deleting the partition entry and shifting back the remaining pointer */
			for(int j=0;j<(ptable.partition_count-i)-1;j++){
				ptable.partition_list[j] = ptable.partition_list[j+1];
				ptable.partition_size[j] = ptable.partition_list[j+1];
			}
			ptable.partition_count -= 1;
			end = true;
		}
	}
	return 0;
}
