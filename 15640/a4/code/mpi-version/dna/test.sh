#!/bin/sh

np=2
machinelist=machinefile.list
input=../../data/dna.input.txt
initial_centroids=../../data/dna.centroids.txt
p=30000
c=3
l=20

mpirun -np $np -machinefile $machinelist main -f $input -i $initial_centroids -p $p -c $c -l $l
