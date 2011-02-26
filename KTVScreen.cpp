#include "KTVScreen.h"
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

void KTVScreen::InitCurses(void)
{
#ifndef KTVTESTMODE
	if (!initscr()) exit(0);
	cbreak();
	noecho();
	nonl();
	leaveok(stdscr,TRUE);
	intrflush(stdscr,FALSE);
	keypad(stdscr,TRUE);
	getmaxyx(stdscr,_height,_width);
//	def_prog_mode();
#else
	_height= 25;
	_width= 80;
#endif

}

void KTVScreen::TerminateCurses(void)
{
#ifndef KTVTESTMODE
	endwin();
//	fflush(stdout);
#endif
}

void KTVScreen::RefitTerminal(void)
{
#ifndef KTVTESTMODE	
	endwin();
	refresh();
	getmaxyx(stdscr,_height,_width);
#endif	
}

int KTVScreen::AddColOnRight(int col,int wid)
{
	int awid= FreeWidth()-CostOfNextActiveCol();
	if (awid<=0) return 0;
	if (awid<wid) wid= awid;
	return _ac.AddBack(col,wid);
}

int KTVScreen::AddColOnLeft(int col,int wid)
{
	int awid= FreeWidth()-CostOfNextActiveCol();
	if (awid<=0) return 0;
	if (awid<wid) wid= awid;
	return _ac.AddFront(col,wid);
}

int KTVScreen::FreezeCol(int col,int wid)
{
	if (_fc.Exists(col)) return 1;		// Already frozen.  OK
	int awid= ActiveRegionWidth() - CostOfNextFrozenCol();
	if (wid>awid) wid= awid;
	return _fc.AddBack(col,wid);
}

bool KTVScreen::FreezeRow(int row)
{
	if (_fr.Exists(row)) return 1;	// Already frozen.  OK
	int ahgt= ActiveRegionHeight() - CostOfNextFrozenRow();
	if (ahgt<1) return 0;	// No space
	return (_fr.AddBack(row,1)>0);
}

int KTVScreen::ResizeRowIndex(int wid)
{
	int awd= ActiveRegionWidth() + TrueRowIndexWid();
	--awd;		// Remove the space separator
	if (wid>awd) wid= awd;
	_rowindexwid= wid;
	return wid;
}

bool KTVScreen::WriteToScreen(int x,int y,int wid,const char *p,bool highlight)
{
	if (!p||wid<=0) return 0;
	if (x<0||x>=ScreenWidth()) return 0;
	if (x+wid>ScreenWidth()) wid= ScreenWidth()-x;
	if (y<0||y>=ScreenHeight()) return 0;
#ifndef KTVTESTMODE
	if (highlight) attron(A_REVERSE);
#endif
	CMVADDNSTR(y,x,p,wid);
#ifndef KTVTESTMODE
	if (highlight) attroff(A_REVERSE);
#endif
	return 1;
}

bool KTVScreen::WriteToCell(int row,int col,int wid,const char *p)
{
	if (row<0||row>=_fr.GetNum()+_ar.GetNum()) return 0;
	if (col<0||col>=_fc.GetNum()+_ac.GetNum()) return 0;
	if (!p||wid<=0) return 0;
	int x= TrueRowIndexWid();
	int y= TrueColIndexHgt();
	if (row>=_fr.GetNum()) y+= FrozenHeight() + row - _fr.GetNum();
	else y+= row;
	int colwid= -1;
	if (col<_fc.GetNum())
	{
		if (_grid&&col>0) x+= col;
		KTVIdxListList::const_iterator ii;
		for (ii=_fc.GetList().begin();ii!=_fc.GetList().end()&&col>0;++ii,--col)
			x+= ii->second;
		if (ii==_fc.GetList().end()) return 0;	// Should never happen!
		colwid= ii->second;
	}
	else
	{
		x+= FrozenWidth();
		col-= _fc.GetNum();
		if (_grid&&col>0) x+= col;
		KTVIdxListList::const_iterator ii;
		for (ii=_ac.GetList().begin();ii!=_ac.GetList().end()&&col>0;++ii,--col)
			x+= ii->second;
		if (ii==_ac.GetList().end()) return 0;
		colwid= ii->second;		
	}
	assert(wid==colwid);
	CMVADDNSTR(y,x,p,wid);
	return 1;
}

