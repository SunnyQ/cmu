# !/usr/bin/env python

import sys
import os

def findMin(res):
    f = open(res, 'r')
    minDWT = -1
    for line in f:
        dwt = line.split()[2]
        if dwt < minDWT or minDWT == -1:
            minDWT = dwt
    f.close()
    return minDWT   
    

def printMinPairs(minDWT, res):
    f = open(res, 'r')
    pool = []
    for line in f:
        e = line.split()
        if minDWT == e[2] \
           and not (e[1], e[0], e[2]) in pool:
            pool.append((e[0], e[1], e[2]))
    
    pool.sort()
    for items in pool:
        print items[0] + " " + items[1] + " "+ items[2]
    
    f.close()

if __name__ == '__main__':
    if (len(sys.argv) > 1):
        res = sys.argv[1]
        print "Most similar pairs are:"
        printMinPairs(findMin(res), res)
    else:	
        print "Usage ./similar.py <result file> \n"
