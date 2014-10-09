#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * [X] Run executables without arguments (10)
 * [X] Run executables with arguments (10)
 * [ ] set for HOME and PATH work properly (5)
 * [?] exit and quit work properly (5)
 * [ ] cd (with and without arguments) works properly (5)
 * [ ] PATH works properly. Give error messages when the executable is not found (10)
 * [ ] Child processes inherit the environment (5)
 * [ ] Allow background/foreground execution (&) (5)
 * [ ] Printing/reporting of background processes, (including the jobs command) (10)
 * [ ] Allow file redirection (> and <) (5)
 * [ ] Allow (1) pipe (|) (10)
 * [ ] Supports reading commands from prompt and from file (10)
 */

#define MAX_LENGTH 1024
#define MAX_JOBS 10
#define MAX_ARGS 10
#define DELIMS " \t\r\n"

int idcount = 1;

struct Job {
	int id, argNum;
	char *args[MAX_ARGS];

	Job() {
		id = 0;
		argNum = 0;
		for(int i = 0; i < MAX_ARGS; i++){
			args[i] = new char[256];
		}
	}

	~Job() {
		for(int i = 0; i < MAX_ARGS; i++){
			delete[] args[i];
		}
	}
};

int parse(char* line, Job* job1) {
	// Split 'line' on spaces to separate args
	
	char* thisArg;
	thisArg = strtok(line," ");
	argCount = 0;
	while(thisArg != NULL) {
		job1->args[argCount] = thisArg;
		argCount++;
		thisArg = strtok(line," ");
	}

	// Add args to job1 struct one by one. First arg is executable, others are parameters
/*	argcount = 0;	
	while(separator != NULL){
		job1->args[argcount] = substring(line,0,separator);
		line = substring(line, separator + 1, line[strlen(line)-(separator + 1)]);
		separator = strchr(line,' ');
		argcount++;
	}
*/
	job1->argNum = argCount;	

	// Return number of jobs
	return 1;
}

int execute(Job* job1) {

	int exitbit = 0;	

	// Make system call to execute job1 using args.
	if (strcmp(job1->args[0],"exit") == 0 || strcmp(job1->args[0],"quit") == 0) {
		exitbit = 1;
	} else {
		char* cmd = "";
		for(i = 0; i < job1->argNum; i++){
			cmd = strcat(strcat(cmd, " "), job1->args[i]);
		}
		system(cmd);
	}

	// Return 0 to continue and 1 to exit. 
	return exitbit;
}

int main(int argc, char **argv, char **envp) {
	char *cmd;
	char line[MAX_LENGTH];
	int exitbit = 0;
	int numJobs = 0;

	while(exitbit == 0) {
		printf("$ ");
		if(!fgets(line, MAX_LENGTH, stdin)){ //fgets => reads all input in string into 'line'
			break;
 		}

		Job* job1;
		// Turns input line into job
		parse(line, job1);
		

		// Execute job
   		exitbit = execute(job1);
	}
	
	return 0;
}
