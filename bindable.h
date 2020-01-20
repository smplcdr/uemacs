#ifndef _BINDABLE_H
#define _BINDABLE_H 1

#include "defines.h"

/* Functions that can be bound to keys or procedure names.  */
int quickexit (bool f, int n);
int quit (bool f, int n);
int ctlxlp (bool f, int n);
int ctlxrp (bool f, int n);
int ctlxe (bool f, int n);
int ctrlg (bool f, int n);
int nullproc (bool f, int n);
int metafn (bool f, int n);
int cex (bool f, int n);
int unarg (bool f, int n);

#endif
