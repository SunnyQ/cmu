# top-level Makefile for Deferred-split R-tree (DR)
#.IGNORE:
demo: all
	bin/DRmain -d 3 < input
	bin/DRmain -d 3 < s.input
	bin/DRmain -d 3 < k.input

hw1: all
	bin/DRmain -d 4 < hw1.input

# Make all or clean...
all clean:
		cd libsrc/src; make $@
		cd binsrc; make $@

spotless: clean
	\rm -f default.idx
	\rm -f bin/DRmain
	\rm -f lib/libdr.a

# Make binary only...
binary:
		cd binsrc; make clean; make all

# Make library only...
library:
		cd libsrc/src; make clean; make all

# Make tar...
tar:
		tar cvFFf ../DRtree.tar .

