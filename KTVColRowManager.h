#ifndef KTVCOLROWMANAGERDEFFLAG
#define KTVCOLROWMANAGERDEFFLAG

#include <stdio.h>
#include <string>
#include <vector>
#include <assert.h>
#include "KTVScreen.h"
#include "KTVTokenizer.h"
using namespace std;

class KTVFileManager;

// Info on the virtual columns for the full row (but used for display too)
class KTVColSpec
{
public:
	bool _rightjust;	// Default= right
	int _width;		// Col width
	bool _frozen;		// Whether fixed or floating
	bool _hidden;

	KTVColSpec(int w,bool rj) : _rightjust(rj), _width(w), _frozen(0), _hidden(0) {}
	void Freeze(bool on) { _frozen= on; }
	void Hide(bool on) { _hidden= on; }
	void Justify(bool onright) { _rightjust= onright; }
	void SetWidth(int val) { if (val>0) _width= val; }
	void AdjustWidth(int chg) { if (_width+chg>0) _width+= chg; }
};

class KTVColRowManager
{
protected:
	vector<KTVColSpec> _cols;
	KTVScreen *_sc;
	KTVFileManager *_fm;
	bool _rf;	// Redraw flag
	int _col1;	// Left active col on screen
	int _col2;	// Right active col on screen
	int _row1;	// Top active row on screen
	int _row2;	// Bottom active row on screen
	map<char,pair<int,int> > _marks;

	// Some user parms
	int _defwidth;
	bool _defrjust;
	int _skip;		// 0 for no skip
	int _maxcols;		// 0 for no max
	bool _aligntofirst;	// Set column widths based on first column

	// local fns
	bool _scanandaddcols(int minrow,int maxrow);
	string _findnextbyrow(const KTVSearch &s,bool allowcurr);
	string _findprevbyrow(const KTVSearch &s,bool allowcurr);
	string _findnextbycol(const KTVSearch &s,bool allowcurr);
	string _findprevbycol(const KTVSearch &s,bool allowcurr);
public:
	// Canonical
	KTVColRowManager(void) : _sc(NULL), _fm(NULL), _cols(), _rf(0), _col1(-1), _col2(-1), _row1(-1), _row2(-1), _defwidth(InitialDefaultColWidth()), _defrjust(1), _skip(0), _maxcols(0), _aligntofirst(0), _marks() {}

	// Attach
	void Attach(KTVScreen *sc,KTVFileManager *fm) { _sc= sc; _fm= fm; }
	void ClearFlag(void) { _rf= 0; }
	void DemandRedraw(void) { _rf= 1; }

	// Marks
	void AddReplaceMark(char c);
	void RestoreMark(char c);

	// Hide/Unhide
	void ExposeAllCols(void) { for (int i=0;i<_cols.size();++i) _cols[i].Hide(0); MoveToCol(_col1); }
	void HideCol(int n);
	void UnHideCol(int n);
	void ToggleHideCol(int n);

	// Freeze/Unfreeze -- make sure only do on rows/cols which are already frozen (or unfrozen)
	void FreezeRow(int n);
	void UnFreezeRow(int n);
	void FreezeCol(int n);
	void UnFreezeCol(int n);
	void ToggleFreezeCol(int n);
	void ToggleFreezeRow(int n);
	void UnFreezeAllCols(void) { for (int i=0;i<_cols.size();++i) UnFreezeCol(i); }
	void UnFreezeAllRows(void) { _sc->UnFreezeAllRows(); _rf= 1; }
	void UnFreezeAll(void) { UnFreezeAllCols(); UnFreezeAllRows(); }

	// Right/Left Justify
	void RightJustifyCol(int n) { if (n>=0&&n<_cols.size()) { _cols[n].Justify(1); _rf= 1; } }
	void LeftJustifyCol(int n) { if (n>=0&&n<_cols.size()) { _cols[n].Justify(0); _rf= 1; } }
	void RightJustifyAllCols(void) { _rf= 1;  for (int i=0;i<_cols.size();++i) _cols[i].Justify(1); }
	void LeftJustifyAllCols(void) { _rf= 1;  for (int i=0;i<_cols.size();++i) _cols[i].Justify(0); }
	void SetDefaultRightJustify(void) { _defrjust= 1; }
	void SetDefaultLeftJustify(void) { _defrjust= 0; }

	// Col Width Set
	void SetColWidth(int n,int wid) { if (n>=0&&n<_cols.size()&&wid>0) { _cols[n].SetWidth(wid); _rf= 1; } }
	void SetAllColWidths(int wid) { if (wid>0) { _rf= 1;  for (int i=0;i<_cols.size();++i) _cols[i].SetWidth(wid); } }
	void SetAllColWidthsToDefault(void) { SetAllColWidths(_defwidth); }
	static int InitialDefaultColWidth(void) { return 12; }
	void SetDefaultColWidth(int n) { if (n>0) _defwidth= n; }
	int GetDefaultWidth(void) const { return _defwidth; }

	// Col Width Adjust (can be neg)
	void IncreaseColWidth(int n,int chg) { if (n>=0&&n<_cols.size()) { _cols[n].AdjustWidth(chg); _rf= 1; } }
	void IncreaseAllColWidths(int chg) { _rf= 1;  for (int i=0;i<_cols.size();++i) _cols[i].AdjustWidth(chg); }	
	void ReturnToDefaults(void)
	{
		ExposeAllCols();
		UnFreezeAll();
		RightJustifyAllCols();
		SetAllColWidthsToDefault();
	}

	// Other Useful
	void SetRowSkip(int n) { _skip= (n>0?n:0); }
	void SetMaxCols(int n) { _maxcols= (n>0?n:0); }
	void SetAlignToFirst(void) { _aligntofirst= 1; }
	bool AddCol(int w,bool rjust); 
	bool AddCol(int w) { return AddCol(w,_defrjust); }
	int BaseRow(void) const { return _skip; }

	////////////  Move ops
	void MoveToRow(int row);
	void MoveToLastRow(void);
	void MoveToCol(int col);
	void MoveToLastCol(void);
	void Move(int row,int col) { MoveToRow(row); MoveToCol(col); }
	void Draw(void);
	void WriteCell(int row,int col,const char *x,int wid,bool rjustify,char filler);
	void Restate(void) { Move(_row1,_col1); }

	// The following are non-inclusive and it doesn't matter if the starting row/col is active
	int NextActiveCol(int i) const;	// Return next non-frozen,non-hidden col after i. -1 if none.
	int NextActiveRow(int i) const;	// Return next active row after i.  Can return 1 past the end of _rows (because there may be more than we know about!
	int PrevActiveCol(int i) const;	// Return next non-frozen,non-hidden col before i. -1 if none.
	int PrevActiveRow(int i) const;	// Return next active row before i. -1 if none
	int GetLeftActiveCol(void) const { return _col1; }
	int GetRightActiveCol(void) const { return _col2; }
	int GetTopActiveRow(void) const { return _row1; }
	int GetBottomActiveRow(void) const { return _row2; }
	int GetNumCols(void) const { return _cols.size(); }
	void RedrawIfNeeded(void) { if (_rf) { Draw();  ClearFlag(); } }
	int GetMinColFittingFromRight(int n);	// If n is rightmost col, which is leftmost full col that would fit on screen?

	// Search
	void Find(const KTVSearch &x,bool prev,bool allowcurr);
	void TestCell(const string &s);

	void SeekEnd(void);
};

#endif

