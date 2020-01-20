#ifndef _WINDOW_H
#define _WINDOW_H 1

#include "defines.h"

#include "estruct.h"
#include "buffer.h"  /* buffer, line */

/*
 * There is a window structure allocated for every active display window. The
 * windows are kept in a big list, in top to bottom screen order, with the
 * listhead at "wheadp". Each window contains its own values of dot and mark.
 * The flag field contains some bits that are set by commands to guide
 * redisplay. Although this is a bit of a compromise in terms of decoupling,
 * the full blown redisplay is just too expensive to run for every input
 * character.
 */
typedef struct window *window_p;
struct window
{
  window_p w_wndp;        /* Next window.  */
  buffer_p w_bufp;        /* Buffer displayed in window.  */
  line_p w_linep;         /* Top line in the window.  */
  line_p w_dotp;          /* Line containing ".".  */
  line_p w_markp;         /* Line containing "mark".  */
  int w_doto;             /* Byte offset for ".".  */
  int w_marko;            /* Byte offset for "mark".  */
  int w_toprow;           /* Origin 0 top row of window.  */
  int w_ntrows;           /* # of rows of text in window.  */
  unsigned int w_force:1; /* If NZ, forcing row.  */
  unsigned char w_flag;   /* Flags.  */
#if COLOR
  char w_fcolor;          /* Current forground color.  */
  char w_bcolor;          /* Current background color.  */
#endif
};

extern window_p curwp;  /* Current window.  */
extern window_p wheadp; /* Head of list of windows.  */

/* curwbyte() return the byte after the dot in current window.  */
#define curwbyte() lgetc (curwp->w_dotp, curwp->w_doto)

#define WFFORCE (1 << 0) /* Window needs forced reframe.  */
#define WFMOVE  (1 << 1) /* Movement from line to line.  */
#define WFEDIT  (1 << 2) /* Editing within a line.  */
#define WFHARD  (1 << 3) /* Better to a full display.  */
#define WFMODE  (1 << 4) /* Update mode line.  */
#define WFCOLR  (1 << 5) /* Needs a color change.  */

#if SCROLLCODE
# define WFKILLS (1 << 6) /* Something was deleted.  */
# define WFINS   (1 << 7) /* Something was inserted.  */
#endif

extern int reposition (bool f, int n);
extern int redraw (bool f, int n);
extern int nextwind (bool f, int n);
extern int prevwind (bool f, int n);
extern int mvdnwind (bool f, int n);
extern int mvupwind (bool f, int n);
extern int onlywind (bool f, int n);
extern int delwind (bool f, int n);
extern int splitwind (bool f, int n);
extern int enlargewind (bool f, int n);
extern int shrinkwind (bool f, int n);
extern int resize (bool f, int n);
extern int scrnextup (bool f, int n);
extern int scrnextdw (bool f, int n);
extern int savewnd (bool f, int n);
extern int restwnd (bool f, int n);
extern int newsize (bool f, int n);
extern int newwidth (bool f, int n);

extern int getwpos (void);
extern void cknewwindow (void);
extern window_p wpopup (void); /* Pop up window creation.  */

#endif /* _WINDOW_H */
