CFLAGS=-g
SRCALL= killFlats.pl calcslope calcslope.awk \
     fdnq.pl mkboxcount.pl loglog.pl
INP= diagonal.inp square5K.inp sierpinski5K.inp \
     mf_N1k_b08_k10.inp mf_N1k_b08_k10_HISTOGRAM.inp \
	 diagonal_HISTOGRAM.inp
DOC= README.fdnq
REV=

alldemo: powerdemo demoHist

fulldemo: mkxFlag
	perl fdnq.pl -q2 -v -r0.1 -R1000 diagonal.inp

demo: mkxFlag
	perl fdnq.pl diagonal.inp

mkxFlag: $(SRCALL)
	chmod u+x $(SRCALL)
	touch mkxFlag

powerdemo: mkxFlag
	perl fdnq.pl -q2  -r0.1 -R1000 diagonal.inp
	perl fdnq.pl -q2  -r0.1 -R1000 sierpinski5K.inp
	perl fdnq.pl -q0  -r0.1 -R1000 square5K.inp
	perl fdnq.pl -q3 -v -r0.1 -R1000 square5K.inp
	
demoHist: mkxFlag
	perl fdnq.pl      -q2  -r1 -R10000 mf_N1k_b08_k10.inp
	perl fdnq.pl -h   -q2  -r1 -R10000 mf_N1k_b08_k10_HISTOGRAM.inp
	perl fdnq.pl -h   -q2  -r1 -R10000 diagonal_HISTOGRAM.inp


all.bundle: $(SRCALL) $(INP) makefile $(DOC)
	bundle $(SRCALL) $(INP) $(DOC) makefile > all.bundle

all.tar: $(SRCALL) $(INP) makefile $(DOC)
	tar cvf all.tar  $(SRCALL) $(INP) $(DOC) makefile

all.zip: $(SRCALL) $(INP) makefile $(DOC)
	zip all.zip  $(SRCALL) $(INP) $(DOC) makefile

clean:
	\rm -f *.points

spotless: clean
	\rm -f mkxFlag

