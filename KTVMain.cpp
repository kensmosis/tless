#include <getopt.h>
#include "KTVMain.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>


void KTVMain::ReadArguments(int argc,char *const *argv)
{
	static struct option lo[] =
	{
		{ "help",		no_argument,		NULL,	'h' },
		{ "file",		required_argument,	NULL,	'f' },
		{ "skip",		required_argument,	NULL,	'k' },
		{ "cache",	required_argument,	NULL,	'z' },
		{ "delim",	required_argument,	NULL,	'd' },
		{ "sdelim",	required_argument,	NULL,	's' },
		{ "csv",		no_argument,		NULL,	'c' },
		{ "fixed",	required_argument,	NULL,	'x' },
		{ "ignoreleading",	required_argument,	NULL,	'l' },
		{ "doseol",	no_argument,		NULL,	'y' },
		{ "eol",		required_argument,	NULL,	'e' },
		{ "aligntofirst",	no_argument,	NULL,	'a' },
		{ "defwidth",	required_argument,	NULL,	'w' },
		{ "grid",		optional_argument,	NULL,	'G' },
		{ "seps",		optional_argument,	NULL,	'S' },
		{ "rowindex",	required_argument,	NULL,	'R' },
		{ "colindex",	required_argument,	NULL,	'C' },
		{ "justify",	required_argument,	NULL,	'j' },
		{ "filler",	required_argument,	NULL,	'F' },
		{ "maxcols",	required_argument,  NULL, 	'M' },
		{ "stripquotes",	no_argument,	NULL,	'Q' },
		{ "exec",		required_argument,  NULL, 	'X' },
		{ "allowexpansion",	no_argument,	NULL,	'm' },
		{ "version",	no_argument, 		NULL,	'v' },
		{ "tab",		no_argument,		NULL,	't' }
	};
	int c;
	while ((c = getopt_long(argc,argv,"hf:k:z:d:s:cx:l:ye:aw:G:S:R:C:j:F:M:QX:mvt", lo, NULL))!= -1)
	{
		#ifdef KTVTESTMODE
		fprintf(stderr,"Registered option %c with arg[%s]\n",c,optarg?optarg:"");
		fflush(stderr);
		#endif
		switch (c)
		{
		case 'f':
			if (_filename!=""&&_filename!="stdin") { fprintf(stderr,"Arg Error:  File specified twice\n"); exit(1); }
			_filename= optarg;
			break;
		case 'k':
			_cr.SetRowSkip(atoi(optarg));
			break;
		case 'z':
			_fm.SetCacheSize(atoi(optarg));
			break;
		case 'd':
			_tok._delim= optarg;
			_tok._strict= 0;
			break;
		case 's':
			_tok._delim= optarg;
			_tok._strict= 1;
			break;
		case 'c':
			_tok._csv= 1;
			break;
		case 'x':
			{
			KTVTokenizer foo;
			foo._delim= ":";
			foo._strict= 1;
			foo._ignoreleading= 1;
//			foo._trimtrailingcr= 0;
			foo.ParseNonDestructive(optarg);
			for (KTVTokenizer::iterator ii=foo.begin();ii!=foo.end();++ii)
			{
				int w= atoi(ii->c_str());
				if (w>0)
				{
					_tok._fixedcols.push_back(w);
					_cr.AddCol(w);		// Note that the order of args can make a difference in choice of rjustify default!
				}
			}
			}
			break;
		case 'l':
			if (!strcmp(optarg,"on")) _tok._ignoreleading= 1;
			else if (!strcmp(optarg,"off")) _tok._ignoreleading= 0;
			else { fprintf(stderr,"Arg Error:  Invalid value for ignoreleading\n"); exit(1); }
			break;
		case 'y':
			_fm.SetEOLChar('\r');
			break;
		case 'e':
			_fm.SetEOLChar(optarg[0]);
			break;
		case 'a':
			_cr.SetAlignToFirst();
			break;
		case 'w':
			_cr.SetDefaultColWidth(atoi(optarg));
			break;
		case 'G':
			_sc.SetGrid(1);
			if (optarg) _sc.SetGridSep(optarg[0]);
			break;
		case 'S':
			_sc.SetSeps(1);
			if (optarg) _sc.SetFrozenSep(optarg[0]);
			break;
		case 'R':
			if (!strcmp(optarg,"on")) _sc.SetRowIndex(1);
			else if (!strcmp(optarg,"off")) _sc.SetRowIndex(0);
			else 
			{
				_sc.SetRowIndex(1);
				_sc.ResizeRowIndex(atoi(optarg));
			}
			break;
		case 'C':
			if (!strcmp(optarg,"on")) _sc.SetColIndex(1);
			else if (!strcmp(optarg,"off")) _sc.SetColIndex(0);
			else { fprintf(stderr,"Arg Error:  Invalid value for colindex flag\n"); exit(1); }
			break;
		case 'j':
			if (!strcmp(optarg,"right")) _cr.SetDefaultRightJustify();
			else if (!strcmp(optarg,"off")) _cr.SetDefaultLeftJustify();
			else { fprintf(stderr,"Arg Error:  Invalid value for justify\n"); exit(1); }
			break;
		case 'F':
			_sc.SetFiller(optarg[0]);
			break;
		case 'M':
			{
			int ncols= atoi(optarg);
			if (ncols>0)
			{
				_cr.SetMaxCols(ncols);
				_tok._maxcols= ncols;
			}
			}
		case 'Q':
			_tok.SetStripQuotes();
			break;
		case 'X':
			_cmds.push_back(optarg);
			break;
		case 'm':
			_fm.AllowExpansion();
			break;
		case 'v':
			printf("tless version 1\n");
			exit(0);
		case 't':
			_tok._delim+= "\t";
			break;
		case 'h':
		default:
			Usage();
			exit(0);
		}
	}	
}

