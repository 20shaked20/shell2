CC = gcc
AR = ar
FLAGS = -Wall -g

OBJECTS_Shell = shell.o
SOURCES = shell.c

all: myshell

myshell: $(OBJECTS_Shell) lib_shell.a
	$(CC) $(FLAGS) -o myshell $(OBJECTS_Shell) lib_shell.a 

#Make commands as singles:

lib_shell.a: $(OBJECTS_Shell) #static library for shell
	$(AR) -rcs lib_shell.a $(OBJECTS_Shell)

myshell.o: shell.c shell.h
	$(CC) $(FLAGS) -c shell.c


.PHONY: clean all

clean:
	rm -f *.o *.so *.txt myshell lib_shell.a

