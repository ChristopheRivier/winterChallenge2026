use strict;
use warnings;


my $out = './';
my $source = 'src/';

open( my $prog, ">", 'perlCodeGen.cpp');

open( my $main, "<" , $source.'main.cpp');

  while ( ! eof($main) ) {
         my $line = readline $main;
	if( $line =~/#include \"([^ \"]*)\"/){
	print $prog "//--------------------- BEGIN $1\n";

	open( my $tmp, "<", $source.$1);

	while ( ! eof($tmp) ) {
         my $linetmp = readline $tmp;
		 if( $linetmp=~/#pragma|#include "/){}
		 else{
		 print $prog $linetmp;
		 }
	}
	close $tmp;
	print $prog "//--------------------- END $1\n";
	}else{
	print $prog $line;
	}

}
close $main;
close $prog;
