#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include "redFs.h"

#define LOCAL_SIZE 1024*8
static char local_mem[LOCAL_SIZE] =  {0};
static size_t local_tracker =	0;
static size_t local_size =		LOCAL_SIZE;

char* path = NULL;

typedef struct{
	char** array;
	size_t tracker;
	size_t size;
}Cmd;

typedef struct{
	pid_t pid;
	int*  ret_status; 
}Process;

void* local_alloc(size_t size){
	if(size+local_tracker >= local_size) local_tracker = 0;
	void* ptr = &local_mem[local_tracker];
	local_tracker += size;
	if(local_tracker >= local_size) local_tracker = 0;
	return ptr;
}
void cmd_append(Cmd* cmd, char* string){
	if(cmd->array == NULL){
		cmd->array = (char**)local_alloc(sizeof(char*)*32);
		cmd->size = 32;
		cmd->tracker = 0;
	}
	cmd->array[cmd->tracker] = string;
	cmd->tracker += 1;
	if(cmd->tracker >= cmd->size){
		char** old = cmd->array;
		cmd->array = (char**)local_alloc(sizeof(char*)*cmd->size*2);
		cmd->size *= 2;
		for(size_t i=0;i<cmd->tracker; i++){
			cmd->array[i] = old[i];
		}
	}
}

pid_t spawn_process(Cmd* cmd){
	printf("[CMD]: [");
	for(size_t i=0;i<cmd->tracker; i++){
		printf("%s, ", cmd->array[i]);
	}
	printf("NULL]\n");
	pid_t pid = fork();
	if(pid < 0){
		abort();
	}
	if(pid > 0){
		return pid;
	}else{
		if(execv(path, cmd->array) < 0){
			fprintf(stderr, "Unable to spawn process: %s\n", strerror(errno));
			exit(1);
		}
	}
	return 0;
}

static void capture_return(Process* process){
	int loc_ret = -69;
	waitpid(process->pid, &loc_ret, 0);
	*(process->ret_status) = WEXITSTATUS(loc_ret);
}

int* spawn_list_synch_wait(Cmd* cmd, int len){
	pid_t* pid = (pid_t*)local_alloc(sizeof(pid_t)*len);
	pthread_t* monitor = (pthread_t*)local_alloc(sizeof(pthread_t)*len);
	int *loc_ret = (int*)local_alloc(sizeof(int)*len);
	for(int i=0;i<len; i++){
		pid_t proc = spawn_process(&cmd[i]);
		if(proc > 0){
			pid[i] = proc;
			Process* process = (Process*)local_alloc(sizeof(Process));
			process->pid = pid[i];
			process->ret_status = &loc_ret[i];
			if(pthread_create(&monitor[i], NULL, (void* _Nullable)&capture_return, process)){
				fprintf(stderr, "Unable to create thread: %s\n", strerror(errno));
			}
		}
	}
	bool end = false;
	while(!end){
		end = true;
		sleep(1);
		for(int i=0;i<len; i++){
			if(loc_ret[i] == -69){
				end = false;
			}
		}
	}
	return loc_ret;
}

int main(){
	printf("RedFs test program.\n");
	FILE* stream = popen("echo $PWD", "r");
	fseek(stream, 0, SEEK_END);
	int size = ftell(stream);
	fseek(stream, 0, SEEK_SET);
	path = (char*)malloc(sizeof(char)*size+5);
	fread(path, sizeof(char), size, stream);
	path[size] = '\0';
	*(strchr(path, '\n')) = '\0';
	strcat(path, "/bin");
	fclose(stream);
	printf("TEST EXECUTION PATH: %s\n", path);
	#define CMD_LEN 1
	Cmd cmd[CMD_LEN] = {0};	
	cmd_append(&cmd[0], "format");
	cmd_append(&cmd[0], "FORMAT_DISK");
	cmd_append(&cmd[0], "20896200");
	cmd_append(&cmd[0], "4890200");

	int *loc_ret = spawn_list_synch_wait(cmd, CMD_LEN);

	printf("Testing completed: ");
	int failed = 0;
	for(int i=0;i<CMD_LEN;i++){
		printf("%s -> %d\n", cmd[i].array[0], loc_ret[i]);
		redFs_strerror(loc_ret[i]);
		if(loc_ret[i] != 0) failed += 1;
	}
	if(failed){
		printf("%d test failed\n", failed);
	}else{
		printf("Testing completed with 0 error reported\n");
	}
	free(path);
	path = NULL;
	return 0;
}
