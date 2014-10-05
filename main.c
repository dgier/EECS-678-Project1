#include <stdio.h>
#include <stdlib.h>

/*
 * 1. Run executables without arguments (10)
 * 2. Run executables with arguments (10)
 * 3. set for HOME and PATH work properly (5)
 * 4. exit and quit work properly (5)
 * 5. cd (with and without arguments) works properly (5)
 * 6. PATH works properly. Give error messages when the executable is not found (10)
 * 7. Child processes inherit the environment (5)
 * 8. Allow background/foreground execution (&) (5)
 * 9. Printing/reporting of background processes, (including the jobs command) (10)
 * 10. Allow file redirection (> and <) (5)
 * 11. Allow (1) pipe (|) (10)
 * 12. Supports reading commands from prompt and from file (10)
 */

#define MAX_LENGTH 1024
#define DELIMS " \t\r\n"

int main(int argc, char **argv, char **envp) {
	char *cmd;
	char line[MAX_LENGTH];

	while(true) {
		printf("$ ");
		if(!fgets(line, MAX_LENGTH, stdin)) //fgets => reads all input in string into 'line'
			break;

		
		system(line);
	}
	
	return 0;

}
