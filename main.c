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
	int id, argc;
	char *argv[MAX_ARGS];

	Job() {
		id = 0;
		argc = 0;
		for(int i = 0; i < MAX_ARGS; i++){
			argv[i] = new char[256];
		}
	}

	~Job() {
		for(int i = 0; i < MAX_ARGS; i++){
			delete[] argv[i];
		}
	}
};

int parse(string line, Job job1) {
	// Split 'line' on spaces to separate args

	// Add args to job1 struct. First arg is executable, others are parameters

	// Return number of jobs
	return 1;
}

int execute(Job job1) {

	exit = 0;	

	// Make system call to execute job1 using args.


	// Return 0 to continue and 1 to exit. 
	return exit;
}

int main(int argc, char **argv, char **envp) {
	char *cmd;
	char line[MAX_LENGTH];
	int exit = 0;

	while(exit == 0) {
		printf("$ ");
		if(!fgets(line, MAX_LENGTH, stdin)){ //fgets => reads all input in string into 'line'
			break;
 		}

		Job job1;
		// Turns input line into job
		parse(line);


		// Execute job
   		exit = execute(job);
	}
	
	return 0;
}