void KTVScreen::GenerateScreenLayout(void)
{
	DrawRowNumbers();
//	ClearInputRow();
	DrawFrozenSeps();
	DrawGrid();
	DrawColNumbers();
	FillUnused();
}

void KTVScreen::FillUnused(void)
{
	int unusedy= FreeHeight();
	int unusedx= FreeWidth();
	assert(unusedx>=0);
	assert(unusedy>=0);	
	if (unusedy>0)
	{
		char buf[_width];
		memset(buf,_filler,_width);
		for (int i=1;i<=unusedy;++i)
			CMVADDNSTR(_height-1-i,0,buf,_width);
	}
	if (unusedx>0)
	{
		char buf[unusedx];
		memset(buf,_filler,unusedx);
		for (int i=0;i<_height-1-unusedy;++i)
			CMVADDNSTR(i,_width-unusedx-1,buf,unusedx);
	}
}

void KTVScreen::DrawGrid(void)
{
	if (!_grid) return;
	int x= TrueRowIndexWid();
	bool first= 1;
	for (KTVIdxListList::const_iterator ii=_fc.GetList().begin();ii!=_fc.GetList().end();++ii)
	{
		if (!first)
		{
			for (int i=0;i<_height-1;++i) CMVADDCH(i,x,_vsep);
			++x;
		}
		x+= ii->second;
		if (first) first= 0;
	}

	// Skip frozen section separator
	if (_seps&&!_fc.IsEmpty()) 
	{
		++x;
		first= 1;
	}	
	for (KTVIdxListList::const_iterator ii=_ac.GetList().begin();ii!=_ac.GetList().end();++ii)
	{
		if (!first)
		{
			for (int i=0;i<_height-1;++i) CMVADDCH(i,x,_vsep);
			++x;
		}
		x+= ii->second;
		if (first) first= 0;
	}
	// Put in a special last column if needed
	if (_grid&&FreeWidth()==1)
	{
		for (int i=0;i<_height-1;++i) CMVADDCH(i,_width-1,_vsep);
	}
}

void KTVScreen::DrawColNumbers(void)
{
	if (!_colindexon) return;
	int x= TrueRowIndexWid();
	bool first= 1;
	for (KTVIdxListList::const_iterator ii=_fc.GetList().begin();ii!=_fc.GetList().end();++ii)
	{
		if (_grid&&!first) ++x;	// Skip grid separator
		char buf[ii->second+1];
		snprintf(buf,ii->second+1,"%*d",ii->second,ii->first);
		buf[ii->second]= 0;
		CMVADDNSTR(0,x,buf,ii->second);
		x+= ii->second;
		if (first) first= 0;
	}

	// Skip frozen section separator
	if (_seps&&!_fc.IsEmpty()) 
	{
		++x;	
		first= 1;
	}

	for (KTVIdxListList::const_iterator ii=_ac.GetList().begin();ii!=_ac.GetList().end();++ii)
	{
		if (_grid&&!first) ++x;	// Skip grid separator
		char buf[ii->second+1];
//		fprintf(stderr,"Snprintf2 Before:  %d %d\n",ii->second,ii->first);
		snprintf(buf,ii->second+1,"%*d",ii->second,ii->first);
		buf[ii->second]= 0;
//		fprintf(stderr,"Snprintf2 After:  [%s]\n",buf);
//		fflush(stderr);
		CMVADDNSTR(0,x,buf,ii->second);
		x+= ii->second;
		if (first) first= 0;
	}
}

