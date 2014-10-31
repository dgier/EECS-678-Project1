#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sys/wait.h>

using namespace std;

/*
 * [X] Run executables without arguments (10)
 * [X] Run executables with arguments (10)
 * [X] set for HOME and PATH work properly (5)
 * [X] exit and quit work properly (5)
 * [X] cd (with and without arguments) works properly (5)
 * [X] PATH works properly. Give error messages when the executable is not found (10)
 * [X] Child processes inherit the environment (5)
 * [X] Allow background/foreground execution (&) (5)
 * [X] Printing/reporting of background processes, (including the jobs command) (10)
 * [X] Allow file redirection (> and <) (5)
 * [X] Allow (1) pipe (|) (10)
 * [X] Supports reading commands from prompt and from file (10)
 * [X] Report (10)
 * [ ] Bonus points:
 *  [X]	Support multiple pipes in one command (10)
 *  [ ]	kill command delivers signals to background processes. The kill 		
 *		command has the format: kill SIGNUM, JOBID, where SIGNUM is an integer
 *		specifying the signal number, and JOBID is an integer that specifies
 *		the job that should receive the signal (5)
 */

#define MAX_LENGTH 1024
#define MAX_JOBS 10
#define MAX_ARGS 10
#define DELIMS " \t\r\n"

int idcount = 0;
extern char ** environ;
pid_t pid;
pid_t forepid = -1;
int exitbit = 0;
bool fore = false;


struct Job {
	int id, argNum;
	char *args[MAX_ARGS];
	bool background;
	bool bgRun;
	string input;
	string output;
	pid_t pid;
	
