%token TOK_IDENTIFIER
%token TOK_STRING
%token TOK_FALSE
%token TOK_TRUE
%token TOK_WHITE
%token TOK_UNKNOWN
%token TOK_INTEGER
%%

config
: lws TOK_IDENTIFIER lws value
;

lws
: /* empty */
| lws TOK_WHITE 
;

value
: TOK_STRING
| TOK_FALSE
| TOK_TRUE
| TOK_WHITE
| TOK_INTEGER
;

