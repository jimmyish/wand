#include <stdlib.h> /* For NULL */
#include <stdio.h> /* For Printf */
#include "config.h"

int foo;
char *baz=NULL;
int boing;

config_t config[] = {
	{ "foo", TYPE_INT|TYPE_NOTNULL, &foo },
	{ "baz", TYPE_STR|TYPE_NOTNULL, &baz },
	{ "boing", TYPE_BOOL|TYPE_NULL, &boing },
 	{ NULL, 0, NULL }
	};

int main(int argc,char **argv) 
{
  if(parse_config(config,argv[1])) {
    return 1;
  }

  printf("----------------\n");
  printf("foo=%i\n",foo);
  printf("baz=%s\n",baz);
  printf("boing=%i\n",boing);

  return 0;
}

