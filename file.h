#ifndef _FILE_H_
#define _FILE_H_

#include "buffer.h"
#include "retcode.h"

extern bool restflag; /* Restricted use?  */
extern int resterr (void); /* Restricted error message.  */

extern int fileread (bool f, int n);
extern int insfile (bool f, int n);
extern int filefind (bool f, int n);
extern int viewfile (bool f, int n);
extern int getfile (const char *fname, bool lockfl);
extern int readin (const char *fname, bool lockfl);
extern void makename (bname_t bname, const char *fname);
extern void unqname (char *name);
extern int filewrite (bool f, int n);
extern int filesave (bool f, int n);
extern int writeout (const char *fn);
extern int filename (bool f, int n);

#endif
