#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * [X] Run executables without arguments (10)
 * [X] Run executables with arguments (10)
 * [ ] set for HOME and PATH work properly (5)
 * [X] exit and quit work properly (5)
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

int parse(Job* jobs) {

	char line[MAX_LENGTH+1];

	fgets(line, MAX_LENGTH, stdin); //fgets => reads all input in string into 'line'
		
	printf("line: %s", line);	

	char* thisArg;
	thisArg = strtok(line," ");
	int argCount = 0;
	while(!(thisArg == NULL)) {
		strcpy(jobs[0].args[argCount], thisArg);
		argCount++;
		thisArg = strtok(NULL," ");
	}

	jobs[0].argNum = argCount;	

	for(int i = 0; i < jobs[0].argNum; i++){
		printf("Arg[%i] = %s\n", i, jobs[0].args[i]);
	}

	// Return number of jobs
	return 1;
}

int execute(Job* jobs) {

	int exitbit = 0;	

	printf("args[0] = %s\n", jobs[0].args[0]);
	// Make system call to execute job1 using args.
	if (strcmp(jobs[0].args[0], "exit\n") == 0 || strcmp(jobs[0].args[0], "quit\n") == 0) {
		exitbit = 1;
	} else {
		char cmd[MAX_LENGTH] = {0};
		strcat(cmd, jobs[0].args[0]);
		for(int i = 1; i < jobs[0].argNum; i++){
			strcat(strcat(cmd, " "), jobs[0].args[i]);
		}
		system(cmd);
	}

	// Return 0 to continue and 1 to exit. 
	return exitbit;
}

int main(int argc, char **argv, char **envp) {
	char* cmd;
	int exitbit = 0;
	int numJobs = 0;

	while(exitbit == 0) {
		printf("$ ");
		
		Job jobs[MAX_JOBS];
		// Turns input line into job
		parse(jobs);
		

		// Execute job
   		exitbit = execute(jobs);
	}
	
	return 0;
}
