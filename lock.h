#ifndef _LOCK_H
#define _LOCK_H 1

#ifndef _ESTRUCT_H
# error uEmacs compilation settings needs to be done!
#endif

#if BSD | SVR4
int lockchk (const char *fname);
int lockrel (void);
int lock (const char *fname);
int unlock (const char *fname);
#endif

#endif
