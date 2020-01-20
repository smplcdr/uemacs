
#ifndef _EBIND_H
#define _EBIND_H 1

#include "defines.h"

/* Structure for the table of initial key bindings.  */
struct key_tab
{
  unsigned int k_code;         /* Key code.  */
  int (*k_fp) (bool f, int n); /* Routine to handle it.  */
};

#define NBINDS 256                    /* max # of bound keys          */
extern struct key_tab keytab[NBINDS]; /* key bind to functions table  */

#endif
