#include "config.h"
#include <stdio.h>

int parse_config(config_t *config,char *filename)
{
  FILE *in = fopen(filename,"r");

  if (!in) {
    return errno;
  }

   
}
