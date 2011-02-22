#  GNU Makefile tailored for tless.  We avoid autoconf which has a lot of unnecessary overhead and 
#  dependencies.
#
#  Note:  For a regular install, the provided ./gen directory contents should be suitable. 
#  	In that case, just run
#		make code
#		make install 
#
#  If you are a developer and have modified tless.pod then you may need to run 
#		make all   or    make gens
#	However, this requires the presence of several programs:
#		pod2*   and   gs
#  Various make bundle options also are provided.
#  If the UNAME autodetection isn't working for you, just hand-tailor to your OS.
#

UNAME := $(shell uname)

SYSBINDIR := /usr/local/bin
SYSMANDIR := /usr/local/share/man/man1

#  Put non-standard cases here!
ifeq ($(UNAME), FreeBSD))
	SYSMANDIR := /usr/local/man/man1
endif
#

BDIR := ./bin
ODIR := ./obj
GDIR := ./gen

CC := g++
CFLAGS := -g
#CFLAGS := $(CFLAGS) -DKTVTESTMODE
LFLAGS := -lcurses

OBJECTS := KTVTokenizer.o KTVFileManager.o KTVScreen.o KTVColRowManager.o KTVMain.o autogen_KTVUsage.o autogen_KTVKeyHelp.o main.o
HFILES := KTVTokenizer.h KTVFileManager.h KTVScreen.h KTVColRowManager.h KTVMain.h KTVIdxList.h

DOBJECTS := $(OBJECTS:%.o=$(ODIR)/%.o)

code: clean tless

tless: $(DOBJECTS) | $(BDIR)
	$(CC) $(DOBJECTS) $(CFLAGS) $(LFLAGS) -o $(BDIR)/tless

$(BDIR)/tless: tless

$(ODIR)/autogen_KTVUsage.o: $(GDIR)/autogen_KTVUsage.cpp $(HFILES) | $(ODIR)
	$(CC) $(CFLAGS) -I. -c $(GDIR)/autogen_KTVUsage.cpp -o $(ODIR)/autogen_KTVUsage.o

$(ODIR)/autogen_KTVKeyHelp.o: $(GDIR)/autogen_KTVKeyHelp.cpp $(HFILES) | $(ODIR)
	$(CC) $(CFLAGS) -I. -c $(GDIR)/autogen_KTVKeyHelp.cpp -o $(ODIR)/autogen_KTVKeyHelp.o

$(ODIR)/%.o: %.cpp $(HFILES) | $(ODIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	-rm -f $(ODIR)/*.o 
	-rm -f $(BDIR)/tless

cleangen:
	-rm -f $(GDIR)/tless.txt $(GDIR)/tless.1 $(GDIR)/tless.ps $(GDIR)/tless.html $(GDIR)/tless.pdf
	-rm -f $(GDIR)/autogen_KTVUsage.cpp
	-rm -f $(GDIR)/autogen_KTVKeyHelp.cpp

$(GDIR)/autogen_KTVUsage.cpp: tless.pod | $(GDIR)
	pod2text tless.pod > $(GDIR)/tless.txt
	cat $(GDIR)/tless.txt | perl ./ktvpod2help.pl > $(GDIR)/autogen_KTVUsage.cpp

$(GDIR)/autogen_KTVKeyHelp.cpp: tless_keys.txt | $(GDIR)
	cat tless_keys.txt | perl ./ktvpod2keyhelp.pl > $(GDIR)/autogen_KTVKeyHelp.cpp

man: tless.pod | $(GDIR)
	pod2man tless.pod > $(GDIR)/tless.1
	groff -man -Tps $(GDIR)/tless.1 > $(GDIR)/tless.ps
	-gs -dNOPAUSE -dBATCH -sDEVICE=pdfwrite -sOutputFile=$(GDIR)/tless.pdf $(GDIR)/tless.ps
	pod2html tless.pod > $(GDIR)/tless.html
	-rm -f pod2htmi.tmp
	-rm -f pod2htmd.tmp

$(GDIR)/:
	-mkdir $(GDIR)

$(BDIR)/: 
	-mkdir $(BDIR)

$(ODIR)/: 
	-mkdir $(ODIR)

gens: cleangen $(GDIR)/autogen_KTVUsage.cpp $(GDIR)/autogen_KTVKeyHelp.cpp man

all: clean gens tless

binbundle:
	tar -c -f tless_bin_dist_intel_$(UNAME).tar README.txt RELEASE.txt ./bin/tless ./gen/tless.1 Makefile TestGenTable.pl
	gzip -9 tless_bin_dist_intel_$(UNAME).tar

srcbundle:
	tar -c -f tless_src_dist.tar KTVTokenizer.cpp KTVTokenizer.h KTVFileManager.cpp KTVFileManager.h KTVScreen.cpp KTVScreen.h KTVColRowManager.cpp KTVColRowManager.h KTVIdxList.h KTVMain.cpp KTVMain.h main.cpp ktvpod2keyhelp.pl ktvpod2help.pl Makefile tless_keys.txt tless.pod README.txt RELEASE.txt TestGenTable.pl ./gen/*
	gzip -9 tless_src_dist.tar

install: $(BDIR)/tless $(GDIR)/tless.1
	cp $(BDIR)/tless $(SYSBINDIR)/tless
	cp $(GDIR)/tless.1 $(SYSMANDIR)/tless.1
	chmod 755 $(SYSBINDIR)/tless
	chmod 644 $(SYSMANDIR)/tless.1

uninstall:
	-rm -f $(SYSBINDIR)/tless
	-rm -f $(SYSMANDIR)/tless.1

