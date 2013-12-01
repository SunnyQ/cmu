#!/usr/bin/python

def getPool(filename):
    pool_total = {}
    pool_in = {}
    pool_out = {}
    f = open(filename)
    for line in f:
        valueSplitted = line.split()
        if (len(valueSplitted) < 2):
            continue
        pool_total[valueSplitted[0]] = \
            pool_total.get(valueSplitted[0], 0) + 1
	pool_out[valueSplitted[0]] = \
            pool_out.get(valueSplitted[0], 0) + 1
			
        pool_total[valueSplitted[1]] = \
            pool_total.get(valueSplitted[1], 0) + 1
	pool_in[valueSplitted[1]] = \
            pool_in.get(valueSplitted[1], 0) + 1
    f.close()
    return pool_total, pool_in, pool_out

def printPDF(pool, filename):
    f = open(filename, "w")
    count = {}
    total = 0
    for v in pool.itervalues():
        count[v] = count.get(v, 0) + 1
        total += 1
        
    for k,v in count.iteritems():
        f.write(str(k) + " " + str(float(v) / float(total))+ "\n")
    f.close()
    
def printZipf(pool, filename):
    f = open(filename, "w")
    v = pool.values()
    v.sort(reverse=True)
    i = 0
    while i < len(v):
        f.write(str(i + 1) + " " + str(v[i]) + "\n")
        i += 1
    f.close()


if __name__ == "__main__":
    filename = "patents.txt"
    output1 = "degreeCount_total.txt"
    output2 = "rankDegree_total.txt"
    output3 = "degreeCount_in.txt"
    output4 = "rankDegree_in.txt"
    output5 = "degreeCount_out.txt"
    output6 = "rankDegree_out.txt"
    #print filename
    pool_total, pool_in, pool_out = getPool(filename)
    printPDF(pool_total, output1)
    printZipf(pool_total, output2)
    printPDF(pool_in, output3)
    printZipf(pool_in, output4)
    printPDF(pool_out, output5)
    printZipf(pool_out, output6)
	
	