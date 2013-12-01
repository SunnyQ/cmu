: # Use perl
   eval 'exec perl -S $0 "$@"'
   if 0;

# it is faster to use something like
#!/usr/local/bin/perl5 
# but this is more portable.
# expects numbers, and prints their logs

while(<>){
   @words = split;
      print (log($words[0]), " ", log($words[1]), "\n" ) 
}
