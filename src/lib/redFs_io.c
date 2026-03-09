#define REDFS_IO_IMP

#include "redFs_io.h"

#ifdef VIRTIO

void redFs_open_static_virtual_memory(const char* name){
	fp  = fopen(name, "w+r");
	if(fp == NULL){
		fprintf(stderr, "Unable to open virtual memory due to: %s\n", strerror(errno));
		exit(errno);
	}
	return;
}

void redFs_close_static_virtual_memory(){
	fclose(fp);
	return;
}

#endif


int redFs_disk_action_write(RED_PTR address, uint8_t data, ...){
	#ifdef VIRTIO

	if(fp == NULL) return 0;
	fseek(fp, address,SEEK_SET);
	fwrite(&data, sizeof(uint8_t), 1, fp);

	#endif
	return 0;
}

int redFs_disk_action_read(RED_PTR address, uint8_t* data, ...){
	#ifdef VIRTIO

	if(fp == NULL) return 0;
	fseek(fp, address,SEEK_SET);
	fread(data, sizeof(uint8_t), 1, fp);

	#endif

	return 0;
}
