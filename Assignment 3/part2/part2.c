#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>
#include <fcntl.h>

char *getinput(char *buffer, size_t buflen) {
    printf("$$ ");
    return fgets(buffer, buflen, stdin);
}

int main(int argc, char **argv) {
    int status;
    int fdout; /* file descriptor for output */
    char buf[1024];
    pid_t pid;
    char *s;
    int i = 1;
    char *command[] = {" ", " ", " ", " "};
    while (getinput(buf, sizeof (buf))) {
        buf[strlen(buf) - 1] = '\0';
        if ((pid = fork()) == -1) {
            fprintf(stderr, "shell: can't fork: %s\n",
                    strerror(errno));
            continue;
        }
        else if (pid == 0) {
            printf("Input: %s\n", buf);
            s = strtok(buf, " 	");
            i = 1;
            while (s != NULL) {
                if (i == 1) {
                    printf("Command: %s\n", s);
                    i++;
                    command[0] = s;
                } else if (strcmp(s, ">") == 0 || strcmp(s, "<") == 0 || strcmp(s, ">>") == 0) {
                    printf("File Redirection: %s\n", s);
                    command[2] = s;
                    s = strtok(NULL, " 	");
                    command[3] = s;
                    printf("File: %s\n", s);

                    if (strcmp(command[2], ">") == 0) {
                        if ((fdout = open(command[3], O_CREAT | O_TRUNC | O_WRONLY, 0644)) < 0) {
                            perror(command[3]); /* open failed */
                            exit(1);
                        }

                        printf("Writing output of the command %s to \"%s\"\n", command[0], command[3]);
                        dup2(fdout, 1);
                    } else if (strcmp(command[2], ">>") == 0) {
                        if ((fdout = open(command[3], O_CREAT | O_RDWR | O_APPEND, 0644)) < 0) {
                            perror(command[3]); /* open failed */
                            exit(1);
                        }

                        printf("Appending output of the command %s to \"%s\"\n", command[0], command[3]);
                        dup2(fdout, 1);
                    } else {
                        if ((fdout = open(command[3], O_RDONLY, 0644)) < 0) {
                            perror(command[3]); /* open failed */
                            exit(1);
                        }

                        printf("Executing %s\n", buf);
                        dup2(fdout, 0);
                    }
                    if (i == 3)
                        execlp(command[0], command[0], command[1], (char *) 0);
                    else
                        execlp(command[0], command[0], (char *) 0);

                    perror(command[0]); /* execvp failed */
                } else if (strcmp(s, "|") == 0) {
                    printf("Pipe\n");
                    i = 1;
                } else if (i == 2) {
                    printf("Options: %s\n", s);
                    i++;
                    command[1] = s;
                } else if (i == 3) {
                    printf("Arguments: %s\n", s);
                    i++;
                } else {
                    printf("strok : %s\n", s);
                }
                s = strtok(NULL, " 	");
            }
            exit(EX_DATAERR);
        }

        if ((pid = waitpid(pid, &status, 0)) < 0)
            fprintf(stderr, "shell: waitpid error: %s\n", strerror(errno));
    }
    exit(EX_OK);

}
