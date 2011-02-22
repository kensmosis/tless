#ifndef KTVSCREENDEFFLAG
#define KTVSCREENDEFFLAG

#include <vector>
#include <string>
#include <assert.h>
#ifndef KTVTESTMODE
#include <curses.h>
#endif
#include "KTVIdxList.h"
using namespace std;


#ifndef KTVTESTMODE
#define CMVADDNSTR	mvaddnstr
#define CMVADDCH	mvaddch
#define CREFRESH	refresh
#define CGETCH		getch
#define CGETNSTR	getnstr
#else
#define CMVADDNSTR	ktestmvaddnstr
#define CMVADDCH	ktestmvaddch
#define CREFRESH	ktestrefresh
#define CGETCH		ktestgetch
#define CGETNSTR	ktestgetnstr
#endif

/* 

In the following, col,row refer to the col and row #'s in the Table
			   scol,srow refer to the col and row #'s on the screen


*/

class KTVScreen
{
protected:
	bool _grid;
	bool _seps;
	int _width;
	int _height;
	int _rowindexwid;		// Is there an index of rows and how wide is it?
	bool _colindexon;	// Is there an index of cols;
	char _filler;
	char _vsep;
	char _fsep;

	KTVIdxList _fc;	// Frozen cols
	KTVIdxList _fr;	// Frozen rows
	KTVIdxList _ac;	// Active cols
	KTVIdxList _ar;	// Active rows

public:
	KTVScreen(void) : _grid(0), _seps(0), _width(0), _height(0), _rowindexwid(DefaultRowIndexWidth()), _colindexon(1), _filler(' '),  _vsep('|'), _fsep('#'), _fc(), _fr(), _ac(), _ar()  {}
	~KTVScreen(void) { TerminateCurses(); }
	void RefitTerminal(void);
	void TerminateCurses(void);
	void InitCurses(void);

	// Parms
	void ToggleGrid(void) { _grid= 1-_grid; }	// Uses | and -
	void SetGrid(bool on) { _grid= on; }
	void ToggleSeps(void) { _seps= 1-_seps; }
	void SetSeps(bool on) { _seps= on; }
	void ToggleColIndex(void) { _colindexon= 1- _colindexon; }
	void SetColIndex(bool on) { _colindexon= on; }
	void ToggleRowIndex(void) { _rowindexwid= ((_rowindexwid>0)?0:DefaultRowIndexWidth()); }
	void SetRowIndex(bool on) { _rowindexwid= (on?DefaultRowIndexWidth():0); }
	void SetFiller(char x) { _filler= x; }
	void SetGridSep(char x) { _vsep= x; }
	void SetFrozenSep(char x) { _fsep= x; }
	void NoIndices(void) { _rowindexwid= 0; _colindexon= 0; }
	char GetFiller(void) const { return _filler; }

	// Column List Management (solely for indices)
	void ClearCols(void) { _ac.Clear(); }	// Clears active columns	
	void ClearRows(void) { _ar.Clear(); }
	int AddColOnRight(int col,int wid);
	int AddColOnLeft(int col,int wid);
	int AddRowOnBottom(int row) { return ((FreeHeight()-CostOfNextActiveRow()<=0)?0:_ar.AddBack(row,1)); }
	int AddRowOnTop(int row) { return ((FreeHeight()-CostOfNextActiveRow()<=0)?0:_ar.AddFront(row,1)); }

	// 	Frozen Column/Row Management
	int UnFreezeCol(int col) { return _fc.Remove(col); }	// Returns space freed
	int FreezeCol(int col,int wid); // Returns width (in case too big for screen) added (0 if already exists)
	void UnFreezeRow(int row) { _fr.Remove(row); }
	void UnFreezeAllRows(void) { _fr.Clear(); }
	bool FreezeRow(int row);
	int GetNumFrozenCol(void) const { return _fc.GetNum(); }
	int GetNumFrozenRow(void) const { return _fr.GetNum(); }
	bool IsFrozenRow(int n) const { return _fr.Exists(n); }
	bool IsFrozenCol(int n) const { return _fc.Exists(n); }
	const KTVIdxListList &GetFrozenRows(void) const { return _fr.GetList(); }	
	const KTVIdxListList &GetFrozenCols(void) const { return _fc.GetList(); }	
	const KTVIdxListList &GetRowList(void) const { return _ar.GetList(); }	
	const KTVIdxListList &GetColList(void) const { return _ac.GetList(); }	

// 	Adjust index width or height
	int GetRowIndexWidth(void) const { return _rowindexwid; }
	int ResizeRowIndex(int wid);	// Returns new width
	static int DefaultRowIndexWidth(void) { return 6; }

