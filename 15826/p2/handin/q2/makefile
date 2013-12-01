# From Kai/christos:
# 	You may use any programming language you want 
#	as long as it runs on the Andrew linux machines
#	In that case, please change the makefile accordingly

all:
		g++ -o fractal fractal.cc
		./fractal 3
		python ./draw_surface.py ./output_3.txt
		\mv fractal.png fractal_3.png
		./fractal 4
		python ./draw_surface.py ./output_4.txt
		mv fractal.png fractal_4.png	

clean:
	\rm -f *.png output.txt fractal

spotless: clean
