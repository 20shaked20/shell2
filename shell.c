#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#include "shell.h"

char *prompt = "hello:";

char *names[10], *values[10]; /*those are used in a manner of 'map' in order to save the names in read and $*/
int num_variables = 0;        /*the amount of saved variables*/

char command[1024];          /*the full line command written by the user*/
char previous_command[1024]; /*previous line command written by the user*/
char *argv[10] /*a command to execute*/, *argv4[10], *commands[100];
int num_comm = 0 /*the amount of saved commands in the array*/;

char *token;
char *outfile;
int i;

/*flags*/
int amper, redirect, concat /*open new file*/, outerr /*related to stderr*/;
int fd, pipe_flag /*check if pipe is currently in progress*/, retid, status, argc1 /*last token indx*/;

/*pipes*/
int ind = 0;
int fildes[3];
char *buffer[256] /*pipe buffer*/;
char command_pipe[1024]; /*pipe command to execute*/

/*if-else-fi*/
char then_command[1024];
char do_command[1024];
char else_command[1024];
char else_do_command[1024];
char fi_command[1024];

int flag_then = 0; // if
int flag_else = 0; // else

void my_remove(char *conn)
{
    if (conn[strlen(conn) - 1] == ' ' || conn[strlen(conn) - 1] == '\n')
        conn[strlen(conn) - 1] = '\0';
    if (conn[0] == ' ' || conn[0] == '\n')
    {
        memmove(conn, conn + 1, strlen(conn));
    }
}

void dev(char **buffer, int *n, char *bf, const char *ch)
{
    char *tk;
    tk = strtok(bf, ch);
    int place = -1;
    while (tk)
    {
        buffer[++place] = malloc(sizeof(tk) + 1);
        strcpy(buffer[place], tk);
        my_remove(buffer[place]);
        tk = strtok(NULL, ch);
    }
    buffer[++place] = NULL;
    *n = place;
}

void my_pipe(char **buf, int index)
{
    if (index > 50)
    {
        return;
    }

    int fd[50][2]; // 50 pipes
    int indx;
    char *array[50];

    for (int i = 0; i < index; i++)
    {
        dev(array, &indx, buf[i], " ");

        if (i != index - 1)
        {
            if (pipe(fd[i]) < 0)
            {
                printf("ERROR!");
                return;
            }
        }
        /*child*/
        if (fork() == 0)
        {
            if (i != index - 1)
            {
                dup2(fd[i][1], STDOUT_FILENO);
                close(fd[i][0]);
                close(fd[i][1]);
            }

            if (i != 0)
            {
                dup2(fd[i - 1][0], STDIN_FILENO);
                close(fd[i - 1][1]);
                close(fd[i - 1][0]);
            }
            execvp(array[0], array);
            exit(1);
        }
        /*parent*/
        if (i != 0)
        {
            close(fd[i - 1][0]);
            close(fd[i - 1][1]);
        }
        wait(NULL);
    }
}

void sig_handler()
{
    signal(SIGINT, sig_handler);
    printf("\n You typed Control-C!\n");
    printf("%s ", prompt);
    fflush(stdout);
}

void get_command_from_std()
{
    printf("%s ", prompt);
    fgets(command, 1024, stdin);
    command[strlen(command) - 1] = '\0';
    if (command[0] != '\033')
    {
        // dont add and ignore blank commands or arrow ones//
        commands[num_comm] = strdup(command);
        num_comm++;
    }
}

void read_variable()
{
    char *str1 = malloc(strlen(argv[0]) + 2); // +2 for "$" and null terminator
    sprintf(str1, "$%s", argv[1]);            // use sprintf to concatenate strings
    names[num_variables] = strdup(str1);
    char command2[1024];
    fgets(command2, 1024, stdin);
    command2[strlen(command2) - 1] = '\0';
    values[num_variables] = strdup(command2);
    num_variables++;
    free(str1); // free memory allocated by malloc
}

