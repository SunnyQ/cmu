# Makefile for Deferred-split R-tree (DR) executable

# Directories...
BINDIR		= ../bin
LIBDIR		= ../lib
INCDIR		= ../include

# Aliases...
LIB		= dr
MAIN		= DRmain

# Link Flags...
LDLIBS		= -l$(LIB) -lm
LDFLAGS		= -L$(LIBDIR)

# Compile Flags...
CFLAGS		= -g -I$(INCDIR)
CC		= gcc

# Targets...
all:		$(MAIN)
		mv $? $(BINDIR)

clean:
		rm -f *.o
