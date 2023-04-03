#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

char *prompt = "hello";

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
int fd, piping, pipings = 0, retid, status, argc1 /*last token indx*/;

/*pipes*/
int ind = 0;
int fildes[3];
char *buffer[256] /*pipe buffer*/;
char command_pipe[1024]; /*pipe command to execute*/

/*remove spaces and '\n'*/
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

/*pipe functionality, is working fine for now need to fix */
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
        }

        if (i != 0)
        {
            close(fd[i - 1][0]);
            close(fd[i - 1][1]);
        }
        wait(NULL);
    }
}

/*this function is responsible for CTRL-C catch and handle*/
void sig_handler()
{
    signal(SIGINT, sig_handler);
    printf("\n You typed Control-C!\n");
    printf("%s: ", prompt);
    fflush(stdout);
}

/*this method gets the command entered by the user in the std and it saves it in the list of commands*/
void get_command_from_std()
{
    printf("%s: ", prompt);
    fgets(command, 1024, stdin);
    command[strlen(command) - 1] = '\0';
    if (command[0] != '\033')
    {
        // dont add and ignore blank commands or arrow ones//
        commands[num_comm] = strdup(command);
        num_comm++;
    }
    piping = 0;
}

/*read function*/
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

/*adds a new variable with the use of $'var' = 'something'*/
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

/*executes the last command that was made by the user*/
void prev_command()
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
            piping = 1;
            break;
        }
    }
}

/*changes the prompt of the user*/
void prompt_change()
{
    if (!strcmp(argv[1], "="))
    {
        char *new_prompt = (char *)malloc(sizeof(argv[2]));
        strcpy(new_prompt, argv[2]);
        prompt = new_prompt;
    }
}

/*the echo function in c*/
void echo()
{
    /* echo $? */
    if (!strcmp(argv[1], "$?"))
    {

        printf("%d", status);

        /* echo ?object */
    }
    else if (argv[1][0] == '$')
    {
        for (int i = 0; i < num_variables; i++)
        {
            if (!strcmp(names[i], argv[1]))
            {
                // Return variable value
                printf("%s", values[i]);
            }
        }
    }
    /* Regular echo */
    else
    {

        for (size_t indx = 1; indx < argc1; ++indx)
        {

            printf("%s ", argv[indx]);
        }
    }
    printf("\n");
}

/*changes the directory of the user*/
void change_dir()
{

    if (chdir(argv[1]) != 0)
    {
        printf("cd: no such file or directory: %s\n", argv[1]);
        exit(1);
    }
}

/*handles the redirects*/
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

/* for commands not part of the shell command language */
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
        get_command_from_std();
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
            if (token && !strcmp(token, "|")) /////* if pips *//////
            {
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
        else if (!strcmp(argv[0], "!!"))
        {
            prev_command();
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

        if (fork() == 0)
        {
            forks();
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