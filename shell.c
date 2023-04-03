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
char command_pipe[1024];
char previous_command[1024];
char *token;
int i ,x;
char *outfile;
char *buffer[256];
int ind = 0;
int amper, redirect, concat /*open new file*/, outerr /*related to stderr*/;
int fd, piping, pipings = 0, retid, status, argc1 /*last token indx*/;
int fildes[3];
char *argv1[10], *argv2[10], *argv3[10], *argv4[10],  *commands[100];
int num_comm=0;


//remove spaces and '\n' 
void my_remove(char* conn){
	if(conn[strlen(conn)-1]==' ' || conn[strlen(conn)-1]=='\n')
	conn[strlen(conn)-1]='\0';
	if(conn[0]==' ' || conn[0]=='\n'){
         memmove(conn, conn+1, strlen(conn));
    }
}

void dev(char** buffer,int *n,char *bf,const char *ch){
	char *tk;
	tk = strtok(bf,ch);
	int place=-1;
	while(tk){
		buffer[++place]=malloc(sizeof(tk)+1);
		strcpy(buffer[place],tk);
		my_remove(buffer[place]);
		tk = strtok(NULL,ch);
	}
	buffer[++place]=NULL;
	*n=place;
}

void my_pipe(char** buf,int index){ 
	if(index>50){
         return;
    }

	int fd[50][2]; // 50 pipes
    int indx;
	char *array[50];

	for(int i=0;i<index;i++){
		dev(array,&indx,buf[i]," ");

		if(i!=index-1){
			if(pipe(fd[i])<0){
				printf("ERROR!");
				return;
			}
		}
		if(fork()==0){
			if(i!=index-1){
				dup2(fd[i][1],STDOUT_FILENO);
				close(fd[i][0]);
				close(fd[i][1]);
			}

			if(i!=0){
				dup2(fd[i-1][0],STDIN_FILENO);
				close(fd[i-1][1]);
				close(fd[i-1][0]);
			}
			execvp(array[0],array);
		}
        
		if(i!=0){ 
			close(fd[i-1][0]);
			close(fd[i-1][1]);
		}
		wait(NULL);
	}
}


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
        //// save the commnd ////
        printf("%s: ", prompt);
        fgets(command, 1024, stdin);
        command[strlen(command) - 1] = '\0';
        commands[num_comm] = strdup(command);
        num_comm++;
        piping = 0;
        char ch = command[0];

        //// if is object save ////
        if (ch == '$'){
            i = 0;
            token = strtok(command, " ");
            while (token != NULL)
            {   
                if(i == 0){
                names[num_variables] = strdup(token);
                token = strtok(NULL, " ");
                }
                if(i == 1){
                argv4[1] = strdup(token);
                token = strtok(NULL, " ");
                }
                if(i == 2){
                vales[num_variables] =  strdup(token); 
                token = strtok(NULL, " ");  
                }            
                i++;
            }   
            num_variables++;
        }
        /* parse command line */
        i = 0;
        strcpy(command_pipe, command);
        token = strtok(command, " ");
        while (token != NULL)
        {
           
            argv1[i] = token;
            token = strtok(NULL, " ");
            i++;
            if (token && !strcmp(token, "|")) /////* if pips *//////
            {
               dev(buffer, &ind, command_pipe, "|");
               my_pipe(buffer, ind);
               break;
            }
        }
        ///////////////////////////////////////////////////////////////
        argv1[i] = NULL;
        argc1 = i;

        /* Is command empty */
        if (argv1[0] == NULL)
            continue;
        
        //////*read*/////
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

        ////////* !! *///////
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

         ////////* echo *///////
        else if (!strcmp(argv1[0], "echo"))
        {
            /* echo $? */
            if (!strcmp(argv1[1], "$?")){

                printf("%d", status);
            
            /* echo ?object */
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
            /* Regular echo */
            else{

                for (size_t indx = 1; indx < argc1; ++indx)
                {

                    printf("%s ", argv1[indx]);
                }
            }
            printf("\n");
            continue;
        }

         //////* cd *////////
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
             //////* > *////////
            if (!strcmp(argv1[argc1 - 2], ">"))
            {
                redirect = 1;
                argv1[argc1 - 2] = NULL;
                outfile = argv1[argc1 - 1];
            }
             //////* 2> *////////
            else if (!strcmp(argv1[argc1 - 2], "2>"))
            {
                outerr = 1;
                argv1[i - 2] = NULL;
                outfile = argv1[i - 1];
            }
             //////* >> *////////
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
