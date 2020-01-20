/* basic.c -- implements basic.h */

#include "basic.h"

/*  basic.c
 *
 * The routines in this file move the cursor around on the screen. They
 * compute a new value for the cursor, then adjust ".". The display code
 * always updates the cursor location, so only moves between lines, or
 * functions that adjust the top line in the window and invalidate the
 * framing, are hard.
 *
 *  modified by Petri Kutvonen
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "input.h"
#include "mlout.h"
#include "random.h"
#include "terminal.h"
#include "window.h"

#define CVMVAS 1 /* Arguments to page forward/back in pages.  */

int overlap = DEFAULT_OVERLAP;
int curgoal;

/*
 * This routine, given a pointer to a struct line, and the current cursor goal
 * column, return the best choice for the offset.  The offset is returned.
 * Used by "C-N" and "C-P".
 */
static int
getgoal (line_p dlp)
{
  int col;
  int idx;
  const int len = llength (dlp);

  col = 0;
  idx = 0;
  while (idx < len)
    {
      unicode_t c;
      unsigned int width = utf8_to_unicode (dlp->l_text, idx, len, &c);

      /* Take tabs, ^X and \xx hex characters into account */
      if (c == '\t')
        col += tabwidth - col % tabwidth;
      else if (c < 0x20 || c == 0x7F)
        col += 2;
      else if (c >= 0x80 && c <= 0xA0)
        col += 3;
      else
        col += 1;

      if (col > curgoal)
        break;

      idx += width;
    }

  return idx;
}

int
gotobol (bool f, int n)
{
  curwp->w_doto = 0;
  return SUCCESS;
}
int
gotoeol (bool f, int n)
{
  curwp->w_doto = llength (curwp->w_dotp);
  return SUCCESS;
}

int
gotobob (bool f, int n)
{
  curwp->w_dotp = lforw (curbp->b_linep);
  curwp->w_doto = 0;
  curwp->w_flag |= WFHARD;
  return SUCCESS;
}
int
gotoeob (bool f, int n)
{
  curwp->w_dotp = curbp->b_linep;
  curwp->w_doto = 0;
  curwp->w_flag |= WFHARD;
  return SUCCESS;
}

int
gotoline (bool f, int n)
{
  /* Get an argument if one doesnt exist.  */
  if (!f)
    {
      int status;
      char *arg; /* Buffer to hold argument.  */

      status = newmlarg (&arg, "Line to GOTO: ", 0);
      if (status != SUCCESS)
        {
          mloutstr ("(Aborted)");
          return status;
        }

      n = atoi (arg);
      free (arg);
    }

  /*
   * Handle the case where the user may be passed something like this:
   *   em filename +
   * In this case we just go to the end of the buffer.
   */
  if (n == 0)
    return gotoeob (f, n);

  /* If a bogus argument was passed, then returns false.  */
  if (n < 0)
    return FAILURE;

  /* First, we go to the begin of the buffer.  */
  gotobob (f, n);
  return n == 1 ? SUCCESS : forwline (f, n - 1);
}

int
forwline (bool f, int n)
{
  line_p dlp;

  if (n < 0)
    return backline (f, -n);

  /* If we are on the last line as we start -- fail the command.  */
  if (curwp->w_dotp == curbp->b_linep)
    return FAILURE;

  /* If the last command was not a line move -- reset the goal column.  */
  if ((lastflag & CFCPCN) == 0)
    curgoal = getccol (FALSE);

  /* Flag this command as a line move.  */
  thisflag |= CFCPCN;

  /* Move the point down.  */
  dlp = curwp->w_dotp;
  while (n != 0 && dlp != curbp->b_linep)
    {
      dlp = lforw (dlp);
      n--;
    }

  /* Reseting the current position.  */
  curwp->w_dotp = dlp;
  curwp->w_doto = getgoal (dlp);
  curwp->w_flag |= WFMOVE;
  return n == 0 ? SUCCESS : FAILURE;
}
int
backline (bool f, int n)
{
  line_p dlp;

  if (n < 0)
    return forwline (f, -n);

  /* If we are on the first line as we start -- fail the command.  */
  if (lback (curwp->w_dotp) == curbp->b_linep)
    return FAILURE;

  /* If the last command was not a line move -- reset the goal column.  */
  if ((lastflag & CFCPCN) == 0)
    curgoal = getccol (FALSE);

  /* Flag this command as a line move.  */
  thisflag |= CFCPCN;

  /* Move the point up.  */
  dlp = curwp->w_dotp;
  while (n != 0 && lback (dlp) != curbp->b_linep)
    {
      dlp = lback (dlp);
      n--;
    }

  /* Reseting the current position.  */
  curwp->w_dotp = dlp;
  curwp->w_doto = getgoal (dlp);
  curwp->w_flag |= WFMOVE;
  return n == 0 ? SUCCESS : FAILURE;
}

