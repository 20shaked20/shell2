# shell2
An extended version of the shell program we made last year

**SHALL.C**
The program executes commands:
1. **ls -l**
2. **ls –l &**
3.  **redirects:** **ls –l > file** Routes output to a file >
                   **ls –l nofile 2> mylo**g write routing to stderr 2>
                   **ls -l >> mylog** Adding to an existing file by >> 
4. **prompt_change** prompt = myprompt Command to change the cursor
5. **echo** command that prints the arguments
6. **echo $?** command Print the status of the last command executed.
7. **change_dir - cd mydir** A command that changes the current working folder of the shell
8. **repeat_last_command - !!** command that repeats the last command
9.  **quit** command to exit from shall
10. **sig_handler - Control-C** The message: You typed Control-C!
11. **my_pipe** - Option to chain up to 50 PIPE commands
12. **new_variable** - Adding **objects** to
13. **read_variable** READ Command - brings the values in the objects
14. **traverse_commands** -  Memory of the last commands (at least 20). Option to browse with the arrows: "up" and "down"
15. **if_then_else_fi** - Flow control support IF/ELSE


