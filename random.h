#ifndef _RANDOM_H_
#define _RANDOM_H_

#include "defines.h"

#define AEDIT 1

extern int fillcol; /* Fill column.  */
extern int hardtab; /* Use hard tab instead of soft tab.  */

/* Uninitialized global external declarations.  */

#define CFCPCN 0x0001 /* Last command was "C-P", "C-N".  */
#define CFKILL 0x0002 /* Last command was a kill.  */

extern int thisflag; /* Flags, this command.  */
extern int lastflag; /* Flags, last command.  */

int setfillcol (bool f, int n);
int showcpos (bool f, int n);
int getcline (void);
int getccol (int bflg);
int setccol (int pos);
int twiddle (bool f, int n);
int quote (bool f, int n);
int insert_tab (bool f, int n);
#if AEDIT
int detab (bool f, int n);
int entab (bool f, int n);
int trim (bool f, int n);
#endif
int openline (bool f, int n);
int insert_newline (bool f, int n);
int deblank (bool f, int n);
int indent (bool f, int n);
int forwdel (bool f, int n);
int backdel (bool f, int n);
int killtext (bool f, int n);
int setemode (bool f, int n);
int delmode (bool f, int n);
int setgmode (bool f, int n);
int delgmode (bool f, int n);
int getfence (bool f, int n);
int istring (bool f, int n);
int ovstring (bool f, int n);

#endif
