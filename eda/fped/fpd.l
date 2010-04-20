%{
/*
 * fpd.l - FootPrint Definition language
 *
 * Written 2009, 2010 by Werner Almesberger
 * Copyright 2009, 2010 by Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#include <stdlib.h>

#include "util.h"
#include "coord.h"
#include "expr.h"
#include "error.h"
#include "meas.h"
#include "fpd.h"

#include "y.tab.h"


int start_token = START_FPD;

static int disable_keywords = 0;
static int is_table = 0;


void scan_empty(void)
{
	yy_scan_string("");
}


void scan_expr(const char *s)
{
	start_token = START_EXPR;
	yy_scan_string(s);
}


void scan_var(const char *s)
{
	start_token = START_VAR;
	yy_scan_string(s);
}


void scan_values(const char *s)
{
	start_token = START_VALUES;
	yy_scan_string(s);
}

%}


/* keywords are only accepted at the beginning of a line */
%s NOKEYWORD

NUM	[0-9]+\.?[0-9]*
SP	[\t ]*


%%

%{
	/*
	 * Nice hack:
	 *
	 * http://www.gnu.org/software/bison/manual/bison.html#
	 *   Multiple-start_002dsymbols
	 */

	if (start_token) {
		int tmp = start_token;
		start_token = 0;
		return tmp;
	}
%}


<INITIAL>"set"			{ BEGIN(NOKEYWORD);
				  return TOK_SET; }
<INITIAL>"loop"			{ BEGIN(NOKEYWORD);
				  return TOK_LOOP; }
<INITIAL>"part"			{ BEGIN(NOKEYWORD);
				  return TOK_PACKAGE; }
<INITIAL>"package"		{ BEGIN(NOKEYWORD);
				  return TOK_PACKAGE; }
<INITIAL>"frame"		{ BEGIN(NOKEYWORD);
				  is_table = 0;
				  return TOK_FRAME; }
<INITIAL>"table"		{ BEGIN(NOKEYWORD);
				  is_table = 1;
				  return TOK_TABLE; }
<INITIAL>"vec"			{ BEGIN(NOKEYWORD);
				  return TOK_VEC; }
<INITIAL>"pad"			{ BEGIN(NOKEYWORD);
				  return TOK_PAD; }
<INITIAL>"rpad"			{ BEGIN(NOKEYWORD);
				  return TOK_RPAD; }
<INITIAL>"rect"			{ BEGIN(NOKEYWORD);
				  return TOK_RECT; }
<INITIAL>"line"			{ BEGIN(NOKEYWORD);
				  return TOK_LINE; }
<INITIAL>"circ"			{ BEGIN(NOKEYWORD);
				  return TOK_CIRC; }
<INITIAL>"arc"			{ BEGIN(NOKEYWORD);
				  return TOK_ARC; }
<INITIAL>"meas"			{ BEGIN(NOKEYWORD);
				  return TOK_MEAS; }
<INITIAL>"measx"		{ BEGIN(NOKEYWORD);
				  return TOK_MEASX; }
<INITIAL>"measy"		{ BEGIN(NOKEYWORD);
				  return TOK_MEASY; }
<INITIAL>"unit"			{ BEGIN(NOKEYWORD);
				  return TOK_UNIT; }

<INITIAL>"%del"			{ BEGIN(NOKEYWORD);
				  return TOK_DBG_DEL; }
<INITIAL>"%move"		{ BEGIN(NOKEYWORD);
				  return TOK_DBG_MOVE; }
<INITIAL>"%print"		{ BEGIN(NOKEYWORD);
				  return TOK_DBG_PRINT; }
<INITIAL>"%dump"		{ BEGIN(NOKEYWORD);
				  return TOK_DBG_DUMP; }
<INITIAL>"%exit"		{ BEGIN(NOKEYWORD);
				  return TOK_DBG_EXIT; }

<INITIAL>[a-zA-Z_][a-zA-Z_0-9]*: { *strchr(yytext, ':') = 0;
				  yylval.id = unique(yytext);
				  return LABEL; }
[a-zA-Z_][a-zA-Z_0-9]*		{ yylval.id = unique(yytext);
				  return ID; }

{NUM}{SP}mm			{ yylval.num.type = nt_mm;
				  yylval.num.exponent = 1;
				  sscanf(yytext, "%lf", &yylval.num.n);
				  return NUMBER; }
{NUM}{SP}mil			{ yylval.num.type = nt_mil;
				  yylval.num.exponent = 1;
				  sscanf(yytext, "%lf", &yylval.num.n);
				  return NUMBER; }
{NUM}				{ yylval.num.type = nt_mm; /* mm or mil */
				  yylval.num.exponent = 0;
				  sscanf(yytext, "%lf", &yylval.num.n);
				  return NUMBER; }

\"(\\[^\n\t]|[^\\"\n\t])*\"	{ *strrchr(yytext, '"') = 0;
				  yylval.str = stralloc(yytext+1);
				  return STRING; }

"->"				return TOK_NEXT;
"<-"				return TOK_NEXT_INVERTED;
">>"				return TOK_MAX;
"<<"				return TOK_MAX_INVERTED;

{SP}				;
\n				{ if (!disable_keywords)
					BEGIN(INITIAL);
				  lineno++; }

;				BEGIN(INITIAL);

"{"				{ BEGIN(NOKEYWORD);
				  disable_keywords = is_table;
				  return '{'; }
"}"				{ disable_keywords = 0;
				  return '}'; }

^#\ [0-9]+\ \"[^"]*\"(\ [0-9]+)*\n {
				  if (!disable_keywords)
				  	BEGIN(INITIAL);
				  lineno = strtol(yytext+2, NULL, 0); }

.				return *yytext;
