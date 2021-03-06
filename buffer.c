/* buffer.c -- implements buffer.h */
#include "buffer.h"

/*  buffer.c
 *
 *  Buffer management.
 *  Some of the functions are internal,
 *  and some are actually attached to user
 *  keys. Like everyone else, they set hints
 *  for the display system
 *
 *  modified by Petri Kutvonen
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "defines.h"
#include "estruct.h"
#include "file.h"
#include "input.h"
#include "mlout.h"
#include "utf8.h"
#include "util.h"
#include "window.h"

buffer_p curbp;  /* Current buffer.  */
buffer_p bheadp; /* Head of list of buffers.  */
buffer_p blistp; /* Buffer for "C-X C-B".  */
buffer_p bscratchp; /* *scratch* */

/* Name of modes.  */
const char *modename[] =
{
  "Wrap",  "C-mode", "Exact", "View", "Over",
  "Magic", "Asave",  "UTF-8", "DOS"
};

int gmode = 0; /* global editor mode           */

static int makelist (bool iflag);
static int addline (char *text);
static void l2a (char *buf, int width, long num);

/*
 * Attach a buffer to a window. The
 * values of dot and mark come from the buffer
 * if the use count is 0. Otherwise, they come
 * from some other window.
 */
int
usebuffer (bool f, int n)
{
  buffer_p bp;
  int status;
  char *bufn;

  /* Get buffer name.  */
  status = newmlarg (&bufn, "Use buffer: ", sizeof (bname_t));
  if (status != SUCCESS)
    return status;

  /* Find buffer in list.  */
  if ((bp = bfind (bufn, 0)) == NULL)
    {
      mloutfmt ("Cannot find buffer: %s", bufn);
      free (bufn);
      return FAILURE;
    }
  free (bufn);

  /* Switch to buffer.  */
  return swbuffer (bp);
}

/*
 * switch to the next buffer in the buffer list
 *
 * int f, n;    default flag, numeric argument
 */
int
nextbuffer (bool f, int n)
{
  buffer_p bp = NULL; /* eligable buffer to switch to */
  buffer_p bbp;       /* eligable buffer to switch to */

  /* make sure the arg is legit */
  if (!f)
    n = 1;
  if (n < 1)
    return FAILURE;

  bbp = curbp;
  while (n-- > 0)
    {
      /* advance to the next buffer */
      bp = bbp->b_bufp;

      /* cycle through the buffers to find an eligable one */
      while (bp == NULL || bp->b_flag & BFINVS)
        {
          if (bp == NULL)
            bp = bheadp;
          else
            bp = bp->b_bufp;

          /* don't get caught in an infinite loop! */
          if (bp == bbp)
            return FAILURE;
        }

      bbp = bp;
    }

  return swbuffer (bp);
}

/*
 * make buffer BP current
 */
int
swbuffer (buffer_p bp)
{
  window_p wp;

  if (--curbp->b_nwnd == 0)
    { /* Last use.            */
      curbp->b_dotp = curwp->w_dotp;
      curbp->b_doto = curwp->w_doto;
      curbp->b_markp = curwp->w_markp;
      curbp->b_marko = curwp->w_marko;
    }
  curbp = bp; /* Switch.              */
  if (!curbp->b_active)
    {
      /* Buffer not active yet.  */
      /* read it in and activate it.  */
      readin (curbp->b_fname, TRUE);
      curbp->b_dotp = lforw (curbp->b_linep);
      curbp->b_doto = 0;
      curbp->b_active = TRUE;
      curbp->b_mode |= gmode; /* P.K. */
    }
  curwp->w_bufp = bp;
  curwp->w_linep = bp->b_linep;               /* For macros, ignored. */
  curwp->w_flag |= WFMODE | WFFORCE | WFHARD; /* Quite nasty.         */
  if (bp->b_nwnd++ == 0)
    { /* First use.           */
      curwp->w_dotp = bp->b_dotp;
      curwp->w_doto = bp->b_doto;
      curwp->w_markp = bp->b_markp;
      curwp->w_marko = bp->b_marko;
      cknewwindow ();
      return SUCCESS;
    }
  wp = wheadp; /* Look for old.        */
  while (wp != NULL)
    {
      if (wp != curwp && wp->w_bufp == bp)
        {
          curwp->w_dotp = wp->w_dotp;
          curwp->w_doto = wp->w_doto;
          curwp->w_markp = wp->w_markp;
          curwp->w_marko = wp->w_marko;
          break;
        }
      wp = wp->w_wndp;
    }
  cknewwindow ();
  return SUCCESS;
}

