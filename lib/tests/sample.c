#include <stdlib.h> /* For NULL */
#include "config.h"

int foo;

config_t config[] = {
	{ "foo", TYPE_INT|TYPE_NOTNULL, &foo },
 	{ NULL, 0, NULL }
	};

int main(int argc,char **argv) 
{
  if(parse_config(config,argv[1])) {
    return 1;
  }

  printf("foo=%i\n",foo);

  return 0;
}

