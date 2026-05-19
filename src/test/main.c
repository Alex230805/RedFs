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

#define INIT_P_RETURN 69420

#define cmd_set(cmd, ...)\
	cmd_set_imp(&cmd, (char* []){__VA_ARGS__, NULL});

#define LOCAL_SIZE 1024*8
static char local_mem[LOCAL_SIZE] =  {0};
static size_t local_tracker =	0;
static size_t local_size =		LOCAL_SIZE;

static char path[512] = {0};

typedef struct{
	char** array;
	size_t tracker;
	size_t size;
}Cmd;

typedef struct{
	pid_t pid;
	int  ret_status; 
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

char *path_compose(char* path, char* string){
	int size = strlen(path) + strlen(string);
	char* out = (char*)local_alloc(sizeof(char)*size+1);
	strcpy(out, path);
	strcat(out, string);
	return out;
}

pid_t spawn_process(Cmd* cmd){
	printf("[CMD]: [");
	for(size_t i=0;i<cmd->tracker; i++){
		if(i==0){
			printf("%s", path);
		}
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
		if(execv(path_compose(path, cmd->array[0]), cmd->array) < 0){
			fprintf(stderr, "Unable to spawn process: %s\n", strerror(errno));
			abort();
		}
	}
	return 0;
}

static void capture_return(Process* process){
	static int loc_ret = INIT_P_RETURN;
	waitpid(process->pid, &loc_ret, 0);
	process->ret_status = WEXITSTATUS(loc_ret);
}

Process* spawn_list_synch_wait(Cmd* cmd, int len){
	pthread_t* monitor = (pthread_t*)local_alloc(sizeof(pthread_t)*len);
	Process* proc = (Process*)local_alloc(sizeof(Process)*len);
	for(int i=0;i<len; i++){
		pid_t p = spawn_process(&cmd[i]);
		if(p > 0){
			proc[i].pid = p;
			proc[i].ret_status = INIT_P_RETURN;
			if(pthread_create(&monitor[i], NULL, (void*)&capture_return, &proc[i])){
				fprintf(stderr, "Unable to create thread: %s\n", strerror(errno));
			}
		}
	}
	bool end = false;
	printf("Waiting for tests to finish\n");
	while(!end){
		end = true;
		for(int i=0;i<len; i++){
			sleep(1);
			if(proc[i].ret_status == INIT_P_RETURN){
				end = false;
			}
		}
	}
	return proc;
}

void cmd_set_imp(Cmd* cmd, char* list[]){
	int i=0;
	while(list[i] != NULL){
		cmd_append(cmd, list[i]);
		i+=1;	
	}
}

int main(){
	printf("RedFs test program.\n");
	FILE* stream = popen("echo $PWD", "r");
	fseek(stream, 0, SEEK_END);
	int size = ftell(stream);
	fseek(stream, 0, SEEK_SET);
	fread(path, sizeof(char), size, stream);
	path[size] = '\0';
	*(strchr(path, '\n')) = '\0';
	strcat(path, "/bin/");
	fclose(stream);

	#define CMD_LEN 2
	Cmd cmd[CMD_LEN] = {0};
	cmd_set(cmd[0], "format", "TEST_FORMAT_IMAGE", "30896200", "2890200");
	cmd_set(cmd[1], "dir"	, "TEST_DIR_IMAGE"	 , "30896200");
	Process *proc = spawn_list_synch_wait(cmd, CMD_LEN);

	printf("Testing completed: \n");
	for(int i=0;i<CMD_LEN;i++){
		printf("[CMD %d] -> %s returned %d\n",i ,cmd[i].array[0], proc[i].ret_status);
		if(proc[i].ret_status){
			redFs_strerror(proc[i].ret_status);
		}
	}
	return 0;
}
