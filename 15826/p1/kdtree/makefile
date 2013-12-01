PACKAGE= kdtree
# kdtree 
MAIN= ./main
# CFLAGS=-g -ansi -pedantic -DNDEBUG # -DDEBUG # -DNDEBUG
CFLAGS=-g -ansi -pedantic -DNDEBUG # -DDEBUG # -DNDEBUG
SRC= kdtree.c vector.c
OBJ= kdtree.o vector.o
SRCALL= $(MAIN).c $(SRC)
ALLH= dfn.h kdtree.h vector.h
CC= gcc
SCRIPTS= input.script inp-large.script
COPYRIGHT= COPYRIGHT
REV=
.IGNORE:
# .SILENT:
LPR= lpr

hw1_1: $(MAIN) hw1_1.script
	echo "**** HOMEWORK 1, DATASET 1 ***"
	$(MAIN) -d 2 < hw1_1.script

hw1_2: $(MAIN) hw1_2.script
	echo "**** HOMEWORK 1, DATASET 2 ***"
	$(MAIN) -d 3 < hw1_2.script

hw1_3: $(MAIN) hw1_3.script
	echo "**** HOMEWORK 1, DATASET 3 ***"
	$(MAIN) -d 4 < hw1_3.script

demo: $(MAIN) input.script
	echo "**** DEMO ***"
	$(MAIN) -d 4 < input.script

largedemo: $(MAIN) inp-large.script
	echo " ***large demo***"
	$(MAIN) -d 2 < inp-large.script

$(MAIN): $(MAIN).o $(OBJ)
	$(CC) $(CFLAGS) -o $(MAIN) $(MAIN).o $(OBJ) -lm

p$(MAIN): $(MAIN).o $(OBJ)
	purify $(CC) $(CFLAGS) -o $(MAIN) $(MAIN).o $(OBJ) -lm

vector.o: dfn.h vector.h

kdtree.o: dfn.h kdtree.h vector.h

$(MAIN).o: dfn.h kdtree.h vector.h kdtree.c

checkout:
	co -l $(REV) $(SRCALL) $(ALLH) README 

checkin:
	ci -l $(REV)  -f $(SRCALL) $(ALLH) README

cleanup:
	\rm -f $(OBJ) $(MAIN).o core

clean: cleanup

spotless: clean
	\rm -f $(MAIN) $(PACKAGE).bundle $(PACKAGE).tar* uu
	\rm -rf TST

$(PACKAGE).tar: $(SRCALL) $(ALLH) README makefile \
	$(SCRIPTS) $(COPYRIGHT)
	tar cvfh $(PACKAGE).tar $(SRCALL) $(ALLH) \
	$(SCRIPTS) README makefile $(COPYRIGHT)

all.tar: $(PACKAGE).tar
	cp $(PACKAGE).tar all.tar 

printout: $(SRCALL) $(ALLH)
	$(LPR) $(SRCALL) $(ALLH)

uu: $(PACKAGE).tar
	gzip $(PACKAGE).tar
	uuencode $(PACKAGE).tar.gz $(PACKAGE).tar.gz > uu