bool KTVMain::InitFromArgs(void)
{
	// Screen
	_sc.InitCurses();

	// FileManager
	if (_filename==""||_filename=="stdin") _fm.SetCacheSize(0);	// Remember all rows
	_fm.InitCache();
	if (_fil!=NULL) return 0;
	if (_filename==""||_filename=="stdin") 
	{
		_fil= stdin;
		_fm.AttachFile(stdin);
		_fm.AllowExpansion();
	}
	else
	{
		_fil= fopen(_filename.c_str(),"r");
		if (!_fil) return 0;
		_fm.AttachFile(_fil);
	}

	// Col/Row Manager
	_cr.Move(_cr.BaseRow(),0);

	return 1;
}

KTVMain::~KTVMain(void)
{
	if (_fil&&_fil!=stdin) fclose(_fil);
}

void KTVMain::PreExecute(const string &x)
{
	string s= x;
	if (s.length()<2) return;
	if (s[0]=='\"'&&s[s.length()-1]=='\"')
	{
		s.erase(0,1);
		s.erase(s.length()-1,1);
	}
	KTVTokenizer foo;
	foo._delim= " ";
	foo._ignoreleading= 1;
//	foo._trimtrailingcr= 1;
	foo._strict= 0;
	foo._stripquotes= 0;
	foo.ParseNonDestructive(s.c_str());
	for (vector<string>::const_iterator ii=foo.begin();ii!=foo.end();++ii)
	{
		string z= *ii;
		if (z[1]!=':'&&z.length()>1) continue;		// Error
		char c= z[0];
		z.erase(0,2);
		ExecCommand(c,z);
	}
}

void KTVMain::Run(int argc,char *const *argv)
{
	const char *ca= "CRsS/?vH+-Wm\'$^L";		// chars that require further input
	ReadArguments(argc,argv);
	if (!_tok.MakeValidSpec()) { fprintf(stderr,"Invalid overall spec for parsing.\n"); exit(1); }
	InitFromArgs();
	_cr.RedrawIfNeeded();
	for (vector<string>::const_iterator ii=_cmds.begin();ii!=_cmds.end();++ii)
		PreExecute(*ii);
	_readytoresize= 1; 
	while (1)
	{
		char c= KTVScreen::GetNextKey();
		if (c=='q') break;
		string a= "";
		if (strrchr(ca,c)!=NULL) a=_sc.GetCommand(c);
		ExecCommand(c,a);
	}
}

