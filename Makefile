#
# Makefile for NotifyWord
# Created date : 2012-12-03 20:03:00
# By Phung Van Tu <tuphungvan@gmai.com>
#

#Define compile
CC = gcc

# Header file path for including
CFLAGS = `pkg-config --cflags libnotify`

# Library for linking
LIBS = `pkg-config --libs libnotify`

# Define target program
PROG = NotifyWord

# Define dependence object file 
OBJS = NotifyWord.o

# Top level : All
all : $(PROG)

#Rule for creating target from object files
$(PROG): $(OBJS)
	$(CC) -g -o $(PROG) $(OBJS) $(LIBS)

# Top level : debug
debug:	NotifyWord.c
	$(CC) -g -o $(PROG) NotifyWord.c $(LIBS) $(CFLAGS) -D _DEBUG_MACRO_

# Create every object file
NotifyWord.o: NotifyWord.c
	$(CC) -c NotifyWord.c $(CFLAGS)

# Top level : clean
clean:
	rm -f $(PROG) $(OBJS)