	Job() {
		id = idcount;
		idcount++;
		argNum = 0;
		background = false;
		bgRun = false;
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

Job jobs[MAX_JOBS];
Job bgJobs[1024];
int bgJobid = 0;

int parse(Job* jobs) {
	
	char line[MAX_LENGTH+1];
	
	// Reads input into 'line'
	fgets(line, MAX_LENGTH, stdin);
	
	char* thisArg;
	thisArg = strtok(line," \n");
	int argCount = 0, currJob = 0;
	while(!(thisArg == NULL)) {
		
		
		if (strcmp(thisArg, "&") == 0) {
			jobs[currJob].background = true;
		} else if (strcmp(thisArg, "|") == 0) {
			jobs[currJob].argNum = argCount;
			jobs[currJob].background = true;
			currJob++; //starting to read new job to pipe to, this should allow multiple pipes per input line once implemented
			argCount = 0;
		} else if (strcmp(thisArg, "<") == 0) {
			thisArg = strtok(NULL, " =\n");	//grab next argument (should be the file name)
			jobs[currJob].input = thisArg; //open file in read only
			if (jobs[currJob].input.empty()) {
				perror ("Error opening input file\n");
			}
		} else if (strcmp(thisArg, ">") == 0) {
			thisArg = strtok(NULL, " =\n");	//grab next argument (should be the file name)
			jobs[currJob].output = thisArg; //open file in write only
			if (jobs[currJob].output.empty()) {
				perror ("Error opening output file\n");
			}
		} else {
			strcpy(jobs[currJob].args[argCount], thisArg);
			argCount++;
		}


		
		
		thisArg = strtok(NULL," =\n");
	}
	
	jobs[currJob].argNum = argCount;	
	
	// Return number of jobs
	return currJob+1;
}

int execute(Job* jobs, int numJobs) {
		
	int exitbit = 0;	
	int pipefd[numJobs-1][2];
	
	for (int i = 0; i < numJobs-1; i++) {
		if (pipe(pipefd[i]) < 0) {
			perror("initializing pipes");
			exit(1);
		}
	}
		
	for (int i = 0; i < numJobs; i++) {		
		
		// Checks whether the command is 'exit' or 'quit' and sets the exit bit
		if (strcmp(jobs[i].args[0], "exit") == 0 || strcmp(jobs[i].args[0], "quit") == 0) {
			exitbit = 1;
			printf("goodbye\n");
			exit(0);
			// Change the working directory
		} else if (strcmp(jobs[i].args[0], "cd") == 0){
			
			// If there is no argument, change to HOME
			if (jobs[i].argNum < 2){
				if(chdir(getenv("HOME")) < 0) {
					printf("ERROR: changing directory to HOME\n");
					exit(1);
				}
				
				// If there is an argument, change to given directory
			} else {

				if(chdir(jobs[i].args[1]) < 0){
					printf("ERROR: changing directory to %s\n", jobs[i].args[1]);
					exit(1);
				}
			}
			
			// Set PATH or HOME
		} else if (strcmp(jobs[i].args[0], "set") == 0){
			char newpath[1024];			
			char cwd[1024];
			getcwd(cwd,sizeof(cwd));
			if(strcmp(jobs[i].args[1], "HOME") == 0){
				strcpy(newpath,cwd);
				strcat(newpath,"/");
				strcat(newpath,jobs[i].args[2]);
			} else {
				strcpy(newpath,getenv(jobs[i].args[1]));
				strcat(newpath,":");
				strcat(newpath,cwd);
				strcat(newpath,"/");
				strcat(newpath,jobs[i].args[2]);
			}
			
			if(setenv(jobs[i].args[1], newpath, 1) < 0){
				printf("ERROR: set for %s as %s\n", jobs[i].args[1], jobs[i].args[2]);
				exit(1);
			}

		// Return PATH or HOME
		} else if (strcmp(jobs[i].args[0], "get") == 0){

			char thisenv[1024];
			strcpy(thisenv, getenv(jobs[i].args[1]));
			printf("%s = %s\n", jobs[i].args[1], thisenv); 			
			
			// Prints jobs running in the background
		} else if (strcmp(jobs[i].args[0], "jobs") == 0) {
			bool nobackground = true;			
			for (int j = 0; j < bgJobid; j++) {
				if(bgJobs[j].bgRun == true){
					printf("[%d] %d %s\n", j, bgJobs[j].pid, bgJobs[j].args[0]);
					nobackground = false;
				}
			}

			if(nobackground) {
				printf("No background processes running\n");
			}
			
			// Runs quash using commands from file
		} else if (strcmp(jobs[i].args[0], "quash") == 0) {
			FILE *f = fopen(jobs[i].args[1], "r");
			
			dup2(fileno(f), STDIN_FILENO);
			fclose(f);
			// Runs an executable with using arguments in job struct
		} else {
			
			// Execute file using arguments
			pid=fork();

			if (pid == 0) {
				//childProcesses;
				
				if ((!jobs[i].input.empty()) || (!jobs[i].output.empty())) { //remember by default all id's > 0 so 0 means empty
				printf("using < or >\n");
					if (!jobs[i].input.empty()) {	// Redirect standard in (due to '<')
						printf("input not empty\n");
						FILE *f = fopen(jobs[i].input.c_str(), "r");
						dup2(fileno(f), STDIN_FILENO);
						fclose(f);
						//set up pipe to read from job[inPipeId]
					}
					if (!jobs[i].output.empty()){	// Redirect standard out (due to '>')
						printf("output not empty\n");
						FILE *f = fopen(jobs[i].output.c_str(), "w+");
						dup2(fileno(f), STDOUT_FILENO);
						fclose(f);
						//set up pipe to write to job[outPipeId]
					}
				}

				if (numJobs > 1) { 
					
					if (i < numJobs-1) { //write end						
						if (dup2(pipefd[i][1], STDOUT_FILENO) < 0) {
							perror("write pipe error");
						} 
					}
					
					if (i > 0) { //read end
						if (dup2(pipefd[i-1][0], STDIN_FILENO) < 0) {
							perror("read pipe error");
						}
					}
					
					for (int j = 0; j < numJobs-1; j++) {
						if (close(pipefd[j][1]) < 0) {
							perror("close writes");
						}
						
						if (close(pipefd[j][0]) < 0) {
							perror("close reads");
						}
					}
				}
				
								
				
				
				// Set argument after last to NULL so exec will know when to stop
				jobs[i].args[jobs[i].argNum] = NULL;
				

				// If executable is in current directory, run it
				if(access(jobs[i].args[0], F_OK) != -1) {
					//if(execvpe(jobs[i].args[0], jobs[i].args, environ) < 0){//linux
					if(execve(jobs[i].args[0], jobs[i].args, environ) < 0){//os x
						char execfile[100];
						strcpy(execfile, "./");
						strcat(execfile, jobs[i].args[0]);
						//if(execvpe(execfile, jobs[i].args, environ) < 0){	//linux					
						if(execve(execfile, jobs[i].args, environ) < 0){	//os x					
							printf("ERROR 292: exec for %s\n", jobs[i].args[0]);
							exit(1);
						}
					}
				} else {
					// Otherwise search path to find it and run it
					char* curPath;
					curPath = strtok(getenv("PATH"),":\n");

					while(curPath != NULL){
						char execfile[100];
						strcpy(execfile, curPath);
						//printf("%s\n", curPath);
						strcat(execfile, "/");
						strcat(execfile, jobs[i].args[0]);
						if(access(execfile, F_OK) != -1) {
							//if(execvpe(execfile, jobs[i].args, environ) < 0){//linux
							if(execve(execfile, jobs[i].args, environ) < 0){//os x
								printf("ERROR 311: exec for %s\n", jobs[i].args[0]);
								exit(1);
							}
						}
						curPath = strtok(NULL,":\n");
					}
		
					printf("ERROR: could not find %s\n", jobs[i].args[0]);
					printf("ERROR: most likely %s not in PATH\n", jobs[i].args[0]);
					exit(1);
				}
				
				// system(cmd);
				
			} else {

				jobs[i].pid = pid;
				//parentProcesses();

				if (!jobs[i].background) {		
				// Wait for child to finish if process is in foreground.
					forepid = pid;
					fore = true;
					while(fore) {
						pause();
					}
	
				// If running in background, add to job list and move on.
				} else {
					printf("[%d] %d %s Running in background\n", bgJobid, jobs[i].pid, jobs[i].args[0]);
					jobs[i].bgRun = true;
					bgJobs[bgJobid].bgRun = true;
					strcpy(bgJobs[bgJobid].args[0], jobs[i].args[0]);
					bgJobs[bgJobid].pid = jobs[i].pid;
					bgJobs[bgJobid].id = bgJobid;
					bgJobid++;
				}
			}
		}
	}
	// Return 0 to continue and 1 to exit. 
	return exitbit;
}

// Exits from a child process
void childExit(int sig) {
	pid_t pid;
	int status;

	pid = waitpid(WAIT_ANY, &status, WNOHANG | WUNTRACED);

	for(int i = 0; i < bgJobid; i++){
		if(bgJobs[i].pid == pid){
			bgJobs[i].bgRun = false;
		}
	}

	if (forepid == pid) {
		forepid = -1;
		fore = false;
		return;
	}
}

// Uses environment from unistd.h rather than parsing envp variable given in main function.
int main() {
	
	int numJobs = 0;
	signal(SIGCHLD, childExit);	

	while(exitbit == 0) {
		printf("$ ");
		
		// Turns input into jobs
		numJobs = parse(jobs);

		if (numJobs>0) {
			exitbit = execute(jobs, numJobs);
		} else {
			exitbit = 1;
		}
		
		if(exitbit == 0) {
			for(int i = 0; i < numJobs; i++) {
				jobs[i].background = false;
				jobs[i].input = "";
				jobs[i].output = "";
			}
		}
	}
	return 0;
}
