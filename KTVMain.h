#ifndef KTVMAINDEFFLAG
#define KTVMAINDEFFLAG

#include <string>
#include "KTVFileManager.h"
#include "KTVScreen.h"
#include "KTVColRowManager.h"
#include "KTVTokenizer.h"
using namespace std;

#ifdef KTVTESTMODE
#define KPRINT 	printf
#else
#define KPRINT		printw
#endif

class KTVMain
{
protected:
	string _filename;
	KTVFileManager _fm;
	KTVScreen _sc;
	KTVColRowManager _cr;
	KTVTokenizer _tok;
	FILE *_fil;
	KTVSearch _search;
	vector<string> _cmds;
	int _readytoresize;
public:
	KTVMain(void) : _filename("stdin"), _fil(NULL), _search(), _cmds(), _readytoresize(0) 
	{
		_fm.AttachTokenizer(&_tok);
		_cr.Attach(&_sc,&_fm);
	}
	~KTVMain(void); 
	bool InitFromArgs(void);
	void ReadArguments(int argc,char *const *argv);
	void PreExecute(const string &x);
	void Run(int argc,char *const *argv);
	void ExecCommand(char cmd,const string &arg);
	static void Usage(void);
	static void KeyHelp(void);
	void ResizeIfReady(void);
};

#endif
