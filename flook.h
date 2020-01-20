
#ifndef _FLOOK_H
#define _FLOOK_H 1

#include "defines.h"

#define rcfname  pathname[0]
#define hlpfname pathname[1]

extern const char *pathname[];

bool fexist (const char *fname);
char *flook (const char *fname, int hflag);

#endif
