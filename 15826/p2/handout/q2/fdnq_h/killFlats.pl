: # Use perl
   eval 'exec perl -S $0 "$@"'
   if 0;

# it is faster to use something like
#!/usr/local/bin/perl5 
# but this is more portable.

# expects a file with points (one per line)
# and drops the parts that are within log(2) from
# the top and the bottom of the plot
# It is supposed to replace 'choptop' and 'chopbottom'
# simultaneously

$verbose = 0;
$debug = 0;


while( $_ = $ARGV[0], /^-/){
   shift;
   last if /^--$/;
   if( /^-D(.*)/){ $debug = $1 }
   if( /^-k(.*)/){ $k = $1 }
   if (/^-v/)    { $verbose++ }
}

$nargs = $#ARGV + 1;   # the number of remaining arguments

if( $nargs > 1 ) { die "USAGE: $0 <file>"  }

# print "first filename is=", $ARGV[0], "\n";


$HUGE = 2^30;
$miny = $HUGE;
$maxy = -$HUGE;
$width = log(2); #drop all points with $y within $width from min or max

$lineno = 0;
while(<>){
    # chomp;
    # s/\s//g; # kill blanks
    # print $_, "\n";
    $lineno ++;
    ($x, $y) = split;
    if($verbose)  { print $x, " ", $y, "\n"; }
    $xa[$lineno] = $x;
    $ya[$lineno] = $y;

    if( $y < $miny) { $miny = $y;}
    if ( $y > $maxy) { $maxy = $y ; }
}

$totpts = $lineno;

if( $verbose ) { print "maxy=", $maxy, " miny=", $miny, "\n" ;}

for( $i=1; $i<= $totpts; $i++){
    $y = $ya[$i];
    if( ( $y > $miny + $width ) && ($y < $maxy - $width) ) {
      print $xa[$i], " ", $ya[$i], "\n";
    }
}
