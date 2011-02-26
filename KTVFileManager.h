#ifndef KTVFILEMANAGERDEFFLAG
#define KTVFILEMANAGERDEFFLAG

#include <stdio.h>
#include <vector>
#include <string>
#include <map>
#include <list>
#include "KTVTokenizer.h"
using namespace std;

// Contains a full row of cell data
class KTVRow : public vector<string>
{
public:
	KTVRow(const vector<string> &x) : vector<string>(x) {}
	const char *GetVal(int n) const
	{
		if (n<0||n>=size()) return NULL;
		return (*this)[n].c_str();
	}
	static KTVRow *FromTokenizer(const KTVTokenizer &x) { return new KTVRow((vector<string>)(x)); }
	static KTVRow *Copy(const KTVRow &x) { return new KTVRow(x); }
};

class KTVCache
{
protected:
	// Cache management
	int _numfromend;			// Do not delete if this far from ends
	map<int,KTVRow *> _fr;			// Frozen and ends
	map<int,KTVRow *> _r;			// Cache for removal/addition
	list<int> _l;			// Cache list for removal/addition
	int _maxsize;			// maxsize of nonfrozen	If <=0, no max size
	int _size;			// Size of nonfrozen
	int _knownmax;			// EOF max
public:
	KTVCache(void) : _numfromend(0), _maxsize(0), _size(0), _knownmax(-1) {}
	~KTVCache(void) { ClearCache(); }
	void SetKnownMax(int n);
	bool Freeze(int row);		// There is no unfreeze command. 0 if row doesn't exist in cache
	bool AddRowFromTokenizer(int row,const KTVTokenizer &x);	// 0 if exists.  Never should happen
	void Init(int maxsize,int numfromendtolock);
	bool IsEmpty(void) const { return (_r.empty()&&_fr.empty()); }
	KTVRow *GetRow(int n);
	void ClearCache(void);
};

// Manages the actual file, read positions
class KTVFileManager
{
protected:
	FILE *_f;
	KTVTokenizer *_tok;
	int _maxrow;	// Only set once encountered
	vector<unsigned long> _seeklocs;	// Seek locations for start of every row registered so far
	int _cachesize;
	KTVCache _c;
	char _eol;
	bool _allowexpansion;

	void SetKnownMax(int n) { _maxrow= n; _c.SetKnownMax(n); }
	static int Kfgets(char *tgt,int maxlen,FILE *f,char eol);
public:
	// Canonical
	KTVFileManager(void) : _f(NULL), _tok(NULL), _maxrow(-1), _seeklocs(), _cachesize(DefaultCacheSize()), _eol(0), _allowexpansion(0) {}
	~KTVFileManager(void) {}

	// Initialization
	static int DefaultCacheSize(void) { return 10000; }
	bool AttachFile(FILE *f);
	void InitCache(int maxsize,int numfromend) { _c.Init(maxsize,numfromend); }
	void InitCache(void) { InitCache(_cachesize,_cachesize); }
	bool AttachTokenizer(KTVTokenizer *tok) { if (_tok||!tok) { return 0; } else { _tok= tok; return 1; } } 
	bool IsActive(void) const { return (_f&&_tok); }

	// Main information extraction
	vector<int> Extract(int minrow,int maxrow,int currnumcols);	// Seeks and parses the relevant rows.  Returns a list of column widths (as parsed) for any columns beyond the number specified (i.e. if 5 cols and give it 1 then it returns 4)
	bool FindAndReadRow(int i,vector<int> &foo,int numcols);
	void SeekEnd(vector<int> &foo,int numcols) { FindAndReadRow(-1,foo,numcols); }		// Scan to the end (may or may not load end cache).  Fills _maxrow
	KTVRow *GetRow(int n) { return _c.GetRow(n); }	// Get an already extracted row (can be frozen or regular)
	bool EOFKnown(void) const { return (_maxrow>=0); }
	int GetEndRow(void) const { return _maxrow; }
	int GetLastRowRead(void) const { return IsSeekable()?(_seeklocs.size()-2):_maxrow; }
	bool FreezeRow(int n) { return _c.Freeze(n); }
	void SetCacheSize(int n) { _cachesize= n; }
	bool IsSTDIN(void) const { return (_f==stdin); }
	void SetEOLChar(char x) { _eol= x; }
	void AllowExpansion(void) { _allowexpansion= 1; }
	bool IsExpandable(void) const { return _allowexpansion; }
	bool IsSeekable(void) const { return (_f&&_f!=stdin); }
	bool IsCached(void) const { return (_cachesize>0); }
	bool AddParseRow(int row,char *buf,vector<int> &foo,int numcols);
};



#endif
