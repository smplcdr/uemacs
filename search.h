#ifndef _SEARCH_H_
#define _SEARCH_H_

#define MAGIC 1 /* include regular expression matching? */

#include "line.h"
#include <stddef.h>

typedef char spat_t[128];    /* search pattern type */
#define NPAT sizeof (spat_t) /* # of bytes, pattern   */

extern unsigned int matchlen;
extern char *patmatch;

extern spat_t pat;  /* Search pattern               */
extern spat_t tap;  /* Reversed pattern array.      */
extern spat_t rpat; /* replacement pattern          */

/*
 * PTBEG, PTEND, FORWARD, and REVERSE are all toggle-able values for
 * the scan routines.
 */
#define PTBEG 0   /* Leave the point at the beginning on search   */
#define PTEND 1   /* Leave the point at the end on search         */
#define FORWARD 0 /* forward direction            */
#define REVERSE 1 /* backwards direction          */

int scanner (const char *patrn, int direct, int beg_or_end);

int forwsearch (bool f, int n);
int forwhunt (bool f, int n);
int backsearch (bool f, int n);
int backhunt (bool f, int n);
int eq (unsigned char bc, unsigned char pc);
void savematch (void);
void rvstrcpy (char *rvstr, char *str);
int sreplace (bool f, int n);
int qreplace (bool f, int n);
int delins (int dlength, char *instr, int use_meta);
int expandp (char *srcstr, char *deststr, int maxlength);
int boundry (line_p curline, int curoff, int dir);

void setprompt (char *tpat, unsigned tpat_size, char *prompt, char *apat);

#if MAGIC
void mcclear (void);
void rmcclear (void);
#endif

#endif
