#include <stdio.h>
#include "KTVMain.h" 
void KTVMain::KeyHelp(void)
{
	KPRINT("*******************************************\n");
	KPRINT("**\n");
	KPRINT("*Action        Cols                 Rows*\n");
	KPRINT("*---------------------------------------------------*\n");
	KPRINT("*Move       a < l r > e    g b|u y|k j|ret f|d|spc G*\n");
	KPRINT("*(Un)Freeze   C [n|.]              R [n|.]          *\n");
	KPRINT("*(Un)Hide   H [n|.]                                 *\n");
	KPRINT("*Search     S|? [str|/str/]      s|/ [str|/str/]    *\n");
	KPRINT("*---------------------------------------------------*\n");
	KPRINT("*Col width:  +/-[n|.]  ( )  W[n:w] W[.:w] W[w]      *\n");
	KPRINT("*Cell just (lft|rgt):   ^|$ [n|.]  [ ]              *\n");
	KPRINT("*Mark set|recover:      m|\' [c]                     *\n");
	KPRINT("*Cell: L[row:col] (goto) v[row:col] (disp)          *\n");
	KPRINT("*General: q(quit) h(hlp) Y(draw) F(rsteof)          *\n");
	KPRINT("*Disp: x(grid) z(seps) P(rowidx) V(colidx)          *\n");
	KPRINT("*---------------------------------------------------*\n");
	KPRINT("************** Press q to return **********\n");
	KPRINT("*******************************************\n");
	while(CGETCH()!='q');
}


