#include "KTVColRowManager.h"
#include "KTVFileManager.h"
#include <string.h>

void KTVColRowManager::MoveToRow(int row)
{
	if (!_sc||!_fm) return;
#ifdef KTVTESTMODE
	printf("Move to Row %d\n",row);
#endif
	if (row<BaseRow()) row=BaseRow();

	_sc->ClearRows();
	int minrow= -1;
	int maxrow= -1;

	// Find row range needed
	for (int i=row;;++i)
	{
		if (_fm->EOFKnown()&&i>_fm->GetEndRow()) 
		{
			if (_fm->IsExpandable()) SeekEnd();;	// In case it has changed
			if (i>_fm->GetEndRow())
			{
				MoveToLastRow(); 
				return;
			} 
		}
		if (_sc->IsFrozenRow(i)) continue;
		if (!_sc->AddRowOnBottom(i)) break;
		if (minrow<0) minrow= i;
		maxrow= i;
	}
	if (minrow<0||_sc->FreeHeight()>0) { MoveToLastRow(); return; }
	if (_scanandaddcols(minrow,maxrow)) MoveToCol(_col1);	// Just in case screen has to be readjusted
	if (_fm->GetLastRowRead()<maxrow)
	{
		if (_fm->EOFKnown()) { MoveToLastRow(); return; }	
		else assert(0);		// Should never happen
	}
	_row1= minrow;
	_row2= maxrow;

	_rf= 1;
#ifdef KTVTESTMODE
	printf("Move concludes with Rows: [%d - %d] and Cols:[%d - %d]\n",_row1,_row2,_col1,_col2);
#endif
}

void KTVColRowManager::MoveToLastRow(void)
{
	if (!_sc||!_fm) return;
#ifdef KTVTESTMODE
	printf("CR Move to Last Row\n");
#endif
	_sc->ClearRows();
	if (_fm->IsExpandable()||!_fm->EOFKnown()) SeekEnd();
	if (!_fm->EOFKnown()) return;		// Error, so return to prevent infinite loop
	int minrow= -1;
	int maxrow= -1;
	for (int i=_fm->GetEndRow();i>=0;--i)
	{
		if (_sc->IsFrozenRow(i)) continue;
		if (!_sc->AddRowOnTop(i)) break;
		if (maxrow<0) maxrow= i;
		minrow= i;
	}
	if (maxrow<0||minrow<0) return;	// No rows!
	if (_scanandaddcols(minrow,maxrow)) MoveToCol(_col1);	// Just in case screen has to be readjusted
	if (_fm->GetLastRowRead()<maxrow) assert(0);
	_row1= minrow;
	_row2= maxrow;

	_rf= 1;
#ifdef KTVTESTMODE
	printf("Move concludes with Rows: [%d - %d] and Cols:[%d - %d]\n",_row1,_row2,_col1,_col2);
#endif
}

void KTVColRowManager::SeekEnd(void)
{
	vector<int> newwids;
	_fm->SeekEnd(newwids,_cols.size());
	for (vector<int>::iterator ii=newwids.begin();ii!=newwids.end();++ii)
	{
		int nwid= _defwidth;
		if (_aligntofirst&&(*ii)>0) nwid= (*ii);
		AddCol(nwid,_defrjust);
	}
}

// Returns 1 if any new cols have been added
bool KTVColRowManager::_scanandaddcols(int minrow,int maxrow) 
{
	vector<int> newwids= _fm->Extract(minrow,maxrow,_cols.size());
	// Note that when past the first read row, additional cols are at default unless aligntofirst
	for (vector<int>::iterator ii=newwids.begin();ii!=newwids.end();++ii)
	{
		int nwid= _defwidth;
		if (_aligntofirst&&(*ii)>0) nwid= (*ii);
		AddCol(nwid,_defrjust);
	}
	return (!newwids.empty());
}

void KTVColRowManager::MoveToCol(int col)
{
	if (!_sc) return;
#ifdef KTVTESTMODE
	printf("CR Move to Col %d\n",col);
#endif
	if (col<0) col= 0;
	if (col>=_cols.size()) { MoveToLastCol(); return; }
	_sc->ClearCols();
	
	// First, scan forward from col and try to fill screen
	for (int i=col;i<_cols.size();++i)
	{
		if (_cols[i]._frozen||_cols[i]._hidden) continue;
		int w= _sc->AddColOnRight(i,_cols[i]._width);
		if (w<_cols[i]._width) break;
	}
	if (_sc->FreeWidth()>(_sc->HasGrid()?1:0)) { MoveToLastCol(); return; }
	
	_col1= _sc->GetColList().empty()?-1:_sc->GetColList().front().first;
	_col2= _sc->GetColList().empty()?-1:_sc->GetColList().back().first;
	_rf= 1;
#ifdef KTVTESTMODE
	printf("Move concludes with Rows: [%d - %d] and Cols:[%d - %d]\n",_row1,_row2,_col1,_col2);
#endif
}

