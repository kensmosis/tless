#
#  Extract usage c program from plain text file
#  pod2text tless.pod > tless_usage.txt
#  cat tless_usage.txt | perl ./ktvpod2help.pl
#

use strict;

main(@ARGV);

sub main
{
	my $fn= shift;
	printf STDOUT "#include <stdio.h>\n#include \"KTVMain.h\" \nvoid KTVMain::Usage(void)\n{\n";
	while (my $i=<STDIN>)
	{
		chomp $i;
		$i=~s/\\/\\\\/g;
		$i=~s/\"/\\\"/g;
		$i=~s/\%/\\\%/g;
		$i=~s/\'/\\\'/g;
		printf STDOUT "	printf(\"$i\\n\");\n";
	}
	printf STDOUT "}\n\n\n";
	
}

