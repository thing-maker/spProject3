#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <dirent.h>
#include <errno.h>
#include "shellFunc.h"

char *get_current_dir_name();

char *ListOfCommands[6] = {"bg", "cd", "exit", "fg", "jobs", "kill"};

//Run a suspended job in background
void background(char **argv){
	//argv[0] should be the program name
	//argv should be the array of arguments for the program
	//jobId will be used to look up information above
	char *job = argv[1];
	int id;
	if(job[0] == '%') {
		sscanf(job, "%%%i", &id);
		id = getPid(id);
		if(id == 0) {
			printf("bg: %s: no such job\n", job);
			return;
		}
	} else {
		sscanf(job, "%i", &id);
		id = findCommand(id);
		if(id == 0) {
			printf("bg: (%s) - no such process\n", job);
			return;
		}
	}
	childContinues(id, 1);
	kill(id, SIGCONT);
	
}

//Change directory
int changeDir(char **argv){
	char *path = argv[1];
	//printf("Beginning: %s\n", getenv("PWD"));
	
	//Given null path
	if(path == NULL){
		if(chdir(getenv("HOME")) == -1){
			printf("%s : %s\n", path, strerror(errno));
			return 0;
		}
		setenv("PWD", getenv("HOME"), 1);
	}
	
	//Given absolute path
	else if(path[0] == '/'){
		if(chdir(path) == -1){
			printf("%s : %s\n", path, strerror(errno));
			return 0;
		}
		setenv("PWD", path, 1);
	}
	
	//Given relative path
	else if(path[0] == '.'){
		if(chdir(path) == -1){
			printf("%s : %s\n", path, strerror(errno));
			return 0;
		}

    		char *myDir = get_current_dir_name();
		setenv("PWD", myDir, 1);
    		free(myDir);

	}
	
	//Given a directory
	else if(path != NULL){
		DIR* dir = opendir(path);
		if (dir) {
	    		//Directory exists.
	    		closedir(dir);
	    		if(chdir(path) == -1){
				printf("%s : %s\n", path, strerror(errno));
				return 0;
			}
			char *newDir = getenv("PWD");
			char *newNew = malloc(strlen(newDir) + strlen(path) + 2);
			strcpy(newNew, newDir);
			strcat(newNew, "/");
			strcat(newNew, path);
			setenv("PWD", newNew, 1);
			free(newNew);
	    	}
		else{
			printf("%s: No such directory\n", path);
		}
	}
	//printf("End: %s\n", getenv("PWD"));
	return 0;
}	

//Exit shell
void leave(){
	sighupIt();
	sigcontIt();
}

//Run a suspended or background job in the foreground
void foreground(char **argv){
	char *job = argv[1];
	int id;
	if(job[0] == '%') {
		sscanf(job, "%%%i", &id);
		id = getPid(id);
		if(id == 0) {
			printf("fg: %s: no such job\n", job);
			return;
		}
	} else {
		sscanf(job, "%i", &id);
		id = findCommand(id);
		if(id == 0) {
			printf("fg: (%s) - no such process\n", job);
			return;
		}
	}
	childContinues(id, 0);
	kill(id, SIGCONT);
	runningAgain(id);
}

//List jobs with job id, process id, current status, and command
void jobs(){
	printChildren();
}


//Send SIGTERM to the given job
void murder(char **argv){
	//Might need to change status in childList?
	char *job = argv[1];
	int id;
	if(job[0] == '%') {
		sscanf(job, "%%%i", &id);
		id = getPid(id);
		if(id == 0) {
			printf("kill: %s: no such job\n", job);
			return;
		}
	} else {
		sscanf(job, "%i", &id);
		id = findCommand(id);
		if(id == 0) {
			printf("kill: (%s) - no such process\n", job);
			return;
		}
	}

	kill(id, SIGTERM);
	childKilled(id, SIGTERM);
}

//Find a builtin
int lookup(char** cmd){
	if(cmd == NULL) return 1;
	if(strcmp(ListOfCommands[0], cmd[0]) == 0){
		background(cmd);
		return 1;
	} if(strcmp(ListOfCommands[1], cmd[0]) == 0){
		changeDir(cmd);
		return 1;
	} if(strcmp(ListOfCommands[2], cmd[0]) == 0){
		leave();
		return 0;
	} if(strcmp(ListOfCommands[3], cmd[0]) == 0){
		foreground(cmd);
		return 1;
	} if(strcmp(ListOfCommands[4], cmd[0]) == 0){
		jobs();
		return 1;
	} if(strcmp(ListOfCommands[5], cmd[0]) == 0){
		murder(cmd);
		return 1;
	}

	return 0;
}
