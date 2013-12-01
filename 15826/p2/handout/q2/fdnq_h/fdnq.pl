: # Use perl
   eval 'exec perl -S $0 "$@"'
   if 0;

# it is faster to use something like
#!/usr/local/bin/perl5 
# but this is more portable.

#
# $Log:	fdnq.pl,v $
# Revision 1.5  99/08/05  01:22:13  christos
# *** empty log message ***
# 
# Revision 1.4  99/08/05  00:59:23  christos
# *** empty log message ***
# 
#
#########################################################
# expects an input file, with new-line separated records
# calls the mkboxcount.pl
# and computes the slope of the log-log plot


# use Carp::Assert;

$hist=0;    # whether the input file is a histogram: 
            #       x1 x2 ... xn value
            # as opposed to just x1 x2 ... xn
$verbose = 0;
$debug = 0;
$q = 0;
$rmin = 0.0000001;
$rmax = 1000000;
$factor = 2;
# $factor = 1.3;

while( $_ = $ARGV[0], /^-/){
   shift;
   last if /^--$/;
   if( /^-D(.*)/){ $debug = $1 }
   if( /^-r(.*)/){ $rmin = $1 }
   if( /^-R(.*)/){ $rmax = $1 }
   if( /^-f(.*)/){ $factor = $1 }
   if( /^-q(.*)/){ $q = $1 }
   if( /^-h/)    { $hist=1 }
   if (/^-v/)    { $verbose++ }
}


if ($q == 1 ) { die "q=1 is not implemented yet\n";}

$nargs = $#ARGV + 1;   # the number of remaining arguments

if( $nargs != 1 ) {
   die "USAGE: $0 [-v] [-q<qval>] " .
   " [-f<factor>] " .
   " [-r<rmin>] [-R<rmax>] <datafile>"  }


$inp = $ARGV[0];

if( $hist ){
    `perl mkboxcount.pl -h  -f$factor -r$rmin -R$rmax -q$q $inp > ${inp}.points`;
}else{
    `perl mkboxcount.pl     -f$factor -r$rmin -R$rmax -q$q $inp > ${inp}.points`;
}

if( $verbose > 0 ) {
   `xgraph -lnx -lny ${inp}.points`;
}

# assert( ( 0 == $q) || ( 2 == $q ) );
print  STDERR "*** need to divide by ", $q-1 , " = (q-1)\n"
     unless ( ( $q == 0) || ( $q == 2) ) ;

# print `cat ${inp}.points | loglog.pl | choptop | chopbottom | csh calcslope `;
print `cat ${inp}.points | perl loglog.pl | perl killFlats.pl | csh calcslope `;


