#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>
#include <fcntl.h>

char *one[] = {" " , " " , " " , " " ," " };
char *two[] = {" " , " " , " " , " " ," " };

void runsource(int pfd[]);
void rundest(int pfd[]);

char *getinput(char *buffer, size_t buflen) {
  printf("$$ ");
  return fgets(buffer, buflen, stdin);
}

int main(int argc, char **argv)
{
  int status;
  int fdout;                 /* file descriptor for output */
  
  char buf[1024];
  pid_t pid;
  char *s;
  int i=0;
  int count=1;
  int fd[2];

  
  

  while (getinput(buf, sizeof(buf))) {
    
    buf[strlen(buf) - 1] = '\0';

  
    
      // printf("Input: %s\n",buf );
      s = strtok(buf,"  ");
      i=0;
      count = 1;
      while(s!=NULL){
        if(strcmp(s,"|")==0){
          count = 2;
          one[i] = (char *)0;
          i=0;
        }
        else if(count ==1){
          one[i] = s;
          // printf("%s , %s , %d\n", one[i] , s , i);
          i++;
        }
        else{
          two[i] = s;
          // printf("%s , %s , %d\n", two[i] , s , i);
          i++;
        }

        s = strtok(NULL,"   ");
      }
      two [i] = (char *)0;
    if(count == 2){
      pipe(fd);
      // printf("Done Parsing\n");
      runsource(fd);
      rundest(fd);
      close(fd[0]); close(fd[1]);   /* this is important! close both file descriptors on the pipe */
      // parent is waiting
      // if ((pid=waitpid(pid, &status, 0)) < 0)
      //   fprintf(stderr, "shell: waitpid error: %s\n",
      //     strerror(errno));
    }
    else if(i!=0){

      if((pid=fork()) == -1) {  
        fprintf(stderr, "shell: can't fork: %s\n",
        strerror(errno));
        continue;
      } else if (pid == 0) {  

        if(i==2){
          execlp(one[0], one[0], one[1], NULL);
          printf("i=2\n");
          sleep(5);
        }
        else{
          
          execlp(one[0], one[0], NULL);
          printf("i!=2\n");
          sleep(5);
        }
        perror(one[0]);

      }
    }
    else{
      
    }
    if ((pid=waitpid(pid, &status, 0)) < 0)
      fprintf(stderr, "shell: waitpid error: %s\n",
          strerror(errno));
  }
  if ((pid=waitpid(pid, &status, 0)) < 0)
      fprintf(stderr, "shell: waitpid error: %s\n",
          strerror(errno));
  
  exit(EX_OK);

}

void
runsource(int pfd[])  /* run the first part of the pipeline, cmd1 */
{
  int pid;  /* we don't use the process ID here, but you may wnat to print it for debugging */

  switch (pid = fork()) {

  case 0: /* child */
    dup2(pfd[1], 1);  /* this end of the pipe becomes the standard output */
    close(pfd[0]);    /* this process don't need the other end */
    execvp(one[0], one);  /* run the command */
    perror(one[0]);  /* it failed! */

  default: /* parent does nothing */
    break;

  case -1:
    perror("fork");
    exit(1);
  }
}

void
rundest(int pfd[])  /* run the second part of the pipeline, cmd2 */
{
  int pid;

  switch (pid = fork()) {

  case 0: /* child */
    dup2(pfd[0], 0);  /* this end of the pipe becomes the standard input */
    close(pfd[1]);    /* this process doesn't need the other end */
    execvp(two[0], two);  /* run the command */
    perror(two[0]);  /* it failed! */

  default: /* parent does nothing */
    break;

  case -1:
    perror("fork");
    exit(1);
  }
}

