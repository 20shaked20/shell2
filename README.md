
# Shell
This is an extended version of the shell program we made last year. </br>

This is a sample of how a shell is operating under the linux os. </br>
We recreated some of the methods that shell supports in our own way. </br>
In this shell we also allowed pipe supporting and if-then-else-fi support. </br>


## How To Use

* clone the folder to a relevant workspace in linux </br>
* make sure you're in the right directory </br> 
* run the command "make/ make all" in terminal which will create the program to run. </br>
* inside the terminal where you created the outfile named "myshell" execute - "./myshell" </br>
* Welcome to the shell program, start testing it! </br>

## Command Support
Our shell program supports multiple commands, including piping. </br>
Listed below are some of the methods we were requsted to rewrite and you can execute to test them : </br>
</br>
1. ``ls -l`` -> shows the current directory with some details of it
2. ``ls –l &``
3.  **Redirects:**
* ``ls –l > file`` -> Routes output to a file >.
* ``ls –l nofile 2> mylog`` -> write routing to stderr 2>.
* ``ls -l >> mylog`` -> Adding to an existing file by >>.
4. ``prompt = newprompt``-> Changes the prompt (cursor).
5. ``echo somt`` Prints the arguments.
6. ``echo $?`` Print the status of the last command executed.
7. ``cd mydir`` Changes the current working folder of the shell.
8. ``!!`` Repeats the last command.
9.  ``quit`` Exit from shell.
10. ``Control-C`` Pressing Ctrl-C will input a message "You Typed Control-C".
11. ``cmd1| cmd2 | ... | cmd 50`` - Option to chain up to 50 PIPE commands. 
12. ``$shaked = bla`` - Allows objectifing variables with data. (here shaked will be contain bla).
13. ``read shaked john kk`` READ Command - brings the values in the first variable,(in this example, anything you put inside shaked will be saved there).
14. ``up arrow key + enter / down arrow key + enter `` -  Memory of the last commands (at least 20), will allow to go up/down the commands and execute them.
15. ``if ... then ... else ... fi`` - Flow control support IF/ELSE
16. Anything not listed here, will be executed by the system, so feel free to try other commands like - ``clear``, ``cat``, ``grep``, etc...


