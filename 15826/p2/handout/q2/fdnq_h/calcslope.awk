# given file with x-y pairs, it calculates the slope for the linear regression

{   
    x = $1
    y = $2
    x1 += x
    y1 += y
    x2 += x*x
    y2 += y*y
    xy += x*y
    n ++
}
END{
    if( n > 1 ) {
        a = (xy - x1*y1/n ) / ( x2 - x1*x1/n )
        b = (y1 - a * x1 ) / n
        # correlation coefficient r
        r = (xy - x1*y1 /n ) / sqrt( x2 - x1*x1 / n) / sqrt ( y2 - y1*y1 /n)
        print "slope= ", a, "	y_intcpt= ", b, "	corr= ", r
    } else {
        print "slope= ", 9999, "	y_intcpt= ", 0, "	corr= ", 0
    }
}
