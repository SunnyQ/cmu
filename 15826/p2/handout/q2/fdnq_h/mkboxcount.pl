: # Use perl
   eval 'exec perl -S $0 "$@"'
   if 0;

# it is faster to use something like
#!/usr/local/bin/perl5 
# but this is more portable.

#
# $Log:	mkboxcount.pl,v $
# Revision 1.5  99/08/05  00:59:23  christos
# *** empty log message ***
# 
# Revision 1.4  99/08/05  00:55:30  christos
# works for any q!=1 - uses a hash table to keep counts (O(N)!)
# 
#
##########################################################
# expects an input file, with new-line separated records
# computes the box-count-plot

$hist = 0;      # hist=1 means that the last column contains 'values':
                #    x1 x2 ... xn value
$verbose = 0;
$debug = 0;
$q = 0;
$rmin = 0.0000001;
$rmax = 1000000;
$factor = 1.3;

while( $_ = $ARGV[0], /^-/){
   shift;
   last if /^--$/;
   if( /^-D(.*)/){ $debug = $1 }
   if( /^-r(.*)/){ $rmin = $1 }
   if( /^-R(.*)/){ $rmax = $1 }
   if( /^-f(.*)/){ $factor = $1 }
   if( /^-q(.*)/){ $q = $1 }
   if (/^-v/)    { $verbose++ }
   if (/^-h/)    { $hist=1 }
   if (/^-g/)    { $graph=1 }
}

$nargs = $#ARGV + 1;   # the number of remaining arguments

if( $nargs != 1 ) { 
   die "USAGE: $0 [-v] [-r<rmin>]" .
       " [-q<power>] " .
       " [-f<factor>] " .
       " [-h] " . 
       " [-R<rmax>] <datafile1>\n";  }

$fname[0] = $ARGV[0];
$fname[1] = $ARGV[1];

if( $verbose > 1){
    print "*** verbose=", $verbose, "\n";
    print "*** factor=", $factor, "\n";
    print "*** q=", $q, "\n";
    print "*** h=", $hist, "\n";
    print "*** rmin=", $rmin, " rmax=", $rmax, "\n";
    print "*** datafname1=|", $fname[0], "|\n";
}

# die "bye";

# $inp = $ARGV[0];
# `mkboxcount.pl -r$rmin -R$rmax -q$q $inp > ${inp}.points`;
# if( $verbose > 0 ) {
   # `xgraph -lnx -lny ${inp}.points`;
# }
# print `cat ${inp}.points | loglog.pl | choptop | chopbottom | calcslope `;
# 


use POSIX;
# we need it for the 'floor' function


# scan the input file,
# compute the 'floor' of each entry
# update the appropriate count in a hash table

$globallen = 0;
$inp = $fname[0]; # try it first!
open (INP, $inp) or die "can not open $inp\n";

for( $r = $rmin; $r <= $rmax; $r *= $factor ){

   %hcount=(); # clear the hash table with the counts per cell
   $lineno = 0;
   seek INP, 0, 0; # make sure we are at the begining

   while(<INP>){
       $lineno ++;

       # s/\s//g; # kill blanks
       s/^\s+//; # kill leading white space
       s/\s+$//; # kill trailing  white space
       if( $verbose > 2) { print "INPUT: ", $_, "\n"; }

       @words = split;
       $len = scalar (@words);

       # check for bad input, with improper dimensionality
       if( $lineno == 1){ $globallen = $len; }
       else {
	        if ($len != $globallen ) {
	            die "line ", $lineno, " has ", 
		        $len, "entries instead of ", 
		        $globallen, "\n";
	        }
       }

#############
       if( 1 == $hist ){            # histogram: x1, x2, ... xn, value
            $val = pop(@words);     # gets the last value
            if( scalar(@words) < 1 ){
                die "ERROR: -h flag, with no coordinates - only values - exiting";
            }
       } else {                     # just points: x1 x2 ... xn
            $val = 1;               # just one point, by default
       }

       # like the earlier processing, with $val instead of just '1'
       @floorwords = map { floor($_ /$r )  } @words;
       $out = join ':', @floorwords;
       if($verbose > 4 ) { print $out, "(value= ", $val, " )\n"; }
       $hcount{$out} += $val;

#############

   }
   # collect the statistics:
   $total =0;
   foreach $key ( keys %hcount ) {
       $val = $hcount{$key};
       if(     $q == 0 ) { $total ++; }
       elsif ( $q == 1 ) { $total += $val * log( $val ); }
       else              { $total +=  ($val ** $q) ; }
   }
   print $r, " ", $total, "\n";


   # close(INP); # maybe 'rewind' would be better
   seek INP, 0, 0; # rewind


} # end for $r


