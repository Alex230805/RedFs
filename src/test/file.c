#define VIRTIO
#include "redFs.h"
#include <stdio.h>
#include <stdlib.h>
#include "unistd.h"

#define NOTY(content) printf("[FILE TEST]: "content"\n")
#define NOTYF(content, ...) printf("[FILE TEST]: "content"\n", __VA_ARGS__);

#define LOCAL_SIZE 1024*8
static char local_mem[LOCAL_SIZE] =  {0};
static size_t local_tracker =	0;
static size_t local_size =		LOCAL_SIZE;

void* local_alloc(size_t size){
	if(size+local_tracker >= local_size) local_tracker = 0;
	void* ptr = &local_mem[local_tracker];
	local_tracker += size;
	if(local_tracker >= local_size) local_tracker = 0;
	return ptr;
}

int main(int argc, char** argv){
	int ret = 0;
	NOTY("Testing folder creation and directory navigation");
	if(argc < 3) {
		NOTY("Not enough argument to begin the testing sequence: file <virtual_disk> <disk_size> <file_name>");
		return 1;
	}
	char* disk_name = argv[1];
	int size = atoi(argv[2]);
	char* file_name = argv[3];
	NOTY("Initializing test disk and partition");
	redFs_open_static_virtual_memory(disk_name);
	redFs_init_disk(size);
	ret = redFs_create_partition("test", 0x2FFFFF);
	if(ret) return ret;
	uint32_t partition_id = redFs_get_partition_id_from_name("test");
	if(!partition_id) return PARTITION_NOT_FOUND_ERROR;

	Red_Header header = {0};
	redFs_get_partition_header(partition_id, &header);
	ret = redFs_partition_header_sanity_check(&header);
	if(ret) return ret;

	NOTYF("Opening test file '%s'", file_name);
	FILE* f = fopen(file_name, "r");
	if(!f){
		fprintf(stderr, "Unable to open test file: '%s'\n", strerror(errno));
		exit(errno);
	}
	fseek(f, 0, SEEK_END);
	int file_size = ftell(f);
	fseek(f, 0, SEEK_SET);
	
	char* file_buffer = (char*)local_alloc(sizeof(char)*file_size+1);
	fread(file_buffer, sizeof(char), file_size, f);
	file_buffer[file_size] = '\0';	
	fclose(f);
	NOTY("Done!");

	NOTY("Creating empty file inside the partition");
	printf("Writing down the test file\n");
	ret =  redFs_touch_file(&header, "./file_test.txt", 0);
	if(ret) return ret;
	NOTY("Current dir content");
	ret = redFs_print_dir_content(&header, "./");
	usleep(220000);

	NOTYF("Writing down test file '%s', length of %d bytes", file_name,file_size);
	ret = redFs_write_file(&header, "./file_test.txt", (uint8_t*)file_buffer, sizeof(char)*file_size);
	if(ret) return ret;
	NOTY("Done!");
	NOTY("Current dir content");
	ret = redFs_print_dir_content(&header, "./");
	if(ret) return ret;
	usleep(220000);

	NOTYF("File size from the file system: %d", redFs_get_file_size(&header, "./file_test.txt"));
	NOTY("Printing fragmentation report");
	redFs_print_fragmentation_report(&header.fstab);
	
	int read_size = 512;
	NOTYF("Reading %d bytes from the selected file", read_size);
	file_buffer[0] = '\0';
	memset(file_buffer, 0, file_size);
	ret = redFs_read_file(&header, "./file_test.txt", (uint8_t*)file_buffer, read_size);
	if(ret) return ret;
	NOTYF("Content from the fetched buffer of length %d", read_size);
	printf("'''\n\n%s\n\n'''\n", file_buffer);
	usleep(520000);

	NOTY("Testing deallocation, try writing down the fetched buffer and then try to remove it");
	ret =  redFs_touch_file(&header, "./buffer", 0);
	if(ret) return ret;
	NOTY("Current dir content");
	ret = redFs_print_dir_content(&header, "./");
	if(ret) return ret;
	usleep(220000);

	file_buffer[read_size] = '\0';
	ret = redFs_write_file(&header, "./buffer", (uint8_t*)file_buffer, sizeof(char)*read_size);
	if(ret) return ret;
	NOTYF("Written file size: %d", redFs_get_file_size(&header, "./buffer"));
	NOTY("Current dir content");
	ret = redFs_print_dir_content(&header, "./");
	if(ret) return ret;
	usleep(220000);
	NOTY("Printing fragmentation report");
	redFs_print_fragmentation_report(&header.fstab);

	NOTY("FIle added, now try to remove it");
	ret = redFs_remove_file(&header, "./buffer");
	NOTY("Done");
	NOTY("Printing fragmentation report");
	redFs_print_fragmentation_report(&header.fstab);
	NOTY("Showing dir content");
	ret = redFs_print_dir_content(&header, "./");
	if(ret) return ret;
	usleep(220000);
	
	NOTY("Try to remove it again to test file detection");
	ret = redFs_remove_file(&header, "./buffer");
	if(!ret) return ret;
	NOTY("Done");
	redFs_close_static_virtual_memory();
	NOTY("Test completed");
	return 0;
}
