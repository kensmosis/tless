#include "KTVTokenizer.h"
#include <string.h>

bool KTVTokenizer::_add(const char *x)	// fails if reaches end or maxcol
{
	if (!x) return 0;
	else if (_maxcols>0&&size()>=_maxcols) return 0;
	push_back(x);
	return 1;
}

bool KTVTokenizer::MakeValidSpec(void)
{
	int n= 0;
	if (_delim!="") ++n;
	if (_csv) ++n;
	if (!_fixedcols.empty()) ++n;
	if (n>1) return 0;
	if (_strict&&_delim=="") return 0;
	if (n==0) _delim= KTVTokenizer::DefaultDelimiters();
	// Note: _ignoreleading is ignored for fixed and csv
	return 1;
}

// If strict then single delimiters only -- otherwise, aggregates adjacent delimiters
void KTVTokenizer::Parse(char *str)
{
	clear();
	if (!str) return;
	int len= strlen(str);
	if (len<1) return;
	if (_csv) _csvparse(str,len);
	else if (!_fixedcols.empty()) _fixedparse(str,len);
	else if (!_strict) _parse(str,_delim.c_str(),len);
	else _sparse(str,_delim.c_str(),len);
	if (_stripquotes) StripQuotes();
}

// If strict then single delimiters only -- otherwise, aggregates adjacent delimiters
void KTVTokenizer::ParseNonDestructive(const char *str)
{
	clear();
	if (!str) return;
	int len= strlen(str);
	if (len<1) return;
	char buf[len+1];
	strcpy(buf,str);
	if (_csv) _csvparse(buf,len);
	else if (!_fixedcols.empty()) _fixedparse(buf,len);
	else if (!_strict) _parse(buf,_delim.c_str(),len);
	else _sparse(buf,_delim.c_str(),len);
	if (_stripquotes) StripQuotes();
}

void KTVTokenizer::_parse(char *buf,const char *tok,int len)
{
	char *p= strtok(buf,tok);
	if (p&&_ignoreleading&&strlen(p)==0) p= strtok(NULL,buf);
	while (_add(p)) p= strtok(NULL,tok);
}

// We assume buf
void KTVTokenizer::_sparse(char *buf,const char *tok,int len)
{
	int lpos= 0;
	int i=0;
	while (i<len)
	{
		if (strchr(tok,buf[i]))	// delimiter
		{
			if (i==0&&_ignoreleading) { lpos++; i++; continue; }
			buf[i]= 0;
			if (!_add(buf+lpos)) break;
			lpos= i+1;
		}
		++i;
	}
	if (lpos<len) _add(buf+lpos); 	// buf[len]= 0 already from input!
}

// Note that x has array size >= len+1  and a terminating 0
void KTVTokenizer::_fixedparse(char *x,int len)
{
	if (!x||len<=0) return;
	int num= _fixedcols.size();
	if (_maxcols>0&&num>_maxcols) num= _maxcols;
	char p;
	int x1,x2;
	x1= 0;
	for (int i=0;i<num&&x1<len;++i)
	{
		x2= x1+_fixedcols[i];
		if (x2>len) x2= len;
		p= x[x2];
		x[x2]= 0;
		_add(x+x1);
		x[x2]= p;
		x1= x2;
	}
}

/*

Primitive CSV version.  Splits on any , outside of "".  Matches "" as pairs.  Doesn't deal with complex nested fields or escaped characters.  Should handle most excel spreadsheets, though
Ignoreleading has no effect for csv files.
*/
void KTVTokenizer::_csvparse(char *buf,int len)
{
	bool inquotes;
	int lpos= 0;
	int i=0;
	inquotes= 0;
	while (i<len)
	{
		if (!buf[i]) break;
		else if (buf[i]=='"') inquotes= 1-inquotes;
		else if (buf[i]==','&&!inquotes)
		{
			buf[i]= 0;
			if (!_add(buf+lpos)) break;
			lpos= i+1;
		}
		++i;
	}
	buf[len]= 0;
	_add(buf+lpos);
	StripQuotes();
}

void KTVTokenizer::StripQuotes(void)
{
	for (iterator ii=begin();ii!=end();++ii)
	{
		if (ii->length()<2) continue;
		if ((*ii)[0]=='\"'&&(*ii)[ii->length()-1]=='\"')
		{
			ii->erase(0,1);
			ii->erase(ii->length()-1,1);
		}
	}
}

void KTVSearch::Clear(void)
{
	_s= "";
	_byrow= 0;
	if (_reg)
	{
		regfree(_reg);
		delete _reg;
	}
	_reg= NULL;
}

string KTVSearch::SetSearch(string x,bool byrow)
{
	Clear();
	_s= x;
	_byrow= byrow;
	if (_s.length()<2) return "";
	if (_s[0]!='/'||_s[_s.length()-1]!='/') return "";
	_s.erase(0,1);
	_s.erase(_s.length()-1,1);
	_reg= new regex_t;
	if (!_reg) { Clear(); return "SetSearch Failed to Allocate regex_t"; }
	int errcode= regcomp(_reg,_s.c_str(),REG_EXTENDED|REG_NOSUB);
	if (!errcode) return "";
	char buf[200]= "";
	regerror(errcode,_reg,buf,200);
	Clear();
	return buf;
}

// If x is empty, returns 0
bool KTVSearch::TestString(const string &x) const
{
	if (x=="") return 0;	
	if (_s==""&&!_reg) return 0;
	if (_reg)
	{
		int retcode= regexec(_reg,x.c_str(),0,NULL,0);
		if (retcode==REG_NOMATCH) return 0;
		else if (!retcode) return 1;
		else return 0;		// Error case, but treat as 0
	}
	else
	{
		return (x.find(_s)!=string::npos);
	}
}

