#!/bin/sh

np=2
machinelist=machinefile.list
input=../../data/cluster.csv
initial_centroids=../../data/centroids.csv
p=30
c=3

mpirun -np $np -machinefile $machinelist main -f $input -i $initial_centroids -p $p -c $c
