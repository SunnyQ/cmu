################################################################################
# Makefile                                                                     #
#                                                                              #
# Description: This file contains the make rules for Project 1.                #
#                                                                              #
# Author: Yang Sun <yksun@cs.cmu.edu>                                          #
#                                                                              #
################################################################################

CC = gcc
CFLAGS = -g -Wall -Werror

all: lisod

lisod.o: lisod.c lisodlib.h lisodlog.h errno.h
	$(CC) $(CFLAGS) -c lisod.c

lisodlib.o: lisodlib.c lisodlib.h lisodlog.h errno.h
	$(CC) $(CFLAGS) -c lisodlib.c
	
lisodlog.o: lisodlog.c lisodlog.h errno.h
	$(CC) $(CFLAGS) -c lisodlog.c
	
lisod: lisod.o lisodlib.o lisodlog.o

clean:
	rm -f *~ *.o lisod