/*
 * Dispose of a buffer, by name.
 * Ask for the name. Look it up (don't get too
 * upset if it isn't there at all!). Get quite upset
 * if the buffer is being displayed. Clear the buffer (ask
 * if the buffer has been changed). Then free the header
 * line and the buffer header. Bound to "C-X K".
 */
int
killbuffer (bool f, int n)
{
  buffer_p bp;
  int status;
  char *bufn;

  /* Get buffer name */
  status = newmlarg (&bufn, "Kill buffer: ", sizeof (bname_t));
  if (status != SUCCESS)
    return status;

  /* Find buffer in list */
  bp = bfind (bufn, 0);
  free (bufn);
  if (bp == NULL) /* Easy if unknown. */
    return SUCCESS;

  if (bp->b_flag & BFINVS) /* Deal with special buffers */
    return SUCCESS;        /* by doing nothing.     */

  return zotbuf (bp);
}

/*
 * kill the buffer pointed to by bp
 */
int
zotbuf (buffer_p bp)
{
  buffer_p bp1;
  buffer_p bp2;
  int status;

  if (bp->b_nwnd != 0)
    { /* Error if on screen.  */
      mloutstr ("Buffer is being displayed");
      return FAILURE;
    }
  if ((status = bclear (bp)) != SUCCESS) /* Blow text away.  */
    return status;
  free (bp->b_linep); /* Release header line.  */
  bp1 = NULL; /* Find the header.  */
  bp2 = bheadp;
  while (bp2 != bp)
    {
      bp1 = bp2;
      bp2 = bp2->b_bufp;
    }
  bp2 = bp2->b_bufp; /* Next one in chain.   */
  if (bp1 == NULL)   /* Unlink it.           */
    bheadp = bp2;
  else
    bp1->b_bufp = bp2;
  free (bp); /* Release buffer block */
  return SUCCESS;
}

/*
 * Rename the current buffer
 *
 * int f, n;    default Flag & Numeric arg
 */
int
namebuffer (bool f, int n)
{
  buffer_p bp; /* pointer to scan through all buffers */
  int status;
  char *bufn; /* buffer to hold buffer name */

  /* Prompt for and get the new buffer name.  */
ask:
  status = newmlarg (&bufn, "Change buffer name to: ", sizeof (bname_t));
  if (status != SUCCESS)
    return status;

  /* and check for duplicates */
  bp = bheadp;
  while (bp != NULL)
    {
      if (bp != curbp)
        {
          /* retry if the names are the same */
          if (strcmp (bufn, bp->b_bname) == 0)
            {
              free (bufn);
              goto ask; /* try again */
            }
        }

      bp = bp->b_bufp; /* onward */
    }

  /* copy buffer name to structure */
  strscpy (curbp->b_bname, bufn, sizeof (bname_t));
  free (bufn);

  curwp->w_flag |= WFMODE; /* make mode line replot */
  mloutstr ("");           /* erase message line */
  return TRUE;
}

/*
 * List all of the active buffers.  First update the special
 * buffer that holds the list.  Next make sure at least 1
 * window is displaying the buffer list, splitting the screen
 * if this is what it takes.  Lastly, repaint all of the
 * windows that are displaying the list.  Bound to "C-X C-B".
 *
 * A numeric argument forces it to list invisible buffers as
 * well.
 */
int
listbuffers (bool f, int n)
{
  window_p wp;
  int status;

  if ((status = makelist (f)) != SUCCESS)
    return status;
  if (blistp->b_nwnd == 0)
    { /* Not on screen yet.   */
      buffer_p bp;

      if ((wp = wpopup ()) == NULL)
        return FALSE;
      bp = wp->w_bufp;
      if (--bp->b_nwnd == 0)
        {
          bp->b_dotp = wp->w_dotp;
          bp->b_doto = wp->w_doto;
          bp->b_markp = wp->w_markp;
          bp->b_marko = wp->w_marko;
        }
      wp->w_bufp = blistp;
      blistp->b_nwnd++;
    }
  wp = wheadp;
  while (wp != NULL)
    {
      if (wp->w_bufp == blistp)
        {
          wp->w_linep = lforw (blistp->b_linep);
          wp->w_dotp = lforw (blistp->b_linep);
          wp->w_doto = 0;
          wp->w_markp = NULL;
          wp->w_marko = 0;
          wp->w_flag |= WFMODE | WFHARD;
        }
      wp = wp->w_wndp;
    }
  delwind (FALSE, 0);
  return SUCCESS;
}

/*
 * This routine rebuilds the
 * text in the special secret buffer
 * that holds the buffer list. It is called
 * by the list buffers command. Return TRUE
 * if everything works. Return FALSE if there
 * is an error (if there is no memory). Iflag
 * indicates wether to list hidden buffers.
 *
 * int iflag;   list hidden buffer flag
 */
