#  Script to test tless.  For each 'type' below, a table is generated in /tmp and used for a tless session.
#	Hit 'q' to end.
#
#  The resulting tless session should look and act the same for all choices ('fixed' may have different col widths, though).  Otherwise there is a bug.
#
#  Arg:  type numrows numcols tlesscmd
#		tab			test tab-delimited using --sdelim
#		spaces		test space-delimited using --delim
#		csv			test csv
#		fixed		test fixed widths (randomly generated)
#		multi		test --delim with multiple delimiters (space | and : and tab) (randomly generated)
#
#
# ex.   perl ./TestGenTable.pl tab 

use strict;

main(@ARGV);

sub main
{
	my $type= shift;
	my $nrows= shift;
	my $ncols= shift;
	my $tlesscmd= shift||"./bin/tless";
	my @wids= ();
	for (my $j=0;$j<$ncols;++$j)
	{
		push @wids,6+int(rand(8));
	}
	my $tmpfil= "/tmp/shaldkfjh.".$$."$^T".".foo";

	my $cmd= "";
	if ($type eq "tab") { $cmd= "$tlesscmd -s \'\' -t -f \"$tmpfil\""; }
	elsif ($type eq "spaces") { $cmd= "$tlesscmd -d \" \" -f \"$tmpfil\""; }
	elsif ($type eq "csv") { $cmd= "$tlesscmd --csv -f \"$tmpfil\""; }
	elsif ($type eq "fixed") { $cmd= "$tlesscmd -x ".join(":",@wids)." -f \"$tmpfil\""; }
	elsif ($type eq "multi") { $cmd= "$tlesscmd -d \' :|\' -t -f \"$tmpfil\""; }
	else { die "Invalid type [$type]"; }

	open (TFIL,">$tmpfil") or die "Could not open [$tmpfil] for output";
	for (my $i=0;$i<$nrows;++$i)
	{
		for (my $j=0;$j<$ncols;++$j)
		{
			if ($type eq "fixed")
			{
				my $x= "$i,$j";
				if (length($x)>$wids[$j]) { substr($x,0,length($x)-$wids[$j])= ""; } 
				elsif (length($x)<$wids[$j]) { $x= (" " x ($wids[$j] - length($x))).$x; }
				printf TFIL $x;
			}
			elsif ($type eq "csv")
			{
				printf TFIL "," if ($j>0);
				printf TFIL "\"$i,$j\"";
			}
			elsif ($type ne "fixed")
			{
				if ($j>0)
				{
					if ($type eq "tab") { printf TFIL "\t"; }
					elsif ($type eq "spaces") { printf TFIL " " x (int(rand(5))+1); }
					elsif ($type eq "multi")
					{
						for (my $k=1;$k<=int(rand(6))+1;++$k)
						{
							my $l= int(rand(4));
							if ($l==0) { printf TFIL " "; }
							elsif ($l==1) { printf TFIL "\t"; }
							elsif ($l==2) { printf TFIL "|"; }
							elsif ($l==3) { printf TFIL ":"; }
						}
					}
				}
				printf TFIL "$i,$j";
			}
		}
		printf TFIL "\n";
	}
	close TFIL;
	printf STDERR "[$cmd]\n";
	my $pid= fork();
	if (!$pid)
	{
		exec $cmd;
		die "Failed to execute $cmd";
	}
	waitpid $pid, 0;
	`rm -f \"$tmpfil\"`;
}