void KTVColRowManager::MoveToLastCol(void)
{
	if (!_sc) return;
#ifdef KTVTESTMODE
	printf("CR Move to Last Col\n");
#endif
	_sc->ClearCols();
	for (int i=_cols.size()-1;i>=0;--i)
	{
		if (_cols[i]._frozen||_cols[i]._hidden) continue;
		int w= _sc->AddColOnLeft(i,_cols[i]._width);
		if (w<_cols[i]._width) break;
	}
	_col1= _sc->GetColList().empty()?-1:_sc->GetColList().front().first;
	_col2= _sc->GetColList().empty()?-1:_sc->GetColList().back().first;
	_rf= 1;
#ifdef KTVTESTMODE
	printf("Move concludes with Rows: [%d - %d] and Cols:[%d - %d]\n",_row1,_row2,_col1,_col2);
#endif
}

void KTVColRowManager::Draw(void)
{
	if (!_sc||!_fm) return;
	int row=0;
	KTVIdxListList::const_iterator ii= _sc->GetFrozenRows().begin();
	while (ii!=_sc->GetRowList().end())
	{
		if (ii==_sc->GetFrozenRows().end()) 
		{
			ii= _sc->GetRowList().begin();
			continue;
		}
		KTVRow *r= _fm->GetRow(ii->first);
		if (r)
		{
			int col=0;
			KTVIdxListList::const_iterator jj= _sc->GetFrozenCols().begin();
			while (jj!=_sc->GetColList().end())
			{
				if (jj==_sc->GetFrozenCols().end()) 
				{
					jj= _sc->GetColList().begin();
					continue;
				}
				bool rjustify= _cols[jj->first]._rightjust;
				int cwid= jj->second;
				if (jj->first<r->size()) WriteCell(row,col,(*r)[jj->first].c_str(),cwid,rjustify,_sc->GetFiller());
				else
				{
					char buf[cwid+1];
					memset(buf,_sc->GetFiller(),cwid);
					buf[cwid]= 0;
					WriteCell(row,col,buf,cwid,rjustify,_sc->GetFiller());
				}
				++jj;
				++col;
			}
			++row;
		}
		++ii;
	}
	_sc->GenerateScreenLayout();
	_sc->RefreshScreen();
	ClearFlag();
}

void KTVColRowManager::WriteCell(int row,int col,const char *x,int wid,bool rjustify,char filler)
{
	char buf[wid+1];
	buf[wid]= 0;
	if (!_sc) return;
	if (!x) memset(buf,filler,wid);
	else
	{
		int slen= strlen(x);
		if (rjustify)
		{
			if (slen>=wid) memcpy(buf,x+slen-wid,wid);
			else
			{
				if (slen>0) memcpy(buf+wid-slen,x,slen);
				memset(buf,filler,wid-slen);
			}
		}
		else
		{
			if (slen>=wid) memcpy(buf,x,wid);
			else
			{
				if (slen>0) memcpy(buf,x,slen);
				memset(buf+slen,filler,wid-slen);
			}
		}
	}
	_sc->WriteToCell(row,col,wid,buf);
}

int KTVColRowManager::NextActiveCol(int i) const
{
	if (i<-1) i=-1;
	int j=i+1;
	for (;j<_cols.size();++j)
		if (!_cols[j]._frozen&&!_cols[j]._hidden) return j;
	return j;	// OK that past the end!
}

int KTVColRowManager::NextActiveRow(int i) const
{
	if (i<-1) i=-1;
	int j= i+1;
	while (_sc->IsFrozenRow(j)) ++j;
	return j;
}

int KTVColRowManager::PrevActiveCol(int i) const
{
	int j= i-1;
	for (;j>=0;--j)
		if (!_cols[j]._frozen&&!_cols[j]._hidden) return j;
	return -1;
}

int KTVColRowManager::PrevActiveRow(int i) const
{
	if (i>_cols.size()) i= _cols.size(); 
	int j= i-1;
	for (;j>=0&&_sc->IsFrozenRow(j);--j);
	return j;		// Correctly returns -1 if none
}

void KTVColRowManager::FreezeCol(int n)
{
	if (n>=0&&n<_cols.size()&&!_cols[n]._frozen) 
	{
		_sc->FreezeCol(n,_cols[n]._width);
		_cols[n].Freeze(1);
		if (n!=_col1) MoveToCol(_col1);
		else MoveToCol(NextActiveCol(n));
	}
}