int
forwpage (bool f, int n)
{
  line_p lp;

  if (!f)
    {
#if SCROLLCODE
      if (term.t_scroll != NULL) /* $scroll == FALSE */
        if (overlap == 0)
          n = curwp->w_ntrows * 2 / 3;
        else
          n = curwp->w_ntrows - overlap;
      else
#endif
        n = curwp->w_ntrows - 2; /* Default scroll.  */

      /* Forget the overlap if tiny window.  */
      if (n <= 0)
        n = 1;
    }
  else if (n < 0)
    return backpage (f, -n);
#if CVMVAS
  /* Convert from pages to lines.  */
  else
    n *= curwp->w_ntrows;
#endif

  /* lp = curwp->w_linep; */
  lp = curwp->w_dotp;
  while (n != 0 && lp != curbp->b_linep)
    {
      lp = lforw (lp);
      n--;
    }

  /* curwp->w_linep = lp; */
  curwp->w_dotp = lp;
  curwp->w_doto = 0;
  reposition (TRUE, 0);

#if SCROLLCODE
  curwp->w_flag |= WFHARD | WFKILLS;
#else
  curwp->w_flag |= WFHARD;
#endif
  return FAILURE;
}

int
backpage (bool f, int n)
{
  line_p lp;

  if (!f)
    {
      /* Interactive, default n = 1 supplied.  */
      /* In interactive mode, first move dot to top of window.  */
      if (curwp->w_dotp != curwp->w_linep)
        {
          curwp->w_dotp = curwp->w_linep;
          curwp->w_doto = 0;
          /* curwp->w_flag |= WFMOVE; */
          return SUCCESS;
        }

#if SCROLLCODE
      if (term.t_scroll != NULL) /* $scroll != FALSE */
        if (overlap == 0)
          n = curwp->w_ntrows * 2 / 3;
        else
          n = curwp->w_ntrows - overlap;
      else
#endif
        n = curwp->w_ntrows - 2; /* Default scroll.  */

      /* Do not blow up if the window is tiny.  */
      if (n <= 0)
        n = 1;
    }
  else if (n < 0)
    return forwpage (f, -n);
#if CVMVAS
  /* Convert from pages to lines. */
  else
    n *= curwp->w_ntrows;
#endif

  /* lp = curwp->w_linep; */
  lp = curwp->w_dotp;
  while (n != 0 && lback (lp) != curbp->b_linep)
    {
      lp = lback (lp);
      n--;
    }

  /* curwp->w_linep = lp; */
  curwp->w_dotp = lp;
  curwp->w_doto = 0;
  reposition (TRUE, !f ? 1 : 0);

#if SCROLLCODE
  curwp->w_flag |= WFHARD | WFINS;
#else
  curwp->w_flag |= WFHARD;
#endif

  return SUCCESS;
}

int
setmark (bool f, int n)
{
  curwp->w_markp = curwp->w_dotp;
  curwp->w_marko = curwp->w_doto;
  mloutstr ("(Mark set)");

  return SUCCESS;
}
int
swapmark (bool f, int n)
{
  line_p odotp;
  int odoto;

  if (curwp->w_markp == NULL)
    {
      mloutstr ("No mark in this window");
      return FAILURE;
    }

  odotp = curwp->w_dotp;
  odoto = curwp->w_doto;
  curwp->w_dotp = curwp->w_markp;
  curwp->w_doto = curwp->w_marko;
  curwp->w_markp = odotp;
  curwp->w_marko = odoto;
  curwp->w_flag |= WFMOVE;

  return SUCCESS;
}

/* end of basic.c */
