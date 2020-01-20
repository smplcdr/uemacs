#ifndef _EVAL_H
#define _EVAL_H 1

#include "defines.h"

#include <stddef.h>

#define DEBUGM 1 /* $debug triggers macro debugging */

#if DEBUGM
int mdbugout (char *fmt, ...);
#endif

extern bool macbug;    /* macro debuging flag          */
extern bool cmdstatus; /* last command status          */
extern int rval;       /* return value of a subprocess */
extern size_t envram;  /* # of bytes current in use by malloc */

int readfirst_f (void);
int is_it_cmd (char *token);

void varinit (void);
int setvar (bool f, int n);
char *getval (char *token);
int stol (char *val);
char *mklower (char *str);

int clrmes (bool f, int n);
int writemsg (bool f, int n);

#endif
