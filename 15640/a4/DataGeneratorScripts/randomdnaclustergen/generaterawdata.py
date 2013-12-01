import sys
import getopt
import math
from random import choice, sample

def usage():
    print '$> python generaterawdata.py <required args> [optional args]\n' + \
        '\t-c <#>\t\tNumber of clusters to generate\n' + \
        '\t-p <#>\t\tNumber of DNA per cluster\n' + \
        '\t-o <file>\tFilename for the output of the raw data\n' + \
        '\t-l [#]\t\tLength of each DNA strand\n' 

def handleArgs(args):
    # set up return values
    numClusters = -1
    numPoints = -1
    output = None
    lenStrand = -1

    try:
        optlist, args = getopt.getopt(args[1:], 'c:p:l:o:')
    except getopt.GetoptError, err:
        print str(err)
        usage()
        sys.exit(2)

    for key, val in optlist:
        # first, the required arguments
        if key == '-c':
            numClusters = int(val)
        elif key == '-p':
            numPoints = int(val)
        elif key == '-o':
            output = val
        elif key == '-l':
            lenStrand = int(val)

    # check required arguments were inputted  
    if numClusters < 0 or numPoints < 0 or \
            lenStrand < 0 or output is None:
        usage()
        sys.exit()
    return (numClusters, numPoints, output, lenStrand)

def drawStrand(lenStrand):
    bases = ['A', 'G', 'C', 'T']
    strand = ''
    for i in range(lenStrand):
        strand += choice(bases)
    return strand

# start by reading the command line
numClusters, \
numPoints, \
output, \
lenStrand = handleArgs(sys.argv)

f = open(output, "w")

# step 2: generate the strands for each centroid
for i in range(0, numClusters):
    for j in range(0, numPoints):
        strand = drawStrand(lenStrand)
        # write the points out
        f.write(strand)
        f.write('\n')
