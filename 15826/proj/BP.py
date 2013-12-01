#!/usr/bin/python

import os

def main():
    os.system("rm -rf output")
    os.system("pig -x local Init.pig")
    os.system("pig -x local Update.pig")
    
    while numLines("output/finishFlag/part-r-00000") != 43128:
        os.system("rm -rf output/prev")
        os.system("mv output/updated output/prev")
        os.system("pig -x local Update.pig")
    
    os.system("pig -x local BP.pig")

def numLines(filename):
    if (os.path.exists(filename)):
        f = open(filename)
        numLines = 0
        for line in f:
            for word in line.split("\t"):
                if word.strip() == "done":
                    numLines += 1
        print "========================="
        print numLines
        print "========================="
        f.close()
        
        os.system("rm -rf output/finishFlag")
        return numLines
    else:
        return 0

if __name__ == "__main__":
    main()
