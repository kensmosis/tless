#include <assert.h>
#include <string.h>
#include "KTVFileManager.h"

// My more flexible version of Kfgets.  maxlen includes null terminator.  If eol is 0 then any of '\r' or '\n' will match it.  Multiple such chars are NOT merged!  Always terminates on a null.  
// Unlike fgets, returns a code.  Number of chars read or -1(EOF), -2(error), -3(ran out of space in tgt)
// If we do run out of space, we continue to scan until EOL but only record the first maxlen-1 chars in the buffer.  We are guarantee on return that the seek location is one past the eol char (or the eof) unless an error code is returned.
int KTVFileManager::Kfgets(char *tgt,int maxlen,FILE *f,char eol)
{
	if (!tgt||!f||maxlen<2) return -2;
	int numread=0;
	char c=0;
	clearerr(f);
	tgt[0]= 0;
	while (1)
	{
		c= fgetc(f);
		if (c==EOF)
		{
			tgt[numread]= 0;
			if (ferror(f)!=0) return -2;
			else if (feof(f)!=0) return -1;
			else { assert(0); return -2; }
		}
		else if (!c||c==eol||(!eol&&(c=='\n'||c=='\r')))
		{
			tgt[numread]= 0;
			return numread;
		}
		else if (numread<maxlen-1) // Only add a char if still under limit
		{
			tgt[numread]= c;
			++numread;
		}
	}
	assert(numread==maxlen-1);
	tgt[numread]= 0;
	return -3;
}

void KTVCache::ClearCache(void)
{
	for (map<int,KTVRow *>::iterator ii=_r.begin();ii!=_r.end();++ii)
		if (ii->second) delete (ii->second);
	for (map<int,KTVRow *>::iterator ii=_fr.begin();ii!=_fr.end();++ii)
		if (ii->second) delete (ii->second);
	_r.clear();
	_fr.clear();
	_l.clear();
	_size= 0;
	_knownmax= 0;
	// preserves _maxsize and _numfromend
}


// 1 if success or if already frozen.  0 if row doesn't exist
bool KTVCache::Freeze(int row)
{
	if (_fr.find(row)!=_fr.end()) return 1;
	map<int,KTVRow *>::iterator ii= _r.find(row);
	if (ii==_r.end()) return 0;
	_fr.insert(*ii);
	_l.remove(ii->first);
	_r.erase(ii);
	--_size;
	return 1;
}

// Returns 0 if exists or fails to ad for some reason
bool KTVCache::AddRowFromTokenizer(int row,const KTVTokenizer &x)
{
	if (row<0) return 0;
	if (GetRow(row)) return 0;	// Protect against existence in either cache
	bool fz= 0;
	if (_numfromend>0&&row<_numfromend) fz= 1;
	else if (_numfromend>0&&_knownmax>=0&&row>_knownmax-_numfromend) fz= 1;
	if (fz)
	{
		KTVRow *r= KTVRow::FromTokenizer(x);
		if (!r) return 0;
		_fr.insert(map<int,KTVRow *>::value_type(row,r));
	}
	else
	{
		KTVRow *r= KTVRow::FromTokenizer(x);
		if (!r) return 0;
		_r.insert(map<int,KTVRow *>::value_type(row,r));
		_l.push_back(row);
		++_size;
		// Free from back of cache if needed
		if (_maxsize>0&&_size>_maxsize)
		{
			int vr= _l.front();
			_l.pop_front();
			_r.erase(vr);
			--_size;
		}
	}
	return 1;
}

void KTVCache::SetKnownMax(int n)
{
	if (n<0) return;
	_knownmax= n;

	// Convert over all within end cache to fixed
	if (_numfromend>0)
	{
		list<int> z;
		for (list<int>::iterator ii=_l.begin();ii!=_l.end();++ii)
			if ((*ii)>_knownmax-_numfromend) z.push_back(*ii);
		for (list<int>::iterator ii=z.begin();ii!=z.end();++ii) Freeze(*ii);
	}
}

KTVRow *KTVCache::GetRow(int n)
{
	map<int,KTVRow *>::iterator ii;
	if ((ii=_fr.find(n))!=_fr.end()) return ii->second;
	if ((ii=_r.find(n))!=_r.end()) return ii->second;
	return NULL;
}

void KTVCache::Init(int maxsize,int numfromendtolock)
{
	_size= 0;
	_maxsize= (maxsize>0)?maxsize:0;
	_numfromend= (numfromendtolock>=0)?numfromendtolock:0;
	_fr.clear();
	_r.clear();
	_l.clear();
	_knownmax= -1;
}

bool KTVFileManager::AttachFile(FILE *f)
{
	if (_f||!f) return 0;
	_f= f;
	if (IsSeekable()) rewind(_f);
	_seeklocs.clear();
	_seeklocs.push_back(0);
	_maxrow= -1;
	return 1;
}

