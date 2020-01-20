#ifndef _EXEC_H_
#define _EXEC_H_

#include "defines.h"

#define PROC 1 /* named procedures  */

#if PROC
int storeproc (bool f, int n);
int execproc (bool f, int n);
#endif

extern int clexec; /* command line execution flag  */

int namedcmd (bool f, int n);
int execcmd (bool f, int n);
void gettoken (char *tok, int maxtoksize);
int gettokval (char *tok, int maxtoksize);
char *getnewtokval (void);
int storemac (bool f, int n);
int execbuf (bool f, int n);
int execfile (bool f, int n);
int dofile (const char *fname);

int cbuf1 (bool f, int n);
int cbuf2 (bool f, int n);
int cbuf3 (bool f, int n);
int cbuf4 (bool f, int n);
int cbuf5 (bool f, int n);
int cbuf6 (bool f, int n);
int cbuf7 (bool f, int n);
int cbuf8 (bool f, int n);
int cbuf9 (bool f, int n);
int cbuf10 (bool f, int n);
int cbuf11 (bool f, int n);
int cbuf12 (bool f, int n);
int cbuf13 (bool f, int n);
int cbuf14 (bool f, int n);
int cbuf15 (bool f, int n);
int cbuf16 (bool f, int n);
int cbuf17 (bool f, int n);
int cbuf18 (bool f, int n);
int cbuf19 (bool f, int n);
int cbuf20 (bool f, int n);
int cbuf21 (bool f, int n);
int cbuf22 (bool f, int n);
int cbuf23 (bool f, int n);
int cbuf24 (bool f, int n);
int cbuf25 (bool f, int n);
int cbuf26 (bool f, int n);
int cbuf27 (bool f, int n);
int cbuf28 (bool f, int n);
int cbuf29 (bool f, int n);
int cbuf30 (bool f, int n);
int cbuf31 (bool f, int n);
int cbuf32 (bool f, int n);
int cbuf33 (bool f, int n);
int cbuf34 (bool f, int n);
int cbuf35 (bool f, int n);
int cbuf36 (bool f, int n);
int cbuf37 (bool f, int n);
int cbuf38 (bool f, int n);
int cbuf39 (bool f, int n);
int cbuf40 (bool f, int n);

#endif
