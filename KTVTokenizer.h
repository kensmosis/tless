#ifndef KTVTOKENIZERDEFFLAG
#define KTVTOKENIZERDEFFLAG

#include <vector>
#include <string>
#include <regex.h>
using namespace std;

#define KTVBIGBUFFERSIZE		1048576

class KTVTokenizer : public vector<string>
{
protected:
	void _parse(char *buf,const char *tok,int len);
	void _sparse(char *buf,const char *tok,int len);
	bool _add(const char *x);	// fails if reaches end or maxcol
	void _fixedparse(char *x,int len);
	void _csvparse(char *x,int len);
	void StripQuotes(void);
public:
	KTVTokenizer(void) : _delim(""), _strict(0), _ignoreleading(1), _maxcols(0), _csv(0), _fixedcols(),  _stripquotes(0)  {}
	string _delim;		
	bool _strict;
	bool _ignoreleading;		// Ignore any delims before first non-delim
	int _maxcols;
	bool _csv;
	vector<int> _fixedcols;
	bool _stripquotes;		// Strip quotes around a field if they appear as a pair.
	void Parse(char *str);
	void ParseNonDestructive(const char *str);
	bool MakeValidSpec(void);	// Are the parms consistent?
	static string DefaultDelimiters(void) { return string(" \t"); }
	void SetStripQuotes(void) { _stripquotes= 1; }
	static char *BB(void)	// A global instance for temporary uses all over
	{
		static char buf[KTVBIGBUFFERSIZE];
		return buf;
	}
};

class KTVSearch
{
protected:
	string _s;
	bool _byrow;
	regex_t *_reg;
public:
	KTVSearch(void) : _s(""), _byrow(0), _reg(NULL) {}
	~KTVSearch(void) { Clear(); }
	void Clear(void);
	string SetSearch(string x,bool byrow); // Returns error string if problem
	bool TestString(const string &x) const;
	bool IsByRow(void) const { return _byrow; }
};

#endif
