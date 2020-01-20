#ifndef _BIND_H
#define _BIND_H 1

#include "defines.h"

#define APROP 1 /* Add code for Apropos command                 */

#if APROP
int apro (bool f, int n);
#endif

/* Some global fuction declarations. */
typedef int (*fn_t) (bool, int);

int help (bool f, int n);
int deskey (bool f, int n);
int bindtokey (bool f, int n);
int unbindkey (bool f, int n);
int desbind (bool f, int n);
int startup (const char *fname);
fn_t getbind (unsigned keycode);
fn_t fncmatch (char *);
char *transbind (char *skey);

#endif
