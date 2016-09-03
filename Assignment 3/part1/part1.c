// For Part2 – simple command shell (for a single command).
// Note: For Part1 you may begin with the following simple shell program.
/*
 * World's simplest shell. Loops, reads input and tries to execute it.
 * Note: no tokenization, can be ^C'd, but does look at PATH not sorted
 * ./simple-shell
 * $$ ls
 * $$ ^C
 */

#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

char *
getinput(char *buffer, size_t buflen) {
	printf("$$ ");
	return fgets(buffer, buflen, stdin);
}

int
main(int argc, char **argv) {
	char buf[1024];
	pid_t pid;
	int status;
	char *s;
	int i=1;

	while (getinput(buf, sizeof(buf))) {
		buf[strlen(buf) - 1] = '\0';

		if((pid=fork()) == -1) {  
			fprintf(stderr, "shell: can't fork: %s\n",
					strerror(errno));
			continue;
		} else if (pid == 0) {   
			/* child to run the command */

			/* your code to parse the command & set any file redirection as needed */
		
		
			printf("Input: %s\n",buf );
			s = strtok(buf," 	");
			i=1;
			while(s!=NULL){
				if(i==1){
					printf("Command: %s\n",s);
					i++;
				}
				else if(strcmp(s,">")==0 || strcmp(s,"<")==0){
					printf("File Redirection: %s\n",s);
					s = strtok(NULL," 	");
					printf("File: %s\n",s);
				
				}
				else if(strcmp(s,"|")==0){
					printf("Pipe\n");
					i=1;
				}
				else if(i==2){
					printf("Options: %s\n",s);
					i++;
				}
				else if(i==3){
					printf("Arguments: %s\n",s);
					i++;
				}
				else{
					printf("strok : %s\n",s);
				}
				s = strtok(NULL," 	");
			}



			//execlp(buf, buf, (char *)0);
			//fprintf(stderr, "shell: couldn't exec %s: %s\n", buf,
			//		strerror(errno));
			exit(EX_DATAERR);
		}

            // parent is waiting
		if ((pid=waitpid(pid, &status, 0)) < 0)
			fprintf(stderr, "shell: waitpid error: %s\n",
					strerror(errno));
	}
	exit(EX_OK);
}
