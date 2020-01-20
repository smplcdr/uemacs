#ifndef __TERMINAL_H__
#define __TERMINAL_H__

#include "estruct.h"
#include "defines.h" /* COLOR, SCROLLCODE */
#include "retcode.h"
#include "utf8.h"

/*
 * The editor communicates with the display using a high level interface. A
 * "TERM" structure holds useful variables, and indirect pointers to routines
 * that do useful operations. The low level get and put routines are here too.
 * This lets a terminal, in addition to having non standard commands, have
 * funny get and put character code too. The calls might get changed to
 * "termp->t_field" style in the future, to make it possible to run more than
 * one terminal type.
 */
#define short int
struct terminal
{
  const short t_maxrow;         /* max number of rows allowable     */
  const short t_maxcol;         /* max number of columns allowable    */
  short t_mrow;                 /* max number of rows displayable   */
  short t_nrow;                 /* current number of rows displayed   */
  short t_mcol;                 /* max number of rows displayable   */
  short t_ncol;                 /* current number of columns displayed  */
  short t_margin;               /* min margin for extended lines */
  short t_scrsiz;               /* size of scroll region "      */
  int t_pause;                  /* # times thru update to pause */
  void (*t_open) (void);        /* Open terminal at the start.  */
  void (*t_close) (void);       /* Close terminal at end.       */
  void (*t_kopen) (void);       /* Open keyboard                */
  void (*t_kclose) (void);      /* close keyboard               */
  int (*t_getchar) (void);      /* Get character from keyboard. */
  int (*t_putchar) (unicode_t); /* Put character to display.    */
  void (*t_flush) (void);       /* Flush output buffers.        */
  void (*t_move) (int, int);    /* Move the cursor, origin 0.   */
  void (*t_eeol) (void);        /* Erase to end of line.        */
  void (*t_eeop) (void);        /* Erase to end of page.        */
  void (*t_beep) (void);        /* Beep.                        */
  void (*t_rev) (int);          /* set reverse video state      */
  int (*t_rez) (char *);        /* change screen resolution     */
#if COLOR
  int (*t_setfor) ();  /* set forground color          */
  int (*t_setback) (); /* set background color         */
#endif
#if SCROLLCODE
  void (*t_scroll) (int, int, int); /* scroll a region of the screen */
#endif
};
#undef short

/* TEMPORARY macros for terminal I/O
   (to be placed in a machine dependant place later).  */
#define TTopen (*term.t_open)
#define TTclose (*term.t_close)
#define TTkopen (*term.t_kopen)
#define TTkclose (*term.t_kclose)
#define TTgetc (*term.t_getchar)
#define TTputc (*term.t_putchar)
#define TTflush (*term.t_flush)
#define TTmove (*term.t_move)
#define TTeeol (*term.t_eeol)
#define TTeeop (*term.t_eeop)
#define TTbeep (*term.t_beep)
#define TTrev (*term.t_rev)
#define TTrez (*term.t_rez)
#if COLOR
# define TTforg (*term.t_setfor)
# define TTbacg (*term.t_setback)
#endif
#if SCROLLCODE
# define TTscroll (*term.t_scroll)
#endif

/* Terminal table defined only in term.c */
extern struct terminal term;

extern int ttrow; /* Row location of HW cursor */
extern int ttcol; /* Column location of HW cursor */

extern int eolexist; /* Does clear to EOL exist? */
extern int revexist; /* Does reverse video exist? */
extern int sgarbf;   /* State of screen unknown.  */

extern char sres[]; /* Current screen resolution: NORMAL, CGA, EGA, VGA */

#endif
