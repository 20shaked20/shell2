/**
 * 07-Apr-2023
 * Authors : Shaked levi, Lior nagar
 * Header file for the shell.c application
 *
 */

/**
 * Gets a char and removes any \n of it.
 */
void my_remove(char *conn);

/**
 * Splits the pipe command into a separte cells ->
 * | cmd 1 | cmd 2  | cmd 3 | ... |
 */
void dev(char **buffer, int *n, char *bf, const char *ch);

/**
 * Pipe functionality limits to 50 pipes,
 * can alter it to support any amount of pipes.
 */
void my_pipe(char **buf, int index);

/**
 * This function is responsible for CTRL-C catch and handle.
 */
void sig_handler();

/**
 * This method gets the command entered by the user in the std and it saves it in the list of commands
 */
void get_command_from_std();

/**
 * This method allows the user to input "read" and then a variable
 * in order to save data written to it inside the variable.
 */
void read_variable();

/**
 * Adds a new variable with the use of echo $'var' = 'data'
 */
void new_variable();

/**
 * This method is responsbile to hanlde the up and down arrow keys in order to search commands in previous order.
 * Use -> press up/down key + enter will show the desired command.
 */
void traverse_commands();

/**
 * changes the prompt of the user
 * Use -> prompt = "new prompt"
 */
void prompt_change();

/**
 * This method is responsible to handle 'echo'
 * couple of uses:
 * To save a variable : echo $var = data
 * To read var : echo var;
 * To repeat saying : echo bla bla
 * View status of last command : echo $?
 */
void echo();

/**
 * changes the directory of the user
 * Use -> cd "folder name"
 */
void change_dir();

/**
 * This method handles the redirects
 * > input to a given file
 * 2> input to file stderr
 * >> input to a file and if file does not exists open one
 * */
void redirects();

/**
 * executes the last command that was made by the user by entering "!!"
 */
void repeat_last_command();

/**
 * This method is responsible to handle a if-then-else-fi input to our shell.
 * if - the condition to check
 * then - if the condition is True, do this command
 * else - if the condition is False, do this command
 * fi - end.
*/
void if_then_else_fi();

/**
 * This method handles all the forks required for this project
 * they are for commands not part of the shell command language 
 * */
void forks();

/**
 * The brain of our project
 * where we decide to which method to call, and how to operate via given inputs to the std.
*/
void shell();

