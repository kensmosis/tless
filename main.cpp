// #define KTVTESTMODE 1		// Deactivate this normally
#include <signal.h>
#include "KTVMain.h"

/*

To do:
	
	To implement:
			proper error handling (report error in input field)
			change so if freeze row and it doesn't exist in cache it will be read in (to allow R:foo commands from the command line -X option)
			adjust key choices to be similar to vi and less
			modify Extract and all places in CR such that EOF is only temporarily known.  That way stdin (and even another file) may have stuff appended dynamically.
			perhaps add a key to reset the EOF flag?
			Modify Extract routine to deal with stdin (no seeks, etc).
			Write own fgets to allow arbirary EOL, and move EOL parm to FileManager from tokenizer.
*/

KTVMain m;

static void resizehandler(int x)
{
	m.ResizeIfReady();
}

int main(int argc,char **argv)
{
	if (signal(SIGWINCH,resizehandler)==SIG_ERR) fprintf(stderr,"Error setting up signal handler\n");
	m.Run(argc,argv);
}
