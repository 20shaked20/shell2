#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "stdio.h"
#include "errno.h"
#include "stdlib.h"
#include "unistd.h"
#include <string.h>

int main() {
char command[1024];
char *token;
int i;
char *outfile;
char* prompt = "hello";
int fd, amper, redirect, piping, retid, status, argc1/*last token indx*/, concat, outerr;
int fildes[2];
char *argv1[10], *argv2[10];
char *cmd;

while (1)
{
    printf("%s: ",prompt);
    fgets(command, 1024, stdin);
    command[strlen(command) - 1] = '\0';
    piping = 0;

    /* parse command line */
    i = 0;
    token = strtok (command," ");
    while (token != NULL)
    {
        argv1[i] = token;
        token = strtok (NULL, " ");
        i++;
        if (token && ! strcmp(token, "|")) {
            piping = 1;
            break;
        }
    }
    argv1[i] = NULL;
    argc1 = i;

    /* Is command empty */
    if (argv1[0] == NULL)
        continue;

    /* Does command contain pipe */
    if (piping) {
        i = 0;
        while (token!= NULL)
        {
            token = strtok (NULL, " ");
            argv2[i] = token;
            i++;
        }
        argv2[i] = NULL;
    }

    /* Does command line end with & */ 
    if (! strcmp(argv1[argc1 - 1], "&")) {
        amper = 1;
        argv1[argc1 - 1] = NULL;
        }
    else 
        amper = 0; 


    /* redirect */
    if (argc1 > 1 && ! strcmp(argv1[argc1 - 2], ">")) {
        concat = 1;
        argv1[argc1 - 2] = NULL;
        outfile = argv1[argc1 - 1];
    }
    else if(argc1 > 1 && ! strcmp(argv1[argc1 - 2], "2>")){
        outerr = 1;
        argv1[i - 2] = NULL;
        outfile = argv1[i - 1];
    }
    else if(! strcmp(argv1[argc1 - 2], ">>")){
        redirect = 1;
        argv1[argc1 - 2] = NULL;
        outfile = argv1[argc1 - 1];
    }
    else
        redirect = 0;

    /* prompt change */ /*fix this its always doing the prompt again..*/
    if (! strcmp(argv1[0], "prompt")) {

        if(! strcmp(argv1[1], "=")) { 
            // argv1[argc1 - 2] = NULL;
            // argv1[argc1 - 3] = NULL;
            prompt = argv1[argc1 - 1];
        }
        continue;    
    }
    else if(! strcmp(argv1[0], "echo")) {

        if(! strcmp(argv1[1], "$?")) 
            printf("%d", status);

        else{

            for (size_t indx = 1 ; indx < argc1 ; ++indx){
                
                printf("%s ",argv1[indx]);
            }
        }
        printf("\n");
        continue;
    }
    // else if(! strcmp(argv1[0], "cd")){

    //     if(chdir(argv1[1]) != 0)
    //         printf("cd: no such file or directory: %s\n",argv1[1]);
        
    //     continue;
    // }

    /* for commands not part of the shell command language */ 

          /* for commands not part of the shell command language */
        if (fork() == 0)
        {
            /* redirection of IO ? */
            if (redirect)
            {
                fd = creat(outfile, 0660);
                close(STDOUT_FILENO);
                dup(fd);
                close(fd);
                /* stdout is now redirected */
            }
            if (concat)
            {
                fd = open(outfile, O_RDWR | O_CREAT | O_APPEND, 0660);
                close(STDOUT_FILENO);
                dup(fd);
                close(fd);
                /* stdout is now redirected */
            }
            if (piping)
            {
                pipe(fildes);
                if (fork() == 0)
                {
                    /* first component of command line */
                    close(STDOUT_FILENO);
                    dup(fildes[1]);
                    close(fildes[1]);
                    close(fildes[0]);
                    /* stdout now goes to pipe */
                    /* child process does command */
                    execvp(argv1[0], argv1);
                }
                /* 2nd command component of command line */
                close(STDIN_FILENO);
                dup(fildes[0]);
                close(fildes[0]);
                close(fildes[1]);
                /* standard input now comes from pipe */
                execvp(argv2[0], argv2);
            }
            else if (outerr)
            {
                // redirect to stderr
                fd = creat(outfile, 0660);
                close(STDERR_FILENO);
                dup(fd);
                close(fd);
                /* stderr is now redirected */
                execvp(argv1[0], argv1);
            }
            else
                execvp(argv1[0], argv1);
        }
        /* parent continues over here... */
        /* waits for child to exit if required */
        if (amper == 0)
            retid = wait(&status);
    }
}
