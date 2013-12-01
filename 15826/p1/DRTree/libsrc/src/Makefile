# Makefile for Deferred-split R-tree (DR) library

LIBDIR		= ../../lib
INCDIR		= ../include

PSOBJS		= PS.o PSbuf.o PShash.o PSerror.o
DROBJS		= DR.o DRtree.o DRsplit.o DRnode.o DRrect.o DRerror.o DRknn.o

LIB		= libdr.a

CFLAGS		= -g -I$(INCDIR)
CC		= gcc

all:		$(LIB)

$(LIB):		$(DROBJS) $(PSOBJS)
		ar crv $@ $?
		ranlib $@
		mv $@ $(LIBDIR)

clean:
		rm -f *.o
