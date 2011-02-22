#ifndef KTVIDXLISTDEFFLAG
#define KTVIDXLISTDEFFLAG

#include <list>
#include <map>
using namespace std;

// Utility Class

typedef list<pair<int,int> > KTVIdxListList;
class KTVIdxList
{
protected:
	KTVIdxListList _l;
	map<int,int> _m;		// Acts as set or map.  Idx->Size
	int _num;
	int _size;
public:
	KTVIdxList(void) : _l(), _m(), _num(0), _size(0) {}
	KTVIdxList(const KTVIdxList &x) : _l(x._l), _m(x._m), _num(x._num), _size(x._size) {}
	int GetSize(int n) const 	// 0 if doesn't find it
	{
		map<int,int>::const_iterator ii= _m.find(n);
		return (ii==_m.end())?0:ii->second;
	}
	int GetNum(void) const { return _num; }
	bool Exists(int n) const { return (GetSize(n)!=0); }
	int GetSize(void) const { return _size; }
	const KTVIdxListList &GetList(void) const { return _l; }
	int Add(int n,int sz,bool tofront)	// Returns size added
	{
		if (sz<=0) return 0;
		if (_m.find(n)!=_m.end()) return 0;
		_m.insert(map<int,int>::value_type(n,sz));
		if (!tofront) _l.push_back(pair<int,int>(n,sz));
		else _l.push_front(pair<int,int>(n,sz));
		++_num;
		_size+= sz;
		return sz;
	}
	int AddFront(int n,int sz) { return Add(n,sz,1); }
	int AddBack(int n,int sz) { return Add(n,sz,0); }
	int Remove(int n)		// Returns size removed
	{
		map<int,int>::iterator ii= _m.find(n);
		if (ii==_m.end()) return 0;
		int sz= ii->second;
		_size-= sz;
		--_num;
		_m.erase(ii);
		_l.remove(pair<int,int>(n,sz));
		return sz;
	}
	void Clear(void)
	{
		_num= _size= 0;
		_m.clear();
		_l.clear();
	}
	bool IsEmpty(void) const { return (_num==0); }
};

#endif
