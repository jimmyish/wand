%%

[A-Za-z0-9]+	{ return TOK_IDENTIFIER; }
"[^"]*"		{ return TOK_STRING; }
'[^']*"		{ return TOK_STRING; }
false		{ return TOK_FALSE; }
off		{ return TOK_FALSE; }
true		{ return TOK_TRUE; }
on		{ return TOK_TRUE; }
[0-9]+		{ return TOK_INTEGER; }
[:space:]	{ return TOK_WHITE; }
.		{ return TOK_UNKNOWN; }

%%
