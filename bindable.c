/* bindable.h -- implements bindable.c */
#include "bindable.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "buffer.h"
#include "defines.h"
#include "display.h"
#include "estruct.h"
#include "file.h"
#include "input.h"
#include "lock.h"
#include "mlout.h"
#include "terminal.h"

/*
 * Fancy quit command, as implemented by Norm. If the any buffer has
 * changed do a write on that buffer and exit emacs, otherwise simply exit.
 */
int
quickexit (bool f, int n)
{
  buffer_p bp;    /* scanning pointer to buffers */
  buffer_p oldcb; /* original current buffer */
  int status;

  oldcb = curbp; /* save in case we fail */

  bp = bheadp;
  while (bp != NULL)
    {
      if ((bp->b_flag & BFCHG) != 0      /* Changed.             */
          && (bp->b_flag & BFTRUNC) == 0 /* Not truncated P.K.   */
          && (bp->b_flag & BFINVS) == 0)
        { /* Real.  */
          curbp = bp; /* Make that buffer current.  */
          mloutfmt ("(Saving %s)", bp->b_fname);
#if !PKCODE
          mloutstr ("\n");
#endif
          if ((status = filesave (f, n)) != SUCCESS)
            {
              curbp = oldcb; /* restore curbp */
              return status;
            }
        }
      bp = bp->b_bufp; /* on to the next buffer */
    }
  quit (f, n); /* conditionally quit   */
  return SUCCESS;
}

static int
savebuffersonexit (bool f, int n)
{
  int c; /* Input character.  */
  buffer_p oldcb;

  oldcb = curbp;
  for (curbp = bheadp; curbp != NULL; curbp = curbp->b_bufp)
    {
      if (curbp != bscratchp && (curbp->b_flag & BFINVS) == 0 && (curbp->b_flag & BFCHG) != 0)
        {
        lagain:
          /* Prompt the user.  */
          mlwrite ("Save file %s? (y, n, .) ", curbp->b_fname);

          /* Get the response.  */
          c = get1key ();

          if (c == abortc)
            return ABORT;
          switch (c)
            {
            case 'Y':
            case 'y':
              filesave (f, n);
              break;
            case 'N':
            case 'n':
              return FAILURE;
            case '.':
              filesave (f, n);
              return SUCCESS;
            default:
              mlwrite ("Please, type 'y', 'n' or '.'");
              sleep (1);
              goto lagain;
            }
        }
    }
  curbp = oldcb;
  return TRUE;
}

/*
 * Quit command. If an argument, always quit. Otherwise confirm if a buffer
 * has been changed and not written out. Normally bound to "C-X C-C".
 */
int
quit (bool f, int n)
{
  int s = FALSE;

  if (f /* Argument forces it.  */
      || (s = savebuffersonexit (f, n)) == SUCCESS
      || anycb () == FAILURE /* All buffers clean.  */
      /* User says it's OK.  */
      || (s = mlyesno ("Modified buffers exist.  Leave anyway?")) == SUCCESS)
    {
#if (FILOCK && BSD) || SVR4
      if (lockrel () != TRUE)
        {
          TTputc ('\n');
          TTputc ('\r');
          TTclose ();
          TTkclose ();
          exit (EXIT_FAILURE);
        }
#endif
      vttidy ();
      if (f)
        exit (n);
      else
        exit (EXIT_SUCCESS);
    }
  mloutstr ("");
  return s;
}

/*
 * Begin a keyboard macro.
 * Error if not at the top level in keyboard processing. Set up variables and
 * return.
 */
int
ctlxlp (bool f, int n)
{
  if (kbdmode != STOP)
    {
      mloutstr ("%Macro already active");
      return FALSE;
    }
  mloutstr ("(Start macro)");
  kbdptr = &kbdm[0];
  kbdend = kbdptr;
  kbdmode = RECORD;
  return TRUE;
}

/*
 * End keyboard macro. Check for the same limit conditions as the above
 * routine. Set up the variables and return to the caller.
 */
int
ctlxrp (bool f, int n)
{
  if (kbdmode == STOP)
    {
      mloutstr ("%Macro not active");
      return FALSE;
    }
  if (kbdmode == RECORD)
    {
      mloutstr ("(End macro)");
      kbdmode = STOP;
    }
  return TRUE;
}

/*
 * Execute a macro.
 * The command argument is the number of times to loop. Quit as soon as a
 * command gets an error. Return TRUE if all ok, else FALSE.
 */
int
ctlxe (bool f, int n)
{
  if (kbdmode != STOP)
    {
      mloutstr ("%Macro already active");
      return FALSE;
    }
  if (n <= 0)
    return TRUE;
  kbdrep = n;        /* remember how many times to execute */
  kbdmode = PLAY;    /* start us in play mode */
  kbdptr = &kbdm[0]; /*    at the beginning */
  return TRUE;
}

/*
 * Abort.
 * Beep the beeper. Kill off any keyboard macro, etc., that is in progress.
 * Sometimes called as a routine, to do general aborting of stuff.
 */
int
ctrlg (bool f, int n)
{
  kbdmode = STOP;
  mloutfmt ("%B(Aborted)");
  return ABORT;
}

/* User function that does NOTHING.  */
int
nullproc (bool f, int n)
{
  return TRUE;
}

/* Dummy function for binding to meta prefix.  */
int
metafn (bool f, int n)
{
  return TRUE;
}

/* Dummy function for binding to C-x prefix.  */
int
cex (bool f, int n)
{
  return TRUE;
}

/* Dummy function for binding to universal-argument.  */
int
unarg (bool f, int n)
{
  return TRUE;
}
