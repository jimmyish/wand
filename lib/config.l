%{
#include <string.h>
#include "config.h"
%}


%%

\"[^"]*\"	{ yylval=(char*)strdup(yytext); return TOK_STRING; }
\'[^']*\"	{ yylval=(char*)strdup(yytext); return TOK_STRING; }
false		{ return TOK_FALSE; }
off		{ return TOK_FALSE; }
true		{ return TOK_TRUE; }
on		{ return TOK_TRUE; }
[0-9]+		{ yylval=atoi(yytext); return TOK_INTEGER; }
[A-Za-z0-9]+	{ yylval=(char*)strdup(yytext); return TOK_IDENTIFIER; }
'\n'		{ return TOK_EOL; }
[[:blank:]]	{ return TOK_WHITE; }
.		{ return TOK_UNKNOWN; }

%%

void yyerror(char *s)
{
	fprintf(stderr,"%s\n",s);
}
