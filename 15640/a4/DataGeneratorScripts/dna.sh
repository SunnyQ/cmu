#!/bin/bash
#Number of points per cluster
p=10000
#Number of cluster
c=3
#Length of DNA 
l=20

echo ********GENERATING $p INPUT POINTS EACH IN $c CLUSTERS

python ./randomdnaclustergen/generaterawdata.py -c $c  -p $p -l $l -o ../data/dna.input.txt
