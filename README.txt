README for tless utility

DESCRIPTION

"tless" is a lightweight curses-based Viewer for text files of tabular data, allowing synchronized scrolling, custom parsing, and movement by columns as well as rows.  It was inspired by less.  

INSTALLATION

---If you downloaded the binary package for the right system, then just use the binary "tless" or run

sudo make install

To uninstall use:

sudo make uninstall


---If you are building from source, then use

make

or 

make code

The output will sit in ./bin

Additional documentation formats are present as ./gen/tless.*   (pdf, html, txt, etc)

---If you wish to modify the source and rebuild everything, use

make all

Note that this will require the presence of certain programs such as pod2* and gs to manage the documentation.  If these do not exist, the Makefile may need to be edited.  

SYSTEMS

tless was developed on Darwin.  It has been tested on several systems and binary packages are available.  Note that I tested it using simple VMs, so these should be rough rules of thumb.

1.  Darwin.  Works.
2.  ARCH Linux.  "make" and "make all" work.  Make install/uninstall work but may require the user to create a directory /usr/local/share/man/man1 if it doesn't exist.  Program executes fine.
3.  FreeBSD.  Use "gmake" instead of make.  "gmake" and "gmake all" work.  Ditto "gmake install" and "gmake uninstall".

CONTENTS

The following are needed for the full build:
Source code:  KTV*.h  KTV*.cpp  Makefile main.cpp
Scripts:      Makefile ktvpod2help.pl ktvpod2keyhelp.pl
Source docs:  tless.pod   tless_keys.txt  RELEASE.txt README.txt TUTORIAL.txt
Test script:  TestGenTable.pl

The following are generated and should not be modified for a simple source build:
gen/*  (includes autogen*.cpp and tless.* documentation files)

The following are constructed by a simple source build or are present in a binary package:

bin/tless   (or tless in binary package)	The executable.
bin/tless.1  (or tless.1 in binary package)  Man page.

LICENSE

tless is offered under a BSD open source license.  Basically, you can copy and use it for any purpose as long as you (1) credit me, (2) include the license, and (3) don't usurp the identity of "tless" or claim that I endorse your product.  The exact license is provided on the man page and in the tless.pod file.  If for some reason, this proves too restrictive for a particular use please contact me at the email below.  

BUGS and CONTRIBUTIONS

If you find a bug, have a suggestion, or wish to help with porting tless, please let me know at tless.code@gmail.com.

At present, I am the sole maintainer of "tless."  When tless has matured (and I have more experience with open-source versioning/collaboration), I will welcome other contributors. 

Please bear in mind that tless is my first open source unix release.  As a result, many of the non-coding aspects involve a learning curve.  This is the reason for a number of the caveats above.  Any help with these aspects  -- porting, packaging, documentation, etc -- would be welcome.  In particular, if you get it working on a particular flavor of unix then please tell me what was needed.

