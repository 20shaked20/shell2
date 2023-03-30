#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "stdio.h"
#include "errno.h"
#include "stdlib.h"
#include "unistd.h"
#include <string.h>

char *prompt = "hello";

char *names[10];
char *vales[10];
int num_variables = 0;
char command[1024];
char previous_command[1024];
char *token;
int i ,x;
char *outfile;
int amper, redirect, concat /*open new file*/, outerr /*related to stderr*/;
int fd, piping, pipings = 0, retid, status, argc1 /*last token indx*/;
int fildes[3];
char *argv1[10], *argv2[10], *argv3[10], *argv4[10] *commands[100];
int num_comm=0;
// char *argvs[3][10];

void sig_handler()
{
        signal(SIGINT, sig_handler);
	    printf("\n You typed Control-C!\n");
        printf("%s: ", prompt);
        fflush(stdout);
}

    
void handler()
{
    
    signal(SIGINT, sig_handler);

    while (1)
    {
        
    
        if (argv1[0] != NULL)
        {
            strcpy(previous_command, "");
            for (size_t i = 0; argv1[i] != NULL; i++)
            {
                strcat(previous_command, argv1[i]);
                strcat(previous_command, " ");
            }
        }
        
        printf("%s: ", prompt);
        fgets(command, 1024, stdin);
        command[strlen(command) - 1] = '\0';
        commands[num_comm] = strdup(command);
        num_comm++;
        piping = 0;
        char shit = command[0];
        if (shit == '$'){
            i = 0;
            token = strtok(command, " ");
            while (token != NULL)
            {   
                printf("token ->  %s  \n", token);
                if(i == 0){
                names[num_variables] = strdup(token);
                token = strtok(NULL, " ");
                printf("%s \n" ,  names[num_variables]);
                }
                if(i == 1){
                argv4[1] = strdup(token);
                token = strtok(NULL, " ");
                }
                if(i == 2){
                vales[num_variables] =  strdup(token); 
                printf("%s \n" ,  vales[num_variables]);   
                token = strtok(NULL, " ");  
                }            
                i++;
            }   
            num_variables++;
        }
        /* parse command line */
        i = 0;
        // x = 0;
        token = strtok(command, " ");
        // printf("token ->  %s  \n", token);
        while (token != NULL)
        {
            argv1[i] = token;
            // argvs[x][i] = token;
            token = strtok(NULL, " ");
            // printf("token ->  %s  \n", token);
            i++;
            if (token && !strcmp(token, "|")) // "cat colors | cat | cat | cat"
            {
               piping = 1;
            //    pipings = 1;
            //    x++;
               break;
            }
        }
        argv1[i] = NULL;
        argc1 = i;

        /* Is command empty */
        if (argv1[0] == NULL)
            continue;
        
        
        if(!strcmp(argv1[0], "read")){

            char *str1 = malloc(strlen(argv1[0]) + 2); // +2 for "$" and null terminator
            sprintf(str1, "$%s", argv1[1]); // use sprintf to concatenate strings
            names[num_variables] = strdup(str1);
            char command2[1024];
            fgets(command2, 1024, stdin);
            command2[strlen(command2) - 1] = '\0';
            vales[num_variables] = strdup(command2);
            num_variables++;
            free(str1); // free memory allocated by malloc
            continue;

        }

        /*exit*/
        if (!strcmp(argv1[0], "quit"))
        {
            exit(0);
        }
        else if (!strcmp(argv1[0], "!!"))
        {
            strcpy(command, previous_command);

            /* parse command line */
            i = 0;
            token = strtok(command, " ");
            while (token != NULL)
            {
                argv1[i] = token;
                token = strtok(NULL, " ");
                i++;
                if (token && !strcmp(token, "|"))
                {
                    piping = 1;
                    break;
                }
            }
        }
        // else if(argv1[0][0] == "$")
        // {
        //     printf("Zubi \n");
        // }

        // /* Does command contain pipe */
        // if (piping == 1) {
        //     i = 0;
        //     while (token!= NULL)
        //     {
        //         token = strtok (NULL, " ");
        //         // printf("token2 ->  %s  \n", token);
        //         argv2[i] = token;
        //         argvs[x][i] = token;
        //         i++;
        //         if (token && !strcmp(token, "|"))
        //         {
        //             piping = 2;
        //             argv2[i] = NULL;
        //             x++;
        //             break;
        //         }
        //     }
        //     argv2[i] = NULL;
        // }
        // if(piping == 2){
        //     i = 0;
        //     while (token!= NULL)
        //     {
        //         token = strtok (NULL, " ");
        //         printf("token3 ->  %s  \n", token);
        //         argv3[i] = token;
        //         argvs[x][i] = token;
        //         printf("%s : \n", argv3[i]);
        //         i++;
        //     }
        //     argv3[i] = NULL;
        // }
       

        /* prompt change */
        if (!strcmp(argv1[0], "prompt"))
        {

            if (!strcmp(argv1[1], "="))
            {
                char *new_prompt = (char *)malloc(sizeof(argv1[2]));
                strcpy(new_prompt, argv1[2]);
                prompt = new_prompt;
            }
            continue;
        }
        else if (!strcmp(argv1[0], "echo"))
        {

            if (!strcmp(argv1[1], "$?")){

                printf("%d", status);

            }else if(argv1[1][0] == '$'){
                for (int i = 0; i < num_variables; i++)
                {
                    if (!strcmp(names[i], argv1[1]))
                    {
                        // Return variable value
                        printf("%s" ,vales[i]);
                    }
                }
            }
            else{

                for (size_t indx = 1; indx < argc1; ++indx)
                {

                    printf("%s ", argv1[indx]);
                }
            }
            printf("\n");
            continue;
        }
        else if (!strcmp(argv1[0], "cd"))
        {

            if (chdir(argv1[1]) != 0)
            {
                printf("cd: no such file or directory: %s\n", argv1[1]);
                exit(1);
            }
        }
        
        
        /* Does command line end with & */
        if (!strcmp(argv1[argc1 - 1], "&"))
        {
            amper = 1;
            argv1[argc1 - 1] = NULL;
        }
        else
            amper = 0;

        /* redirect */
        if (argc1 > 1)
        {
            if (!strcmp(argv1[argc1 - 2], ">"))
            {
                redirect = 1;
                argv1[argc1 - 2] = NULL;
                outfile = argv1[argc1 - 1];
            }
            else if (!strcmp(argv1[argc1 - 2], "2>"))
            {
                outerr = 1;
                argv1[i - 2] = NULL;
                outfile = argv1[i - 1];
            }
            else if (!strcmp(argv1[argc1 - 2], ">>"))
            {
                argv1[argc1 - 2] = NULL;
                outfile = argv1[argc1 - 1];
                if (access(outfile, F_OK) == 0) {
                    redirect=1;
                } else {
                    concat = 1;
                }
            }
            else
            {
                redirect = 0;
                outerr = 0;
                concat = 0;
            }
        }

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
            else if (outerr)
            {
                // redirect to stderr
                fd = creat(outfile, 0660);
                close(STDERR_FILENO);
                dup(fd);
                close(fd);
            }
            /*case where there is no file opened*/
            else if (concat)
            {
        
                fd = open(outfile, O_RDWR | O_CREAT | O_APPEND, 0660);
                close(STDOUT_FILENO);
                dup(fd);
                close(fd);
                /* stdout is now redirected */
            }
            // else if (pipings)
            // {
            //     printf("Startings pipes! \n");
                // pipe(fildes);
                // if (fork() == 0)
                // {
                //     /* first component of command line */
                //     close(STDOUT_FILENO);
                //     dup(fildes[1]);
                //     close(fildes[1]);
                //     close(fildes[0]);
                //     /* stdout now goes to pipe */
                //     /* child process does command */
                //     execvp(argv1[0], argv1);
                // }
                // /* 2nd command component of command line */
                // close(STDIN_FILENO);
                // dup(fildes[0]);
                // close(fildes[0]);
                // close(fildes[1]);
                // /* standard input now comes from pipe */
                // execvp(argv2[0], argv2);
    //                 int num_pipes = 3;
    // int **pipes = malloc(sizeof(int *) * num_pipes); // array of pipe arrays

    // // Parse arguments and allocate pipes
    // for (int i = 0; i < num_pipes; i++)
    // {
    //     // Allocate pipe
    //     pipes[i] = malloc(sizeof(int) * 2);
    //     if (pipe(pipes[i]) == -1)
    //     {
    //         perror("pipe");
    //         exit(EXIT_FAILURE);
    //     }
    // }

    // // Fork child processes and connect pipes
    // int fd_in = STDIN_FILENO;
    // for (int i = 0; i < num_pipes; i++)
    // {
    //     pid_t pid = fork();
    //     if (pid == -1)
    //     {
    //         perror("fork");
    //         exit(EXIT_FAILURE);
    //     }
    //     else if (pid == 0)
    //     {
    //         // Child process
    //         if (fd_in != STDIN_FILENO)
    //         {
    //             if (dup2(fd_in, STDIN_FILENO) == -1)
    //             {
    //                 perror("dup2");
    //                 exit(EXIT_FAILURE);
    //             }
    //             close(fd_in);
    //         }
    //         if (i != num_pipes - 1)
    //         {
    //             if (dup2(pipes[i][1], STDOUT_FILENO) == -1)
    //             {
    //                 perror("dup2");
    //                 exit(EXIT_FAILURE);
    //             }
    //         }
    //         for (int j = 0; j < num_pipes; j++)
    //         {
    //             close(pipes[j][0]);
    //             close(pipes[j][1]);
    //         }
    //         execvp(argvs[i][0], argvs[i]);
    //         perror("execvp");
    //         exit(EXIT_FAILURE);
    //     }
    //     else
    //     {
    //         // Parent process
    //         close(pipes[i][1]);
    //         fd_in = pipes[i][0];
    //     }
    // }

    // // Wait for all child processes to finish
    // for (int i = 0; i < num_pipes; i++)
    // {
    //     wait(NULL);
    // }

    // // Free memory
    // for (int i = 0; i < num_pipes; i++)
    // {
    //     free(pipes[i]);
    // }
    // free(pipes);

    //         }
            execvp(argv1[0], argv1);
        }
        /* parent continues over here... */
        /* waits for child to exit if required */
        if (amper == 0)
            retid = wait(&status);
    }
}

int main()
{
    signal(SIGINT, sig_handler);
    handler();
} 