/* Layout: "ACT MODES          Size Buffer          File"
            AAA MMMMMMMMMSSSSSSSSSS BBBBBBBBBBBBBBB FFF...
   FNAMSTART ---------------------------------------^
*/
#define FNAMSTART (3 + 1 + NUMMODES + 10 + 1 + (sizeof (bname_t) - 1) + 1)

static void
do_layout (char *line, int mode)
{
  int i;

  /* build line to report global mode settings */
  strcpy (line, "    WCEVOMAUD           Global Modes");

  /* output the mode codes */
  for (i = 0; i < NUMMODES; i++)
    if ((mode & (1 << i)) == 0)
      line[4 + i] = '.';
}

static unsigned int
utf8_disp_len (const char *s)
{
  unsigned int len;

  len = 0;
  while (*s != '\0')
    {
      unicode_t c;

      s += utf8_to_unicode (s, 0, 4, &c);
      len++; /* Single width unicode for now.  */
    }

  return len;
}

static int
makelist (bool iflag)
{
  buffer_p bp;
  int s;
  char line[FNAMSTART + sizeof (fname_t)];

  blistp->b_flag &= ~BFCHG; /* Do not complain! Mute bclear().  */
  if ((s = bclear (blistp)) != TRUE) /* Blow old text away.  */
    return s;

  blistp->b_fname[0] = 0; /* In case of user override.  */

  if (   addline ("ACT MODES          Size Buffer          File") == FALSE
      || addline ("--- -----          ---- ------          ----") == FALSE)
    return FALSE;

  /* Report global mode settings.  */
  do_layout (line, gmode);
  if (addline (line) == FALSE)
    return FALSE;

  /* Output the list of buffers.  */
  for (bp = bheadp; bp != NULL; bp = bp->b_bufp)
    {
      /* For all buffers.  */
      char *cp1, *cp2;
      int c;
      line_p lp;
      size_t nbytes; /* # of bytes in current buffer */
      size_t nlines; /* # of lines in current buffer */
      int len;

      /* skip invisible buffers if iflag is false */
      if (((bp->b_flag & BFINVS) != 0) && !iflag)
        continue;

      do_layout (line, bp->b_mode);
      cp1 = line; /* Start at left edge   */

      /* Output status of ACTIVE flag ('@' when the file has been read in).  */
      *cp1++ = (bp->b_active == TRUE) ? '@' : ' ';

      /* Report if the file is truncated */
      *cp1++ = ((bp->b_flag & BFTRUNC) != 0) ? '#' : ' ';

      /* Output status of changed flag ('*' when the buffer is changed).  */
      *cp1 = ((bp->b_flag & BFCHG) != 0) ? '*' : ' ';

      /* Buffer size.  */
      nbytes = 0; /* Count bytes in buf.  */
      nlines = 0;
      for (lp = lforw (bp->b_linep); lp != bp->b_linep; lp = lforw (lp))
        {
          nbytes += llength (lp) + 1;
          nlines++;
        }

      if (bp->b_mode & MDDOS)
        nbytes += nlines;

      l2a (&line[13], 10 + 1, nbytes); /* "%10d" formatted numbers */
      cp1 = &line[23];
      *cp1++ = ' ';

      /* Display buffer name.  */
      cp2 = &bp->b_bname[0];
      while ((c = *cp2++) != 0)
        *cp1++ = c;

      /* Pad with spaces to max buffer name length */
      len = sizeof (bp->b_bname);
      len -= utf8_disp_len (bp->b_bname);
      while (len-- != 0)
        *cp1++ = ' ';

      /* Display filename if any */
      if (bp->b_fname[0] != 0)
        strscpy (cp1, bp->b_fname, &line[sizeof (line)] - cp1);
      else
        *cp1 = 0; /* Terminate string */

      if (addline (line) == FALSE) /* Add to the buffer.   */
        return FAILURE;
    }

  /* Do not allow to change list of buffers.  */
  blistp->b_mode |= MDVIEW;

  return SUCCESS; /* All done.  */
}

static void
l2a (char *buf, int width, long int num)
{
  buf[--width] = '\0'; /* End of string.  */
  do
    {
      /* Conditional digits.  */
      buf[--width] = (int) (num % 10L) + '0';
      num /= 10L;
    }
  while (num != 0);
  if (num < 0)
    buf[--width] = '-';
  while (width-- > 0) /* Pad with blanks.  */
    buf[width] = ' ';
}

/*
 * The argument "text" points to
 * a string. Append this line to the
 * buffer list buffer. Handcraft the EOL
 * on the end. Return TRUE if it worked and
 * FALSE if you ran out of room.
 */