	//	Output to Cells
	void GenerateScreenLayout(void);
	void	DrawRowNumbers(void);
	void ClearInputRow(void);
	void DrawFrozenSeps(void);
	void DrawGrid(void);
	void DrawColNumbers(void);
	void FillUnused(void);
	bool WriteToCell(int srow,int scol,int wid,const char *p);	// row, col numbered to include frozen and are "screen" coords, not DB row,col #'s

//	Explicit Terminal Management
	static char GetNextKey(void)  { return CGETCH(); }
	bool WriteToScreen(int x,int y,int wid,const char *p,bool highlight=0);
	static void RefreshScreen(void) { CREFRESH(); }
	string GetCommand(char c);
	void StatusMessage(const char *x);
	bool HasGrid(void) const { return _grid; }
	static int ktestmvaddnstr(int y,int x,const char *str,int n);
	static int ktestmvaddch(int y,int x,char ch);
	static int ktestrefresh(void);
	static int ktestgetch(void);
	int ktestgetnstr(char *buf,int n);

/*	Information for sizing
	TrueRowIndexWid+FrozenWidth+ActiveRegionWidth= ScreenWidth
	1+TrueColIndexHgt+FrozenHeight+ActiveRegionHeight= ScreenHeight
	
	Note:  for our purposes, the active region begins with the first char of the first active col.  Any seperator between frozen and active (whether due to _grid or _seps) is part of the frozen region and is included.  If _grid and !_seps then this may not be visible if no active cols are present.  If no frozen cols are present then there is no width to the frozen region.


*/
	int ScreenWidth(void) const { return _width; }
	int ScreenHeight(void) const { return _height; }
	int TrueRowIndexWid(void) const { return ((_rowindexwid>0)?_rowindexwid+1:0); }
	int TrueColIndexHgt(void) const { return (_colindexon?1:0); }
	int FrozenWidth(void) const 	// Size of frozen region including any seps/grid
	{ 
		if (_fc.IsEmpty()) return 0;
		int w= _fc.GetSize();
		if (_grid) w+= _fc.GetNum() - 1;	// Grid lines between frozen cols
		if (_grid||_seps) ++w;			// Grid line or separator after frozen cols
		return w;
	}
	int FrozenHeight(void) const // Size of frozen region including any sep
	{
		return _fr.GetNum() + ((_seps&&!_fr.IsEmpty())?1:0);
	}
	int FreeWidth(void) const  // Size of unused portion of active region
	{
		if (_ac.IsEmpty()) return ActiveRegionWidth();
		int w= ActiveRegionWidth() - _ac.GetSize();
		if (_grid) w-= (_ac.GetNum()-1);		// Size of active region grid separators
		return w;
	}
	int FreeHeight(void) const // Size of unused portion of active region
	{
		return ActiveRegionHeight() - _ar.GetNum();
	}
	int ActiveRegionWidth(void) const	// Total size of active region (indep of usage)
	{ 
		return ScreenWidth() - FrozenWidth() - TrueRowIndexWid(); 
	} 
	int ActiveRegionHeight(void) const  // Total size of active region (indep of usage)
	{ 
		return ScreenHeight() - 1 - FrozenHeight() - TrueColIndexHgt(); 
	}
	int CostOfNextFrozenCol(void) const	// Aside from width, how much additional stuff (1 or 0) really
	{
		if (_grid) return 1;		// Add another separator between frozen cols or frozen/active
		else return ((_fc.IsEmpty()&&_seps)?1:0);	// Add separator between frozen/active
	}
	int CostOfNextFrozenRow(void) const	// Aside from rows, how much additional stuff (1 or 0) really
	{
		return (_seps&&_fr.IsEmpty())?1:0;
	}
	int CostOfNextActiveCol(void) const
	{
		return (!_ac.IsEmpty()&&_grid)?1:0;
	}
	int CostOfNextActiveRow(void) const
	{
		return 0;
	}
	int CostOfColumns(int nfrozen,int nactive) const  // Total non-column space used.
	{
		if (nfrozen<0||nactive<0) return 0;
		else if (nfrozen==0&&nactive==0) return 0;
		else if (_grid) return nfrozen+nactive-1;
		else if (_seps&&nfrozen>0) return 1;
		else return 0;
	}
	int CostOfRows(int nfrozen,int nactive) const  // Total non-row space used.
	{
		return (nfrozen>0)?1:0;
	}
};

#endif
