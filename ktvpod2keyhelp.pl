#
#  Extract usage c program from plain text file
#  cat tless_keys.txt | perl ./ktvpod2keyhelp.pl
#

use strict;

main(@ARGV);

sub main
{
	my $fn= shift;
	printf STDOUT "#include <stdio.h>\n#include \"KTVMain.h\" \nvoid KTVMain::KeyHelp(void)\n{\n";
	printf STDOUT "	KPRINT(\"*******************************************\\n\");\n";
	while (my $i=<STDIN>)
	{
		chomp $i;
		$i=~s/\\/\\\\/g;
		$i=~s/\"/\\\"/g;
		$i=~s/\%/\\\%/g;
		$i=~s/\'/\\\'/g;
		printf STDOUT "	KPRINT(\"*$i*\\n\");\n";
	}
	printf STDOUT "	KPRINT(\"************** Press q to return **********\\n\");\n";
	printf STDOUT "	KPRINT(\"*******************************************\\n\");\n";
	printf STDOUT "	while(CGETCH()!='q');\n";
	printf STDOUT "}\n\n\n";
	
}