static int
addline (char *text)
{
  line_p lp;
  int i;
  int ntext;

  ntext = strlen (text);
  if ((lp = lalloc (ntext)) == NULL)
    return FALSE;
  for (i = 0; i < ntext; ++i)
    lputc (lp, i, text[i]);

  blistp->b_linep->l_bp->l_fp = lp; /* Hook onto the end.  */
  lp->l_bp = blistp->b_linep->l_bp;
  blistp->b_linep->l_bp = lp;
  lp->l_fp = blistp->b_linep;

  if (blistp->b_dotp == blistp->b_linep)
    /* If "." is at the end move it to new line.  */
    blistp->b_dotp = lp;
  return TRUE;
}

/*
 * Look through the list of
 * buffers. Return TRUE if there
 * are any changed buffers. Buffers
 * that hold magic internal stuff are
 * not considered; who cares if the
 * list of buffer names is hacked.
 * Return FALSE if no buffers
 * have been changed.
 */
int
anycb (void)
{
  buffer_p bp;

  bp = bheadp;
  while (bp != NULL)
    {
      if ((bp->b_flag & BFINVS) == 0 && (bp->b_flag & BFCHG) != 0)
        return TRUE;
      bp = bp->b_bufp;
    }
  return FALSE;
}

/*
 * Create the buffer is not found
 * The "bflag" is the settings for the flags in in buffer.
 */
buffer_p
bcreate (const char *bname, unsigned int bflag)
{
  buffer_p bp;
  line_p lp;

  if ((bp = malloc (sizeof (*bp))) == NULL)
    return NULL;
  if ((lp = lalloc (0)) == NULL)
    {
      free (bp);
      return NULL;
    }
  /* Find the place in the list to insert this buffer.  */
  if (bheadp == NULL || strcmp (bheadp->b_bname, bname) > 0)
    {
      /* Insert at the beginning.  */
      bp->b_bufp = bheadp;
      bheadp = bp;
    }
  else
    {
      buffer_p sb; /* Buffer to insert after.  */

      for (sb = bheadp; sb->b_bufp != NULL; sb = sb->b_bufp)
        if (strcmp (sb->b_bufp->b_bname, bname) > 0)
          break;

      /* Insert it.  */
      bp->b_bufp = sb->b_bufp;
      sb->b_bufp = bp;
    }

  /* Set up the other buffer fields.  */
  bp->b_active = TRUE;
  bp->b_dotp = lp;
  bp->b_doto = 0;
  bp->b_markp = NULL;
  bp->b_marko = 0;
  bp->b_flag = bflag;
  bp->b_mode = gmode;
  bp->b_nwnd = 0;
  bp->b_linep = lp;
  bp->b_fname[0] = '\0';
  strscpy (bp->b_bname, bname, sizeof (bname_t));

  lp->l_fp = lp;
  lp->l_bp = lp;

  mloutstr ("New buffer");

  return bp;
}

/*
 * Find a buffer, by name.  Return a pointer
 * to the buffer structure associated with it.
 */
buffer_p
bfind (const char *bname, unsigned int bflag)
{
  buffer_p bp;

  for (bp = bheadp; bp != NULL; bp = bp->b_bufp)
    if (strcmp (bname, bp->b_bname) == 0)
      return bp;

  return bp;
}

/*
 * This routine blows away all of the text
 * in a buffer. If the buffer is marked as changed
 * then we ask if it is ok to blow it away; this is
 * to save the user the grief of losing text. The
 * window chain is nearly always wrong if this gets
 * called; the caller must arrange for the updates
 * that are required. Return TRUE if everything
 * looks good.
 */
int
bclear (buffer_p bp)
{
  line_p lp;
  int status;

  if ((bp->b_flag & BFINVS) == 0 /* Not scratch buffer.  */
      && (bp->b_flag & BFCHG) != 0 /* Something changed.  */
      && (status = mlyesno ("Discard changes?")) != SUCCESS)
    return status;
  bp->b_flag &= ~BFCHG; /* Not changed.  */
  while ((lp = lforw (bp->b_linep)) != bp->b_linep)
    lfree (lp);
  bp->b_dotp = bp->b_linep; /* Fix ".".  */
  bp->b_doto = 0;
  bp->b_markp = NULL; /* Invalidate "mark".  */
  bp->b_marko = 0;
  return SUCCESS;
}

/*
 * unmark the current buffers change flag
 *
 * int f, n;    unused command arguments
 */
int
unmark (bool f, int n)
{
  curbp->b_flag &= ~BFCHG;
  curwp->w_flag |= WFMODE;
  return SUCCESS;
}
