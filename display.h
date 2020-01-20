#ifndef _DISPLAY_H
#define _DISPLAY_H 1

#include "defines.h"

#include <stdarg.h>
#include <stddef.h>

#include "estruct.h"
#include "utf8.h"

extern size_t scrollcount; /* Number of lines to scroll.  */
extern bool mpresf;        /* Stuff in message line.  */
extern bool discmd;        /* Display command flag.  */
extern bool disinp;        /* Display input characters (echo).  */
extern int gfcolor;        /* Global forgrnd color (white).  */
extern int gbcolor;        /* Global backgrnd color (black).  */

void vtinit (void);
void vtfree (void);
void vttidy (void);
void vtmove (int row, int col);
int upscreen (bool f, int n);
int update (int force);
void updpos (void);
void upddex (void);
void updgar (void);
int updupd (bool force);
void upmode (void);
void movecursor (int row, int col);
void mlerase (void);
void vmlwrite (const char *fmt, va_list ap);
void mlwrite (const char *fmt, ...);
void ostring (char *s);
void echoc (unicode_t c);
void echos (char *s);
void rubout (void);
void getscreensize (int *widthp, int *heightp);

#if UNIX
# include <signal.h>
# ifdef SIGWINCH
extern int chg_width, chg_height;

void sizesignal (int signr);
# endif
#endif

#endif
