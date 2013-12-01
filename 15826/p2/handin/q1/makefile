# From Kai/Christos: 
#     You may can use any programming language you want 
#     as long as it runs on the Andrew linux machines.
#     In that case, change the makefile accordingly

# Please, do NOT include the data file in your hand-in tar-archive


all:
		g++ -o timewarp ./timewarp.cc
		./q1_driver.sh
		python similar.py hw2.q1.output.txt

clean:
	\rm -f *.o *.pyc

# delete all derived files
spotless: clean
	\rm -f *.o timewarp hw2.q1.output.txt
