#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * [X] Run executables without arguments (10)
 * [X] Run executables with arguments (10)
 * [X] set for HOME and PATH work properly (5)
 * [X] exit and quit work properly (5)
 * [X] cd (with and without arguments) works properly (5)
 * [X] PATH works properly. Give error messages when the executable is not 		found (10)
 * [ ] Child processes inherit the environment (5)
 * [ ] Allow background/foreground execution (&) (5)
 * [ ] Printing/reporting of background processes, (including the jobs 		command) (10)
 * [ ] Allow file redirection (> and <) (5)
 * [ ] Allow (1) pipe (|) (10)
 * [ ] Supports reading commands from prompt and from file (10)
 * [ ] Report (10)
 * [ ] Bonus points:
 *  [ ]	Support multiple pipes in one command (10)
 *  [ ]	kill command delivers signals to background processes. The kill 		
 *		command has the format: kill SIGNUM, JOBID, where SIGNUM is an integer
 *		specifying the signal number, and JOBID is an integer that specifies
 *		the job that should receive the signal (5)
 */

#define MAX_LENGTH 1024
#define DIR_LENGTH 1024
#define MAX_JOBS 10
#define MAX_ARGS 10
#define DELIMS " \t\r\n"

int idcount = 1;
extern char ** environ;
pid_t pid;

struct Job {
	int id, argNum, outPipeId, inPipeId;
	char *args[MAX_ARGS];
	bool background;
	FILE * input, output;
	
	Job() {
		id = idcount;
		idcount++;
		argNum = 0;
		outPipeId = 0;
		inPipeId = 0; //idcount starts at 1 so if 0, not piping to/from other job.
		background = false;
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
	int argCount = 0, jobCount = 0; 
	while(!(thisArg == NULL)) {
		strcpy(jobs[jobCount].args[argCount], thisArg);
		
		if (strcmp(thisArg, "&")) {
			jobs[jobCount].background = true;
		} else if (strcmp(thisArg, "|")) {
			jobs[jobCount].outPipeId = jobs[jobCount].id+1; //let the curr job know who to output to the next job
			jobs[jobCount+1].inPipeId = jobs[jobCount].id; //let the new job know who take input from NOTE MAY CAUSE SEG FAULT CHECK THIS
			jobCount++; //starting to read new job to pipe to, this should allow multiple pipes per input line once implemented
		}
		
		argCount++;
		thisArg = strtok(NULL," =\n");
	}
	
	jobs[jobCount].argNum = argCount;	
	
	for(int i = 0; i < jobs[0].argNum; i++){
		printf("Arg[%i] = %s\n", i, jobs[0].args[i]);
	}
	
	// Return number of jobs
	return jobCount+1;
}

int execute(Job* jobs, int numJobs) {
	
	int exitbit = 0;	
	
	for (int i = 0; i < numJobs; i++) {		
		
		printf("args[0] = %s\n", jobs[i].args[0]);
		
		// Checks whether the command is 'exit' or 'quit' and sets the exit bit
		if (strcmp(jobs[i].args[0], "exit") == 0 || strcmp(jobs[i].args[0], "quit") == 0) {
			exitbit = 1;
			
			// Change the working directory
		} else if (strcmp(jobs[i].args[0], "cd") == 0){
			
			// If there is no argument, change to HOME
			if (jobs[i].argNum < 2){
				printf("Change working directory to HOME.\n");
				if(chdir(getenv("HOME")) < 0) {
					printf("ERROR: changing directory to HOME\n");
				}
				
				// If there is an argument, change to given directory
			} else {
				printf("Change working directory to %s.\n", jobs[i].args[1]);
				if(chdir(jobs[i].args[1]) < 0){
					printf("ERROR: changing directory to %s\n", jobs[i].args[1]);
				}
			}
			
			// Set PATH or HOME
		} else if (strcmp(jobs[i].args[0], "set") == 0){
			printf("Set environment (generally PATH or HOME)\n");
			
			if(setenv(jobs[i].args[1], jobs[i].args[2], 1) < 0){
				printf("ERROR: set for %s as %s\n", jobs[i].args[1], jobs[i].args[2]);
			}
			
			
			// Runs an executable with using arguments in job struct
		} else {
			/*		char cmd[MAX_LENGTH] = {0};
			 strcat(cmd, jobs[i].args[0]);
			 for(int i = 1; i < jobs[i].argNum; i++){
			 strcat(strcat(cmd, " "), jobs[i].args[i]);
			 }
			 */
			
			// Set argument after last to NULL so exec will know when to stop
			jobs[i].args[jobs[i].argNum] = NULL;
			
			
			
			// Execute file using arguments
		
			pid=fork();
			if (pid == 0) {
				//childProcesses;
				printf("childProcesess\n");
				
			} else {
				printf("parentProcess\n");
				//parentProcesses();
			}

			
			//if(execvpe(jobs[i].args[0], jobs[i].args, environ) < 0){//linux?
			if(execve(jobs[i].args[0], jobs[i].args, environ) < 0){//os x?
				printf("ERROR 155: exec for %s\n", jobs[i].args[0]);
				printf("ERROR: most likely %s not in PATH\n", jobs[i].args[0]);
			} else {
				printf("made it here\n");
			}

			
			// system(cmd);
		}
	}
	printf("exitbit leaving execute %i\n", exitbit);
	// Return 0 to continue and 1 to exit. 
	return exitbit;
}

int main(int argc, char **argv, char **envp) {
	
	char dir[DIR_LENGTH];
	int exitbit = 0;
	int numJobs = 0;
	
	getcwd(dir, DIR_LENGTH);
	int iters = 0;
	while(exitbit == 0) {
		printf("iters = %i\n", iters);
		printf("$ ");
		
		Job jobs[MAX_JOBS];
		
		// Turns input into jobs
		numJobs = parse(jobs);
		
		if (numJobs>0) {
			exitbit = execute(jobs, numJobs);
		} else {
			exitbit = 1;
			printf("numJobs = %i, exiting loop", numJobs);
		}
		
		// Executes jobs
   		//exitbit = execute(jobs, numJobs);
		
		printf("exitbit: %i\n", exitbit);
	}
	printf("goodbye\n");
	return 0;
}