void new_variable()
{
    i = 0;
    token = strtok(command, " ");
    while (token != NULL)
    {
        if (i == 0)
        {
            names[num_variables] = strdup(token);
            token = strtok(NULL, " ");
        }
        if (i == 1)
        {
            argv4[1] = strdup(token);
            token = strtok(NULL, " ");
        }
        if (i == 2)
        {
            values[num_variables] = strdup(token);
            token = strtok(NULL, " ");
        }
        i++;
    }
    num_variables++;
}

void traverse_commands()
{

    char buf2[1024];
    switch (command[2])
    { // the real value
    case 'A':
        num_comm -= 1; // remove one location from the current function//
        // Print the command to the terminal
        printf("Command to execute: %s\n", commands[num_comm]);

        // Wait for the user to press Enter
        printf("Press Enter to execute the command...");
        while (fgets(buf2, sizeof(buf2), stdin) != NULL)
        {
            if (strcmp(buf2, "\n") == 0)
            {
                break;
            }
        }
        system(commands[num_comm]);
        break;
    case 'B':
        // code for arrow down
        num_comm += 1; // add one location from the current function//
                       // Print the command to the terminal
        printf("Command to execute: %s\n", commands[num_comm]);

        // Wait for the user to press Enter
        printf("Press Enter to execute the command...");
        while (fgets(buf2, sizeof(buf2), stdin) != NULL)
        {
            if (strcmp(buf2, "\n") == 0)
            {
                break;
            }
        }
        system(commands[num_comm]);
        break;
    }
}

void prompt_change()
{
    if (!strcmp(argv[1], "="))
    {
        char *new_prompt = (char *)malloc(sizeof(argv[2]));
        strcpy(new_prompt, argv[2]);
        prompt = new_prompt;
    }
}

void echo()
{
    /* echo $? */
    if (!strcmp(argv[1], "$?"))
    {
        printf("%d ", status);
    }
    /* echo ?object */
    if (argv[1][0] == '$')
    {
        for (int i = 0; i < num_variables; ++i)
        {
            if (!strcmp(names[i], argv[1]))
            {
                // Return variable value
                printf("%s ", values[i]);
            }
        }
    }
    /* Regular echo */
    if (argv[1][0] != '$' && strcmp(argv[1], "$?"))
    {
        for (int i = 1; i < argc1; ++i)
        {
            printf("%s ", argv[i]);
        }
    }
    printf("\n");
}

void change_dir()
{

    if (chdir(argv[1]) != 0)
    {
        printf("cd: no such file or directory: %s\n", argv[1]);
        exit(1);
    }
}