void KTVMain::ExecCommand(char c,const string &arg)
{
	int cl= _cr.GetLeftActiveCol();
	int cr= _cr.GetRightActiveCol();
	int rt= _cr.GetTopActiveRow();
	int rb= _cr.GetBottomActiveRow();
	_cr.ClearFlag();
	int n;

#ifdef KTVTESTMODE
	fprintf(stderr,"ExecCommand received char [%c] and arg[%s]\n",c,arg.c_str());
#endif

	switch(c)
	{
	case 'h':	KeyHelp(); _cr.DemandRedraw();  break;
	case 'a':	_cr.MoveToCol(0);	break;
	case 'e':	_cr.MoveToLastCol();	break;
	case 'l':	_cr.MoveToCol(_cr.PrevActiveCol(cl));  break;
	case 'r': _cr.MoveToCol(_cr.NextActiveCol(cl)); break;
	case '>': _cr.MoveToCol(cr);  break;
	case '<': _cr.MoveToCol(_cr.GetMinColFittingFromRight(cl));	break;
	case 'g': _cr.MoveToRow(_cr.BaseRow());  break;
	case 'G':	_cr.MoveToLastRow(); break;
	case 'f': 
	case ' ':
	case 'd':
			_cr.MoveToRow(rb); break;
	case 'b': 
	case 'u': 
			_cr.MoveToRow(_cr.NextActiveRow(rt-_sc.ActiveRegionHeight())); break;	// Approx 
	case 'y':
	case 'k':
			_cr.MoveToRow(_cr.PrevActiveRow(rt)); break;
	case 'j':
		// *** Add return here like d or j!
			_cr.MoveToRow(_cr.NextActiveRow(rt)); break;
	case 'C':
			if (arg==".") _cr.UnFreezeAllCols();
			else
			{
				n= strtol(arg.c_str(),NULL,10);
				_cr.ToggleFreezeCol(n);
			}
			break;
	case 'R':
			if (arg==".") _cr.UnFreezeAllRows();
			else
			{
				n= strtol(arg.c_str(),NULL,10);
				_cr.ToggleFreezeRow(n);		 // Only allow if on screen!
			}
			break;
	case 's':
	case '/':
			_search.SetSearch(arg.c_str(),1);
			_cr.Find(_search,0,1);
			break;
	case 'S':
	case '?':
			_search.SetSearch(arg.c_str(),0);
			_cr.Find(_search,0,1);
			break;
	case 'n':
			_cr.Find(_search,0,0);
			break;
	case 'p':
			_cr.Find(_search,1,0);
			break;
	case 'v':
			{
			char buf[200];
			strncpy(buf,arg.c_str(),199);
			char *p= strtok(buf,":");
			int row= -1;
			int col= -1;
			if (p)
			{
				row= strtol(p,NULL,10);
				p= strtok(NULL,":");
				if (p) col= strtol(p,NULL,10);
			}
			if (row>=0&&col>=0&&col<_cr.GetNumCols())
			{
				KTVRow *r= _fm.GetRow(row);
				if (r) _sc.StatusMessage(r->GetVal(col)); 
			}
			}
			break;
	case 'H':
			if (arg==".") _cr.ExposeAllCols();
			else
			{
				n= strtol(arg.c_str(),NULL,10);
				_cr.ToggleHideCol(n);
			}
			break;
	case '+':
			if (arg==".") _cr.IncreaseAllColWidths(1);
			else
			{
				n= strtol(arg.c_str(),NULL,10);
				_cr.IncreaseColWidth(n,1);
			}
			_cr.Restate();
			break;
	case '-':
			if (arg==".") _cr.IncreaseAllColWidths(-1);
			else
			{
				n= strtol(arg.c_str(),NULL,10);
				_cr.IncreaseColWidth(n,-1);
			}
			_cr.Restate();
			break;
	case 'W':
			{
			char buf[200];
			strncpy(buf,arg.c_str(),199);
			char *p= strtok(buf,":");
			if (p)
			{
				string col= p;
				p= strtok(NULL,":");
				if (p) 
				{
					int wid= strtol(p,NULL,10);
					if (wid>0&&col==".") _cr.SetAllColWidths(wid);
					else if (wid>0)
					{
						n= strtol(col.c_str(),NULL,10);
						_cr.SetColWidth(n,wid);
					}
				}
			}
			}
			_cr.Restate();
			break;
	case 'm':
			_cr.AddReplaceMark(arg[0]);
			break;			
	case '\'':
			_cr.RestoreMark(arg[0]);
			break;
	case '^':
			if (arg==".") _cr.LeftJustifyAllCols();
			else
			{
				n= strtol(arg.c_str(),NULL,10);
				_cr.LeftJustifyCol(n);
			}
			break;
	case '$':
			if (arg==".") _cr.RightJustifyAllCols();
			else
			{
				n= strtol(arg.c_str(),NULL,10);
				_cr.RightJustifyCol(n);
			}
			break;
	case 'x':
			_sc.ToggleGrid();
			_cr.Restate();
			break;
	case 'z':
			_sc.ToggleSeps();
			_cr.Restate();
			break;
	case 'P':
			_sc.ToggleRowIndex();
			_cr.Restate();
			break;
	case 'V':			
			_sc.ToggleColIndex();
			_cr.Restate();
			break;
	case 'L':
			{
			char buf[200];
			strncpy(buf,arg.c_str(),199);
			char *p= strtok(buf,":");
			int row= -1;
			int col= -1;
			if (p)
			{
				row= strtol(p,NULL,10);
				p= strtok(NULL,":");
				if (p) col= strtol(p,NULL,10);
			}
			if (row>=0&&col>=0) _cr.Move(row,col);
			}
			break;
	case 'Y':
			_sc.RefitTerminal();
			_cr.Restate();
			break;
	case 'F':	
			if (_fm.EOFKnown()&&_fm.IsExpandable()) _cr.SeekEnd();
			_cr.Restate();
			break;
//	case 'q':		// 'q' is dealt with explicitly in other code
//			exit(0);
	default:
			{
			const char *err= "Unknown command";
			_sc.StatusMessage(err);
			}
	}
	_cr.RedrawIfNeeded();
}

void KTVMain::ResizeIfReady(void)
{
	if (_readytoresize)	// acts as a primitive type of mutex
	{
		_readytoresize= 0;
		_sc.RefitTerminal();
		_cr.Restate();
		_cr.RedrawIfNeeded();
		_readytoresize= 1;
	}
}