vector<int> KTVFileManager::Extract(int minrow,int maxrow,int currnumcols)
{
	vector<int> foo;
	if (!IsActive()) return foo;
	if (minrow<0) minrow= 0;
	if (EOFKnown()&&maxrow>GetEndRow()&&!IsExpandable()) maxrow= GetEndRow();
	if (maxrow<minrow) return foo;

	for (int i=minrow;i<=maxrow;++i)
	{
		if (_c.GetRow(i)!=NULL);
		else if (!FindAndReadRow(i,foo,currnumcols)) break;
	}
	return foo;
}

bool KTVFileManager::AddParseRow(int i,char *buf,vector<int> &foo,int currnumcols)
{
	if (!buf||!_tok) return 0;
	_tok->Parse(buf);
	if (!_c.AddRowFromTokenizer(i,*_tok)) { assert(0); return 0; }
	
	// Add new cols if needed
	int cn= foo.size()+currnumcols;
	if (cn<_tok->size())
	{
		for (int j=cn;j<_tok->size();++j) 
			foo.push_back((*_tok)[j].length());
	}
	return 1;
}

/*

If we encounter a partial row, we do not process it.  If the last line in a file has no eol then this may cause it to be omitted.  However, it also allows us to properly acquire new lines

Note that when this is called, we are guaranteed that the row doesn't exist in cache!

if i==-1 we seek to end of file

*/
bool KTVFileManager::FindAndReadRow(int i,vector<int> &foo,int currnumcols)
{
	char *buf= KTVTokenizer::BB();		// Useful empty buffer
	int err;
	if (!IsActive()) return 0;
	if (i<-1) return 0;

	// Check if past the end of a fixed file
	if (EOFKnown()&&!IsExpandable()&&i>GetEndRow()) return 0;  // Note that GetEndRow()>=-1 always!!
	else if (i==-1&&EOFKnown()&&!IsExpandable()) return 1;	// Already know end of file

	// If already read the row
	if (i>=0&&i<=GetLastRowRead())
	{
		if (!IsCached()) { assert(0); return 0; }  // Should be in the cache!
		else if (!IsSeekable()) { assert(0); return 0; }  // Test.  If not seekable then must have cache=0 (but not necessarily converse)
		if (fseek(_f,_seeklocs[i],SEEK_SET)!=0) { assert(0); return 0; }  // Why can't we seek back to it?
		err= Kfgets(buf,KTVBIGBUFFERSIZE-1,_f,_eol); 
		if (err==-2) { assert(0); return 0; }		// Shouldn't have a read error.
		else if (err==-1) 		// EOF encountered
		{ 
			assert(EOFKnown()); 		// It better be if we've already encountered it!
			assert(i==_maxrow);	// We can't have passed the end, so it better be the same EOF as before!
		}
		else return AddParseRow(i,buf,foo,currnumcols);
	}
	else
	{
		if (EOFKnown()) assert(IsExpandable());		// This must be true, so test for it

		// Seek next unread row if not stdin
		if (IsSeekable()&&fseek(_f,_seeklocs.back(),SEEK_SET)!=0) { assert(0); return 0; }

		// Read until we reach the row or EOF
		while (GetLastRowRead()<i||i==-1)
		{
			err= Kfgets(buf,KTVBIGBUFFERSIZE-1,_f,_eol);
			if (err==-2) { assert(0); return 0; }		// Shouldn't have a read error
			else if (err==-1) 	// 		EOF.  Maybe old one, maybe new one
			{
				if (strlen(buf)<=0) // Don't change anything if empty last row.  Succeeded if i==-1!  Failed otherwise.
				{
					SetKnownMax(GetLastRowRead());
					return (i==-1);		
				}
				SetKnownMax(GetLastRowRead()+1);
				if (!IsSeekable()||GetEndRow()==i)		// If not seekable, must add the row or lose it forever!  _maxrow has changed here, so add if it is equal also!  Ok even if i==-1
				{
					if (!AddParseRow(GetEndRow(),buf,foo,currnumcols)) return 0;
				}
				if (IsSeekable()) _seeklocs.push_back(ftell(_f));		// Also brings GetLastRowRead into line!
				return (i==-1);		// Didn't reach row i
			}
			else 
			{
				if (!IsSeekable()||GetLastRowRead()+1==i) // If not seekable, must add the row or lose it forever!  OK  even if i==-1
				{
					if (!AddParseRow(GetLastRowRead()+1,buf,foo,currnumcols)) return 0;
				}
				if (IsSeekable()) _seeklocs.push_back(ftell(_f));
				else SetKnownMax(GetEndRow()+1);
			}
		}
		if (i>0&&GetLastRowRead()!=i) { assert(0); return 0; }
		else return 1;
	}
	assert(0);	// should never reach here
	return 0;	
}

