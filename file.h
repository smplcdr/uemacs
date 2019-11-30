#ifndef _FILE_H_
#define _FILE_H_

#include "buffer.h"
#include "retcode.h"

extern int restflag; /* restricted use?              */
int resterr (void);  /* restricted error message		*/

int fileread (int f, int n);
int insfile (int f, int n);
int filefind (int f, int n);
int viewfile (int f, int n);
int getfile (const char *fname, int lockfl);
int readin (const char *fname, int lockfl);
void makename (bname_t bname, const char *fname);
void unqname (char *name);
int filewrite (int f, int n);
int filesave (int f, int n);
int writeout (const char *fn);
int filename (int f, int n);

#endif
