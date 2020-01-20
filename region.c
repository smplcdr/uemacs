/* region.c -- implements region.h */
#include "region.h"

/*  region.c
 *
 *      The routines in this file deal with the region, that magic space
 *      between "." and mark. Some functions are commands. Some functions are
 *      just for internal use.
 *
 *  Modified by Petri Kutvonen
 */

#include <ctype.h>
#include <stdio.h>

#include "buffer.h"
#include "estruct.h"
#include "line.h"
#include "mlout.h"
#include "random.h"
#include "window.h"

/*
 * Kill the region. Ask "getregion"
 * to figure out the bounds of the region.
 * Move "." to the start, and kill the characters.
 * Bound to "C-W".
 */
int
killregion (bool f, int n)
{
  int status;
  struct region region;

  if (curbp->b_mode & MDVIEW)
    /* Do not allow this command if we are in read only mode.  */
    return rdonly ();
  if ((status = getregion (&region)) != SUCCESS)
    return status;
  if ((lastflag & CFKILL) == 0) /* This is a kill type  */
    kdelete ();                 /* command, so do magic */
  thisflag |= CFKILL;           /* kill buffer stuff.   */
  curwp->w_dotp = region.r_linep;
  curwp->w_doto = region.r_offset;
  return ldelete (region.r_size, TRUE);
}

/*
 * Copy all of the characters in the
 * region to the kill buffer. Don't move dot
 * at all. This is a bit like a kill region followed
 * by a yank. Bound to "M-W".
 */
int
copyregion (bool f, int n)
{
  line_p linep;
  int loffs;
  int status;
  struct region region;

  if ((status = getregion (&region)) != SUCCESS)
    return status;
  if ((lastflag & CFKILL) == 0) /* Kill type command.   */
    kdelete ();
  thisflag |= CFKILL;
  linep = region.r_linep;  /* Current line.  */
  loffs = region.r_offset; /* Current offset.  */
  while (region.r_size--)
    {
      if (loffs == llength (linep))
        {
          /* End of line.  */
          if ((status = kinsert ('\n')) != SUCCESS)
            return status;
          linep = lforw (linep);
          loffs = 0;
        }
      else
        {
          /* Middle of line.  */
          if ((status = kinsert (lgetc (linep, loffs))) != SUCCESS)
            return status;
          loffs++;
        }
    }
  mloutstr ("(region copied)");
  return SUCCESS;
}

/*
 * Lower case region. Zap all of the upper
 * case characters in the region to lower case. Use
 * the region code to set the limits. Scan the buffer,
 * doing the changes. Call "lchange" to ensure that
 * redisplay is done in all buffers. Bound to
 * "C-X C-L".
 */
int
lowerregion (bool f, int n)
{
  line_p linep;
  int loffs;
  int c;
  int status;
  struct region region;

  if (curbp->b_mode & MDVIEW)
    /* Do not allow this command if we are in read only mode.  */
    return rdonly ();
  if ((status = getregion (&region)) != TRUE)
    return status;
  lchange (WFHARD);
  linep = region.r_linep;
  loffs = region.r_offset;
  while (region.r_size--)
    {
      if (loffs == llength (linep))
        {
          linep = lforw (linep);
          loffs = 0;
        }
      else
        {
          /* TODO */
#if 0
          unicode_t uc;
          unsigned int width;

          width = utf8_to_unicode (linep->l_text, loffs, llength (linep), &uc);

          if (isupper (uc))
            uc = tolower (uc);
          unicode_to_utf8 (uc, linep->l_text + loffs);

          loffs += width;
#endif

          c = lgetc (linep, loffs);
          if (c >= 'A' && c <= 'Z')
            lputc (linep, loffs, c + 'a' - 'A');
          loffs++;
        }
    }
  return SUCCESS;
}

/*
 * Upper case region. Zap all of the lower
 * case characters in the region to upper case. Use
 * the region code to set the limits. Scan the buffer,
 * doing the changes. Call "lchange" to ensure that
 * redisplay is done in all buffers. Bound to
 * "C-X C-L".
 */
int
upperregion (bool f, int n)
{
  line_p linep;
  int loffs;
  int c;
  int status;
  struct region region;

  if (curbp->b_mode & MDVIEW)
    /* Do not allow this command if we are in read only mode.  */
    return rdonly ();
  if ((status = getregion (&region)) != TRUE)
    return status;
  lchange (WFHARD);
  linep = region.r_linep;
  loffs = region.r_offset;
  while (region.r_size--)
    {
      if (loffs == llength (linep))
        {
          linep = lforw (linep);
          loffs = 0;
        }
      else
        {
          c = lgetc (linep, loffs);
          if (c >= 'a' && c <= 'z')
            lputc (linep, loffs, c - 'a' + 'A');
          loffs++;
        }
    }
  return TRUE;
}

/*
 * This routine figures out the
 * bounds of the region in the current window, and
 * fills in the fields of the "struct region" structure pointed
 * to by "rp". Because the dot and mark are usually very
 * close together, we scan outward from dot looking for
 * mark. This should save time. Return a standard code.
 * Callers of this routine should be prepared to get
 * an "ABORT" status; we might make this have the
 * conform thing later.
 */
int
getregion (struct region *rp)
{
  line_p flp;
  line_p blp;
  int fsize;
  int bsize;

  if (curwp->w_markp == NULL)
    {
      mloutstr ("No mark set in this window");
      return FAILURE;
    }
  if (curwp->w_dotp == curwp->w_markp)
    {
      rp->r_linep = curwp->w_dotp;
      if (curwp->w_doto < curwp->w_marko)
        {
          rp->r_offset = curwp->w_doto;
          rp->r_size = curwp->w_marko - curwp->w_doto;
        }
      else
        {
          rp->r_offset = curwp->w_marko;
          rp->r_size = curwp->w_doto - curwp->w_marko;
        }
      return SUCCESS;
    }
  blp = curwp->w_dotp;
  bsize = curwp->w_doto;
  flp = curwp->w_dotp;
  fsize = llength (flp) - curwp->w_doto + 1;
  while (flp != curbp->b_linep || lback (blp) != curbp->b_linep)
    {
      if (flp != curbp->b_linep)
        {
          flp = lforw (flp);
          if (flp == curwp->w_markp)
            {
              rp->r_linep = curwp->w_dotp;
              rp->r_offset = curwp->w_doto;
              rp->r_size = fsize + curwp->w_marko;
              return SUCCESS;
            }
          fsize += llength (flp) + 1;
        }
      if (lback (blp) != curbp->b_linep)
        {
          blp = lback (blp);
          bsize += llength (blp) + 1;
          if (blp == curwp->w_markp)
            {
              rp->r_linep = blp;
              rp->r_offset = curwp->w_marko;
              rp->r_size = bsize - curwp->w_marko;
              return SUCCESS;
            }
        }
    }
  mloutstr ("Bug: lost mark");
  return FAILURE;
}
