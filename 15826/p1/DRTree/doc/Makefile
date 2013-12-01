MAIN= manual
REV = 
LPR = qpr -q ps3

$(MAIN).dvi: $(MAIN).tex
	latex $(MAIN).tex
	latex $(MAIN).tex

print: $(MAIN).ps
	$(LPR) $(MAIN).ps

$(MAIN).ps: $(MAIN).dvi
	dvips $(MAIN).dvi > $(MAIN).ps

clean:
	\rm -f *.dvi $(MAIN).ps *.bbl *.aux *.log *.blg *.toc

spotless: clean
	\rm -f all.tar


all.tar: $(MAIN).tex makefile
	tar cvf all.tar $(MAIN).tex makefile
