%{
#include "config_internal.h"
%}
%token TOK_IDENTIFIER
%token TOK_STRING
%token TOK_FALSE
%token TOK_TRUE
%token TOK_WHITE
%token TOK_EOL
%token TOK_UNKNOWN
%token TOK_INTEGER
%%

config
: /* Empty */ 
| config directive
| error TOK_EOL { printf("Expected configuration directive\n"); }
;

directive
: lws id lws value lws { set_config_int($2,$4); }
;

lws
: /* empty */
| lws TOK_WHITE { }
;

id
: TOK_IDENTIFIER {  $$=$1; }
;

value
: TOK_STRING
| TOK_FALSE
| TOK_TRUE
| TOK_WHITE
| TOK_INTEGER { $$ = $1; }
;

