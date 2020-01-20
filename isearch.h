#ifndef _ISEARCH_H
#define _ISEARCH_H 1

#include "defines.h"

#define ISRCH 1 /* Incremental searches like ITS EMACS */

#if ISRCH
int risearch (bool f, int n);
int fisearch (bool f, int n);
#endif

#endif
