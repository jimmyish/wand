#include "debug.h"

extern int modtolevel[];

int main(int argc, char **argv)
{
	modtolevel[MOD_MISC] = 1;

	logger(MOD_MISC, 1, "Hello world\n");

	return 0;
}
