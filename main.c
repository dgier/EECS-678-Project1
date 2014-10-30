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
 * [?] Allow file redirection (> and <) (5)
 * [?] Allow (1) pipe (|) (10)
 * [?] Supports reading commands from prompt and from file (10)
 * [X] Report (10)
 * [ ] Bonus points:
 *  [?]	Support multiple pipes in one command (10)
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
bool fore = false;


struct Job {
	int id, argNum, outPipeId, inPipeId;
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
			jobs[currJob+1]=Job(); //create next job to output to
			jobs[currJob].outPipeId = jobs[currJob].id+1; //let the curr job know who to output to the next job
			jobs[currJob+1].inPipeId = jobs[currJob].id; //let the new job know who take input from
			currJob++; //starting to read new job to pipe to, this should allow multiple pipes per input line once implemented
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

// Finds a path to a file if it exissts on the PATH
char * search_path(char* filename) {
/*	char* path_to_file;
	char* path = getenv("PATH");
	string cur_path = "";
	DIR * d;
	dirent *ent;

	int char_count = 0;
	while(char_count < strlen(filename)){
		while(strcmp(path[char_count],":") != 0 && char_count < stren(filename)){
			curpath += path[char_count];
			char_count++;
		}
		
		if ((d = opendir(cur_path.c_str())) != NULL) {
			while ((ent = readdir(d)) != NULL) {
				if (ent->d_name == filename.c_str()) {
					return cur_path + '/' + filename;
				}
			}
		}
		char_count++;
	
	}*/
	return filename;

}

int execute(Job* jobs, int numJobs) {
	
	int exitbit = 0;	
	int pipefd[numJobs-1][2];
		
	for (int i = 0; i < numJobs; i++) {		
		
		// Checks whether the command is 'exit' or 'quit' and sets the exit bit
		if (strcmp(jobs[i].args[0], "exit") == 0 || strcmp(jobs[i].args[0], "quit") == 0) {
			//exitbit = 1;
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
			}
			
			// Prints jobs running in the background
		} else if (strcmp(jobs[i].args[0], "jobs") == 0) {
			bool nobackground = true;			
			for (int i = 0; i < bgJobid; i++) {
				if(bgJobs[i].bgRun == true){
					printf("[%d] %d %s\n", i, bgJobs[i].pid, bgJobs[i].args[0]);
					nobackground = false;
				}
			}

			if(nobackground) {
				printf("No background processes running.");
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
				
				if ((jobs[i].inPipeId != 0) || (jobs[i].outPipeId != 0)) { //remember by default all id's > 0 so 0 means empty
				printf("using < or >\n");
					if (jobs[i].inPipeId != 0) {	// Redirect standard in (due to '<')
						printf("inPipeId != 0\n");
						FILE *f = fopen(jobs[i].input.c_str(), "r");
						dup2(fileno(f), STDIN_FILENO);
						fclose(f);
						//set up pipe to read from job[inPipeId]
					}
					if (jobs[i].outPipeId != 0){	// Redirect standard out (due to '>')
						printf("outPipeId != 0\n");
						FILE *f = fopen(jobs[i].output.c_str(), "w+");
						dup2(fileno(f), STDOUT_FILENO);
						fclose(f);
						//set up pipe to write to job[outPipeId]
					}
				}
				
				/*
				// Redirect standard in (due to '<')
				if (!jobs[i].input.empty()) {
					FILE *f = fopen(jobs[i].input.c_str(), "r");
					dup2(fileno(f), STDIN_FILENO);
					fclose(f);
				}

				// Redirect standard out (due to '>')
				if (!jobs[i].output.empty()) {
					FILE *f = fopen(jobs[i].output.c_str(), "w+");
					dup2(fileno(f), STDOUT_FILENO);
					fclose(f);
				}*/
			

				if (numJobs > 1) { 
					printf("numJobs > 1\n");
					if (i == 0) { //first pipe
						if (dup2(pipefd[i][1], STDOUT_FILENO) < 0) {
							perror("first pipe");
						}
					} else if (i < numJobs-1) {
						if (dup2(pipefd[i-1][0], STDIN_FILENO) < 0) {
							perror("middle pipe read");
						}
						if (dup2(pipefd[i][1], STDOUT_FILENO) < 0) {
							perror("middle pipe write");
						}
					} else if (i == numJobs-1) {
						if (dup2(pipefd[i-1][0], STDIN_FILENO) < 0)
							perror("last pipe");
					}
					
					for (int j = 0; j < numJobs-1; j++) {
						if (close(pipefd[j][1]) < 0) {
							
						}
						
						if (close(pipefd[j][0]) < 0) {
							perror("close all");
						}
					}
				}
				
								
				// Set argument after last to NULL so exec will know when to stop
				jobs[i].args[jobs[i].argNum] = NULL;
			
				// If executable is in current directory, run it
				if(access(jobs[i].args[0], F_OK) != -1) {
					if(execvpe(jobs[i].args[0], jobs[i].args, environ) < 0){//linux?
					//if(execve(jobs[i].args[0], jobs[i].args, environ) < 0){//os x?
						char execfile[100];
						strcpy(execfile, "./");
						strcat(execfile, jobs[i].args[0]);
						if(execvpe(execfile, jobs[i].args, environ) < 0){						
							printf("ERROR 155: exec for %s\n", jobs[i].args[0]);
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
						strcat(execfile, "/");
						strcat(execfile, jobs[i].args[0]);

						if(access(execfile, F_OK) != -1) {
							if(execvpe(execfile, jobs[i].args, environ) < 0){//linux?
							//if(execve(jobs[i].args[0], jobs[i].args, environ) < 0){//os x?
								printf("ERROR 155: exec for %s\n", jobs[i].args[0]);
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
					printf("[%d] %d Running in background\n", jobs[i].id, jobs[i].pid);
					jobs[i].bgRun = true;
					bgJobs[bgJobid] = jobs[i];
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
		if(jobs[i].pid == pid){
			jobs[i].bgRun = false;
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
	
	int exitbit = 0;
	int numJobs = 0;
	int iters = 0;
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
		
		// Executes jobs
   		//exitbit = execute(jobs, numJobs);
		
		iters++;
	}
	return 0;
}
