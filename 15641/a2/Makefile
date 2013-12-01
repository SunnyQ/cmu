################################################################################
# Makefile                                                                     #
#                                                                              #
# Description: This file contains the make rules for Project 2.                #
#                                                                              #
# Author: Yang Sun <yksun@cs.cmu.edu>                                          #
#                                                                              #
################################################################################

CC = gcc
CFLAGS = -g -Wall -Werror

all: routed

routed.o: routed.c routed.h ospf.h rTable.h
	$(CC) $(CFLAGS) -c routed.c
	
rTable.o: rTable.c rTable.h
	$(CC) $(CFLAGS) -c rTable.c
	
ospf.o: ospf.c ospf.h
	$(CC) $(CFLAGS) -c ospf.c
	
routed: routed.o ospf.o rTable.o

clean:
	rm -f *~ *.o routed