void redirects()
{
    //////* > *////////
    if (!strcmp(argv[argc1 - 2], ">"))
    {
        redirect = 1;
        argv[argc1 - 2] = NULL;
        outfile = argv[argc1 - 1];
    }
    //////* 2> *////////
    else if (!strcmp(argv[argc1 - 2], "2>"))
    {
        outerr = 1;
        argv[i - 2] = NULL;
        outfile = argv[i - 1];
    }
    //////* >> *////////
    else if (!strcmp(argv[argc1 - 2], ">>"))
    {
        argv[argc1 - 2] = NULL;
        outfile = argv[argc1 - 1];
        if (access(outfile, F_OK) == 0)
        {
            redirect = 1;
        }
        else
        {
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

void repeat_last_command()
{
    strcpy(command, previous_command);
    /* parse command line */
    i = 0;
    token = strtok(command, " ");
    while (token != NULL)
    {
        argv[i] = token;
        token = strtok(NULL, " ");
        i++;
        if (token && !strcmp(token, "|"))
        {
            pipe_flag = 1;
            dev(buffer, &ind, command_pipe, "|");
            my_pipe(buffer, ind);
            break;
        }
    }
    argv[i] = NULL;
    argc1 = i;
    pipe_flag = 0;
}

void if_then_else_fi()
{
    char *argv_if[10];

    /*extract  data*/
    char cmd[1024];
    memset(cmd, 0, sizeof(cmd)); // set all elements to zero
    // build the command to run in system, and check if its good//
    for (int i = 1; i < argc1; ++i)
    {
        strcat(cmd, argv[i]);
        strcat(cmd, " ");
    }

    fgets(then_command, 1024, stdin); // then
    fgets(do_command, 1024, stdin);

    fgets(else_command, 1024, stdin); // else
    fgets(else_do_command, 1024, stdin);

    fgets(fi_command, 1024, stdin); // fi

    if (!strcmp(then_command, "then\n") && !strcmp(else_command, "else\n") && !strcmp(fi_command, "fi\n"))
    {
        if (!system(cmd))
        {
            // want to execute do command//
            flag_then = 1;
        }
        else
        {
            // want to execute else_do_command//
            flag_else = 1;
        }
    }
    else
    {
        printf("Bad syntax in if-then-else-fi, enter again\n");
    }
}

void forks()
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
    execvp(argv[0], argv);
}

void shell()
{

    signal(SIGINT, sig_handler);

    while (1)
    {
        pipe_flag = 0;

        if (argv[0] != NULL)
        {
            strcpy(previous_command, "");
            for (size_t i = 0; argv[i] != NULL; i++)
            {
                strcat(previous_command, argv[i]);
                strcat(previous_command, " ");
            }
        }
        /*saving the commnd entered by the user*/
        if (flag_then == 1) // if-then command//
        {
            strcpy(command, do_command);
            command[strlen(command) - 1] = '\0';
            if (command[0] != '\033')
            {
                // dont add and ignore blank commands or arrow ones//
                commands[num_comm] = strdup(command);
                num_comm++;
            }
            flag_then = 0;
        }
        else if (flag_else == 1) // else command//
        {
            strcpy(command, else_do_command);
            command[strlen(command) - 1] = '\0';
            if (command[0] != '\033')
            {
                // dont add and ignore blank commands or arrow ones//
                commands[num_comm] = strdup(command);
                num_comm++;
            }
            flag_else = 0;
        }
        else // std command from normal type.
        {
            get_command_from_std();
        }
        char ch = command[0];

        /*if is object save*/
        if (ch == '$')
        {
            new_variable();
        }
        else if (ch == '\033')
        {
            traverse_commands();
            continue;
        }

        /* parse command line */
        i = 0;
        strcpy(command_pipe, command);
        token = strtok(command, " ");
        while (token != NULL)
        {
            argv[i] = token;
            token = strtok(NULL, " ");
            i++;
            if (token && !strcmp(token, "|") && strcmp(argv[0], "if") /*in case this is happens, we dont want it to do pipes in the first time, because the system will handle it.*/) /////* if pips *//////
            {
                pipe_flag = 1; // pipe is in progress//
                dev(buffer, &ind, command_pipe, "|");
                my_pipe(buffer, ind);
                break;
            }
        }
        argv[i] = NULL;
        argc1 = i;

        /* Is command empty */
        if (argv[0] == NULL)
            continue;
        //

        if (!strcmp(argv[0], "read"))
        {
            read_variable();
            continue;
        }

        //

        if (!strcmp(argv[0], "quit"))
        {
            exit(0);
        }
        else if (!strcmp(argv[0], "!!")) // make this an outside method later//
        {
            repeat_last_command();
        }

        if (!strcmp(argv[0], "if"))
        {
            if_then_else_fi();
            continue;
        }

        //

        if (!strcmp(argv[0], "prompt"))
        {
            prompt_change();
            continue;
        }
        else if (!strcmp(argv[0], "echo"))
        {
            echo();
            continue;
        }
        else if (!strcmp(argv[0], "cd"))
        {
            change_dir();
        }

        /* Does command line end with & */
        if (!strcmp(argv[argc1 - 1], "&"))
        {
            amper = 1;
            argv[argc1 - 1] = NULL;
        }
        else
            amper = 0;

        /* redirect */
        if (argc1 > 1)
        {
            redirects();
        }
        if (pipe_flag == 0)
        {
            if (fork() == 0)
            {
                forks();
            }
        }
        else
        {
            pipe_flag = 0;
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
    shell();
}