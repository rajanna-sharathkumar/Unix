#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>

/*
    CONCURRENT SERVER: THREAD EXAMPLE
    Must be linked with the "pthread" library also, e.g.:
       cc -o example example.c -lnsl -lsocket -lpthread 

    This program creates a connection socket, binds a name to it, then
    listens for connections to the sockect.  When a connection is made,
    it accepts messages from the socket until eof, and then waits for
    another connection...

    This is an example of a CONCURRENT server -- by creating threads several
    clients can be served at the same time...

    This program has to be killed to terminate, or alternately it will abort in
    120 seconds on an alarm...
*/

#define PORTNUMBER 10206

struct serverParm {
           int connectionDesc;
       };

char *one[] = {" " , " " , " " , " " ," " };
char *two[] = {" " , " " , " " , " " ," " };

char* cmd(char *buf){
        
  int status;
  int fdout;                 /* file descriptor for output */
  pid_t pid;
  char *s;
  int i=0;
  int count=1;
  int fd[2];
  char outp[1024];

     // printf("Message: %s\n",buf);

      // printf("Input: %s\n",buf );
     s = strtok(buf, "\n");
      s = strtok(s,"  ");
      
      // printf("S : %s\n",s );
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

        if((pid=fork()) == -1) {  
        fprintf(stderr, "shell: can't fork: %s\n",
        strerror(errno));
        // continue;
      } else if (pid == 0) {  
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
    else{
        if ((pid=waitpid(pid, &status, 0)) < 0)
            fprintf(stderr, "(count = 2)shell: waitpid error: %s\n",
          strerror(errno));
      }

    }
    else if(i!=0){

      if((pid=fork()) == -1) {  
        fprintf(stderr, "shell: can't fork: %s\n",
        strerror(errno));
        // continue;
      } else if (pid == 0) {  
         // int fd1 = open("sv.txt", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        int fd1 =  open("sv.txt", O_CREAT|O_TRUNC|O_WRONLY, 0644);
        dup2(fd1, 1);   // make stdout go to file
        dup2(fd1, 2);   // make stderr go to file - you may choose to not do this

            // fd no longer needed - the dup'ed handles are sufficient
        if(i==2){
          // printf("i=2; %d\n",i);
          // printf("%s %s\n",one[0],one[1] );
        // child
        execlp(one[0], one[0], one[1], (char *)0);
        }
        else{
          // printf("i!=2; %d\n",i);
          // printf("%s\n", one[0] );
         execlp(one[0], one[0], (char *)0);
        }
        close(fd1);
        perror(one[0]);
      }
      else{
        if ((pid=waitpid(pid, &status, 0)) < 0)
            fprintf(stderr, "(count = 1 )shell: waitpid error: %s\n",
          strerror(errno));
      }
    }
    else{
        if ((pid=waitpid(pid, &status, 0)) < 0)
            fprintf(stderr, "(count = 2)shell: waitpid error: %s\n",
          strerror(errno));
    }
    
    // while (-1 == waitpid(pid, &status, 0));
    sleep(1);
    FILE *f = fopen("sv.txt", "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);  //same as rewind(f);

    char *string = malloc(fsize + 1);
    fread(string, fsize, 1, f);
    fclose(f);

    // remove("sv.txt");
    string[fsize] = 0;
    // printf("%s\n", string);
    return string;
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
  int pid,status;

  switch (pid = fork()) {

  case 0: /* child */
    dup2(pfd[0], 0);  /* this end of the pipe becomes the standard input */
    close(pfd[1]);    /* this process doesn't need the other end */
    int fd1 =  open("sv.txt", O_CREAT|O_TRUNC|O_WRONLY, 0644);
    dup2(fd1,1);
    execvp(two[0], two);  /* run the command */
    close(fd1);
    perror(two[0]);  /* it failed! */

  default: /* parent does nothing */
    break;

  case -1:
    perror("fork");
    exit(1);
  }
  // printf("In Dest\n");
  // while(waitpid(pid, &status, 0) > 0);
}


char* concat(char *s1, char *s2)
{
    char *result = malloc(strlen(s1)+strlen(s2)+1);//+1 for the zero-terminator
    //in real code you would check for errors in malloc here
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

void *serverThread(void *parmPtr) {

#define PARMPTR ((struct serverParm *) parmPtr)
    int recievedMsgLen;
    char messageBuf[1025];

    /* Server thread code to deal with message processing */
    printf("DEBUG: connection made, connectionDesc=%d\n",
            PARMPTR->connectionDesc);
    if (PARMPTR->connectionDesc < 0) {
        printf("Accept failed\n");
        return(0);    /* Exit thread */
    }
    
    /* Receive messages from sender... */
    while ((recievedMsgLen=
            read(PARMPTR->connectionDesc,messageBuf,sizeof(messageBuf)-1)) > 0) 
    {
        recievedMsgLen[messageBuf] = '\0';
        printf("Message: %s\n",messageBuf);
        // printf("check newline\n");
        char* outp = cmd(messageBuf);
        // printf("Output: %s ; Size: %d\n", outp, sizeof(outp) );
        if (write(PARMPTR->connectionDesc, /*concat(*/outp/*,"\0")*/,1025) < 0) {
               perror("Server: write error");
               return(0);
           }
    }
    close(PARMPTR->connectionDesc);  /* Avoid descriptor leaks */
    free(PARMPTR);                   /* And memory leaks */
    return(0);                       /* Exit thread */
}

main () {
    int listenDesc;
    struct sockaddr_in myAddr;
    struct serverParm *parmPtr;
    int connectionDesc;
    pthread_t threadID;

    /* For testing purposes, make sure process will terminate eventually */
    alarm(120);  /* Terminate in 120 seconds */

    /* Create socket from which to read */
    if ((listenDesc = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("open error on socket");
        exit(1);
    }

    /* Create "name" of socket */
    myAddr.sin_family = AF_INET;
    myAddr.sin_addr.s_addr = INADDR_ANY;
    myAddr.sin_port = htons(PORTNUMBER);
        
    if (bind(listenDesc, (struct sockaddr *) &myAddr, sizeof(myAddr)) < 0) {
        perror("bind error");
        exit(1);
    }

    /* Start accepting connections.... */
    /* Up to 5 requests for connections can be queued... */
    listen(listenDesc,5);

    while (1) /* Do forever */ {
        /* Wait for a client connection */
        connectionDesc = accept(listenDesc, NULL, NULL);

        /* Create a thread to actually handle this client */
        parmPtr = (struct serverParm *)malloc(sizeof(struct serverParm));
        parmPtr->connectionDesc = connectionDesc;
        if (pthread_create(&threadID, NULL, serverThread, (void *)parmPtr) 
              != 0) {
            perror("Thread create error");
            close(connectionDesc);
            close(listenDesc);
            exit(1);
        }

        printf("Parent ready for another connection\n");
    }

}