void KTVScreen::DrawRowNumbers(void)
{
	// First, row indices
	if (_rowindexwid<=0) return;
	int w= _rowindexwid+1;
	char buf[w+1];
	int i=0;

	// Initial empty cell if there is a column index in the first row
	if (_colindexon)
	{
		memset(buf,_filler,w);
		CMVADDNSTR(i,0,buf,w);
		++i;
	}

	// Frozen rows
	for (KTVIdxListList::const_iterator ii=_fr.GetList().begin();ii!=_fr.GetList().end();++ii)
	{
		snprintf(buf,w+1,"%*d ",w-1,ii->first);
		buf[w]= 0;
		CMVADDNSTR(i,0,buf,w);
		++i;
	}

	// Skip frozen row sep
	if (_seps&&!_fr.IsEmpty()) ++i;
	
	// Regular rows
	for (KTVIdxListList::const_iterator ii=_ar.GetList().begin();ii!=_ar.GetList().end();++ii)
	{
		snprintf(buf,w+1,"%*d ",w-1,ii->first);
		buf[w]= 0;
		CMVADDNSTR(i,0,buf,w);
		++i;
	}
}

void KTVScreen::ClearInputRow(void)
{
	char buf[_width];
	memset(buf,' ',_width);
	CMVADDNSTR(_height-1,0,buf,_width);
}

void KTVScreen::DrawFrozenSeps(void)
{
	if (!_seps) return;

	// The horizontal separator
	if (!_fr.IsEmpty())
	{
		char buf[_width];
		memset(buf,_fsep,_width);
		int y= TrueColIndexHgt() + FrozenHeight() - 1;
		CMVADDNSTR(y,0,buf,_width);
	}

	// The vertical separator
	if (!_fc.IsEmpty())
	{
		int x= TrueRowIndexWid() + FrozenWidth() - 1;
		for (int i=0;i<_height-1;++i) CMVADDCH(i,x,_fsep);
	}
}

string KTVScreen::GetCommand(char c)
{
	const char blnk[200]= "                                                                                                                                                                                                ";
	int nwd= _width-2;
	if (nwd>180) nwd= 180;
	CMVADDCH(_height-1,0,c);
	CMVADDNSTR(_height-1,1,blnk,nwd);
	CMVADDCH(_height-1,1,' ');		// To move to correct location
#ifndef KTVTESTMODE
	echo();
#endif
	char buf[128];
	buf[0]= 0;
	CGETNSTR(buf,127);
#ifndef KTVTESTMODE
	noecho();	
#endif
	return string(buf);
}

// Draw status message
void KTVScreen::StatusMessage(const char *x)
{
	if (!x) return;
	char buf[_width+1];
	memset(buf,' ',_width);
	buf[_width]= 0;
	strncpy(buf,x,_width);
	if (strlen(x)<_width) buf[strlen(x)]=' ';
	ClearInputRow();
	CMVADDNSTR(_height-1,0,x,_width);
	CREFRESH();
}

int KTVScreen::ktestmvaddnstr(int y,int x,const char *str,int n)
{
	char buf[n+1];
	buf[n]= 0;
	if (str) memcpy(buf,str,n);
	else buf[0]= 0;
	fprintf(stderr,"Curses mvaddnstr to row=%d col=%d str=[%s]\n",y,x,buf);
//	printw("Curses mvaddnstr to row=%d col=%d char=[%s]\n",y,x,buf);
	fflush(stderr);
	return 1;
}

int KTVScreen::ktestmvaddch(int y,int x,char ch)
{
	fprintf(stderr,"Curses mvaddch to row=%d col=%d char=[%c]\n",y,x,ch);
//	printw("Curses mvaddch to row=%d col=%d char=[%c]\n",y,x,ch);
	fflush(stderr);
	return 1;
}

int KTVScreen::ktestrefresh(void)
{
	fprintf(stderr,"Curses refresh command\n");
//	printw("Curses refresh command\n");
	fflush(stderr);
	return 1;
}

int KTVScreen::ktestgetch(void)
{
	char c= fgetc(stdin);
	fprintf(stderr,"Curses getch obtained [%c]\n",c);
	fflush(stderr);
	return c;
}

int KTVScreen::ktestgetnstr(char *buf,int n)
{
	if (!buf&&n<=0) return 0;
	char *p= fgets(buf,n,stdin);
	return p?1:0; 
}

