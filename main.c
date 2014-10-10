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
#define DIR_LENGTH 1024
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

	// Reads input into 'line'
	fgets(line, MAX_LENGTH, stdin);
		
	printf("line: %s", line);	

	char* thisArg;
	thisArg = strtok(line," \n");
	int argCount = 0;
	while(!(thisArg == NULL)) {
		strcpy(jobs[0].args[argCount], thisArg);
		argCount++;
		thisArg = strtok(NULL," \n");
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

	// Checks whether the command is 'exit' or 'quit' and sets the exit bit
	if (strcmp(jobs[0].args[0], "exit") == 0 || strcmp(jobs[0].args[0], "quit") == 0) {
		exitbit = 1;

	// Change the working directory
	} else if (strcmp(jobs[0].args[0], "cd") == 0){
		
		// If there is no argument, change to HOME
		if (jobs[0].argNum < 2){
			printf("Change working directory to HOME.");
			chdir(getenv("HOME");

		// If there is an argument, change to given directory
		} else {
			printf("Change working directory to %s.", jobs[0].args[1]);
			chdir(jobs[0].args[1]);
		}

	// Set PATH or HOME
	} else if (strcmp(jobs[0].args[0], "set") == 0){
		printf("Set PATH or HOME");	
 
	// Runs an executable with using arguments in job struct
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
	
	char dir[DIR_LENGTH];
	int exitbit = 0;
	int numJobs = 0;

	getcwd(dir, DIR_LENGTH);

	while(exitbit == 0) {
		printf("$ ");
		
		Job jobs[MAX_JOBS];

		// Turns input into jobs
		parse(jobs);
		

		// Executes jobs
   		exitbit = execute(jobs);
	}
	
	return 0;
}