void KTVColRowManager::ToggleFreezeCol(int n)
{
	if (n<0||n>=_cols.size()) return;
	else if (_cols[n]._frozen) UnFreezeCol(n);
	else FreezeCol(n);
}

void KTVColRowManager::HideCol(int n) 
{ 
	if (n>=0&&n<_cols.size()) 
	{ 
		_cols[n].Hide(1); 
		if (n!=_col1) MoveToCol(_col1);
		else MoveToCol(NextActiveCol(n));
	}
}


void KTVColRowManager::UnHideCol(int n) 
{ 
	if (n>=0&&n<_cols.size()) 
	{  
		_cols[n].Hide(0); 
		MoveToCol(_col1);
	}
} 

void KTVColRowManager::ToggleHideCol(int n)
{
	if (n<0||n>=_cols.size()) return;
	else if (_cols[n]._hidden) UnHideCol(n);
	else HideCol(n);
}

void KTVColRowManager::FreezeRow(int n) 
{ 
	if (!_fm->FreezeRow(n)) return;
	if (n>=0) 
	{ 
		_sc->FreezeRow(n); 
		if (n!=_row1) MoveToRow(_row1);
		else MoveToRow(NextActiveRow(n));
	}
}

void KTVColRowManager::UnFreezeRow(int n) 
{ 
	_sc->UnFreezeRow(n); 
	MoveToRow(_row1);
}

void KTVColRowManager::ToggleFreezeRow(int n)
{
	if (_sc->IsFrozenRow(n)) UnFreezeRow(n);
	else FreezeRow(n);
}

void KTVColRowManager::UnFreezeCol(int n)
{
	if (n>=0&&n<_cols.size()&&_cols[n]._frozen) 
	{
		_sc->UnFreezeCol(n);
		_cols[n].Freeze(0);
		MoveToCol(_col1);
	}
}

int KTVColRowManager::GetMinColFittingFromRight(int n)
{
	if (!_sc) return -1;
	if (n<0) return -1;
	n= PrevActiveCol(n+1);
	if (n<0) return -1;
	int wid= _sc->ActiveRegionWidth()-_cols[n]._width;
	if (wid<=0) return n;	// Special case
	while (1)
	{
		int m= PrevActiveCol(n);
		if (m<0) return n;
		wid-=_cols[m]._width;
		if (_sc->HasGrid()) --wid;
		if (wid==0) return m;
		else if (wid<0) return n;
		else n=m;
	}
	return -1;	// Should never reach this!
}

void KTVColRowManager::AddReplaceMark(char c)
{
	map<char,pair<int,int> >::iterator ii=_marks.find(c);
	if (ii!=_marks.end()) ii->second = pair<int,int>(_row1,_col1);
	else _marks.insert(map<char,pair<int,int> >::value_type(c,pair<int,int>(_row1,_col1)));
}

void KTVColRowManager::RestoreMark(char c)
{
	map<char,pair<int,int> >::iterator ii=_marks.find(c);
	if (ii!=_marks.end()) 
	{
		MoveToRow(ii->second.first);
		MoveToCol(ii->second.second);
		DemandRedraw();
	}
}

// Fails if reach max
bool KTVColRowManager::AddCol(int w,bool rjust)
{
	if (_maxcols>0&&_cols.size()>=_maxcols) return 0;
	_cols.push_back(KTVColSpec(w,rjust));
#ifdef KTVTESTMODE
	printf("Added Column %ld with width %d and rjust=%d\n",_cols.size()-1,w,rjust);
#endif
	return 1;
}

void KTVColRowManager::Find(const KTVSearch &s,bool prev,bool allowcurr)
{
	if (!_fm||!_sc) return;
	string x= "";
	if (!prev&&s.IsByRow()) x= _findnextbyrow(s,allowcurr);
	else if (!prev) x= _findnextbycol(s,allowcurr);
	else if (s.IsByRow()) x= _findprevbyrow(s,allowcurr);
	else x= _findprevbycol(s,allowcurr);

	// Display cell on message line!!
	_sc->ClearInputRow();
	if (x!="") _sc->StatusMessage(x.c_str());
	DemandRedraw();
}

string KTVColRowManager::_findnextbyrow(const KTVSearch &s,bool allowcurr)
{
	int c= (!allowcurr)?_col1+1:_col1;
	int r= _row1;
	bool eflag= 0;
	while (1)
	{
		if (_fm->EOFKnown()&&r>_fm->GetEndRow())
		{
			if (_fm->IsExpandable()) SeekEnd();	// In case it changed
			if (r>_fm->GetEndRow()) return "No Match";
		}
		KTVRow *x=_fm->GetRow(r);
		if (!x)
		{
			if (eflag) { assert(0); return "Search Error. EOF should have been reached."; }
			_scanandaddcols(r,r);
			eflag= 1;
			continue;		// Restart loop since now acquired!
		}
		eflag= 0;
		for (;c<_cols.size()&&c<x->size();++c)
		{
			if (_cols[c]._hidden) continue;
			if (s.TestString((*x)[c]))
			{
				char buf[200];
				buf[199]= 0;
				snprintf(buf,200,"Match %d:%d %s",r,c,(*x)[c].c_str());
				Move(r,c);
				return buf;
			}
		}
		c= 0;
		++r;
	}
	return "Search Error.";
}

string KTVColRowManager::_findprevbyrow(const KTVSearch &s,bool allowcurr)
{
	int c= (!allowcurr)?_col1-1:_col1;
	int r= _row1;
	while (r>=0)
	{
		KTVRow *x=_fm->GetRow(r);
		if (!x)
		{
			_scanandaddcols(r,r);
			x= _fm->GetRow(r);
			if (!x) { assert(0); return "Search Error. Failed to read row."; }
		}
		for (;c>=0;--c)
		{
			if (c>=x->size()) continue;
			if (_cols[c]._hidden) continue;
			if (s.TestString((*x)[c]))
			{
				char buf[200];
				buf[199]= 0;
				snprintf(buf,200,"Match %d:%d %s",r,c,(*x)[c].c_str());
				Move(r,c);
				return buf;
			}
		}
		c= _cols.size()-1;
		--r;
	}
	return "No Match.";
}

/*

Unlike for findnext/prev by row, the use of allowcurr makes things ambiguous.  Whatever choice we make must allow for consistent repetition.  We choose to scan from our current point down and to the right.  The region to the left is omitted.  

For next search only, We must be careful to check frozen cols first or we will never be on top of the frozen cols so they never get searched.

*/
string KTVColRowManager::_findnextbycol(const KTVSearch &s,bool allowcurr)
{
	int c= _col1;
	int r= (!allowcurr)?_row1+1:_row1;
	SeekEnd();	// Just so we know that we have acquired it!
	if (!_fm->EOFKnown()) return "Search Error:  Unable to Acquire EOF";
	vector<int> foo;
	for (KTVIdxListList::const_iterator ii=_sc->GetFrozenCols().begin();ii!=_sc->GetFrozenCols().end();++ii) foo.push_back(ii->first);
	for (int i=c;i<_cols.size();++i)
		if (!_cols[i]._hidden&&!_cols[i]._frozen) foo.push_back(i);
	int msize= _cols.size();
	for (vector<int>::iterator ii=foo.begin();ii!=foo.end();++ii)
	{
		c= *ii;
		for (;r<=_fm->GetEndRow();++r)
		{
			KTVRow *x=_fm->GetRow(r);
			if (!x)
			{
				_scanandaddcols(r,r);
				if (_cols.size()>msize)	// Accommodate any new cols
				{
					for (int i=msize;i<_cols.size();++i) foo.push_back(i);
					msize= _cols.size();
				}
				x= _fm->GetRow(r);
				if (!x)
				{
					assert(0); 
					return "Search Error. Failed to Scan Row.";
				}
			}
			if (c>=x->size()) continue;	// Protect against this
			if (s.TestString((*x)[c]))
			{
				char buf[200];
				buf[199]= 0;
				snprintf(buf,200,"Match %d:%d %s",r,c,(*x)[c].c_str());
				Move(r,c);
				return buf;
			}
		}
		r= 0;
	}
	return "No Match.";
}

string KTVColRowManager::_findprevbycol(const KTVSearch &s,bool allowcurr)
{
	int c= _col1;
	int r= (!allowcurr)?_row1-1:_row1;
	SeekEnd();;	// Just so we know that we have acquired it!
	if (!_fm->EOFKnown()) return "Search Error:  Unable to Acquire EOF";
	while (c>=0)	// This may change as we scan rows, so we keep it dynamic
	{
		if (_cols[c]._hidden) 
		{
			--c;
			continue;
		}
		for (;r>=0;--r)
		{
			KTVRow *x=_fm->GetRow(r);
			if (!x)
			{
				_scanandaddcols(r,r);
				x= _fm->GetRow(r);
				if (!x)
				{
					assert(0); 
					return "Search Error. Failed to Scan Row.";
				}
			}
			if (c>=x->size()) continue;	// Protect against this
			if (s.TestString((*x)[c]))
			{
				char buf[200];
				buf[199]= 0;
				snprintf(buf,200,"Match %d:%d %s",r,c,(*x)[c].c_str());
				Move(r,c);
				return buf;
			}
		}
		r= _fm->GetEndRow();
		--c;
	}
	return "No Match.";
}

