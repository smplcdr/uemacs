/* main.c -- */

/*
 *  main.c
 *
 *  µEMACS 4.2
 *
 *  Based on:
 *
 *  uEmacs/PK 4.0
 *
 *  Based on:
 *
 *  MicroEMACS 3.9
 *  Written by Dave G. Conroy.
 *  Substantially modified by Daniel M. Lawrence
 *  Modified by Petri Kutvonen
 *
 *  MicroEMACS 3.9 (c) Copyright 1987 by Daniel M. Lawrence
 *
 *  Original statement of copying policy:
 *
 *  MicroEMACS 3.9 can be copied and distributed freely for any
 *  non-commercial purposes. MicroEMACS 3.9 can only be incorporated
 *  into commercial software with the permission of the current author.
 *
 *  No copyright claimed for modifications made by Petri Kutvonen.
 *
 *  This file contains the main driving routine, and some keyboard
 *  processing code.
 *
 * REVISION HISTORY:
 *
 * 1.0  Steve Wilhite, 30-Nov-85
 *
 * 2.0  George Jones, 12-Dec-85
 *
 * 3.0  Daniel Lawrence, 29-Dec-85
 *
 * 3.2-3.6 Daniel Lawrence, Feb...Apr-86
 *
 * 3.7  Daniel Lawrence, 14-May-86
 *
 * 3.8  Daniel Lawrence, 18-Jan-87
 *
 * 3.9  Daniel Lawrence, 16-Jul-87
 *
 * 3.9e Daniel Lawrence, 16-Nov-87
 *
 * After that versions 3.X and Daniel Lawrence went their own ways.
 * A modified 3.9e/PK was heavily used at the University of Helsinki
 * for several years on different UNIX, VMS, and MSDOS platforms.
 *
 * This modified version is now called eEmacs/PK.
 *
 * 4.0  Petri Kutvonen, 1-Sep-91
 *
 * This modified version is now called uEMACS.
 *
 * 4.1  Renaud Fivet, 1-May-13
 *
 * Renamed as µEMACS to emphasize UTF-8 support.
 *
 * 4.2  Renaud Fivet, 2015-02-12
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "estruct.h" /* Global structures and defines. */
#if UNIX
# include <signal.h>
#endif

#include "basic.h"
#include "bind.h"
#include "bindable.h"
#include "buffer.h"
#include "display.h"
#include "eval.h"
#include "execute.h"
#include "file.h"
#include "lock.h"
#include "mlout.h"
#include "random.h"
#include "search.h"
#include "terminal.h"
#include "termio.h"
#include "util.h"
#include "version.h"
#include "window.h"

#if UNIX
static void
emergencyexit (int signr)
{
  quickexit (FALSE, 0);
  quit (TRUE, 0); /* If quickexit fails (to save changes), do a force quit */
}
#endif

static void edinit (char *bname);

static void
version (void)
{
  fputs (PROGRAM_NAME_UTF8 " version " VERSION "\n", stdout);
}

static void
usage (void)
{
  puts ("Usage: "PROGRAM_NAME" [OPTION|FILE]..\n");
  puts ("      +             start at the end of file");
  puts ("      +<n>          start at line <n>");
  puts ("      --help        display this help and exit");
  puts ("      --version     output version information and exit");
  puts ("      -a            process error file");
  puts ("      -e            edit file");
  puts ("      -g<n>         go to line <n>");
  puts ("      -r            restrictive use");
  puts ("      -s <string>   search string");
  puts ("      -v            view file");
  puts ("      -x <cmdfile>  execute command file\n");
  puts ("      @cmdfile      execute startup file\n");
}

int
main (int argc, char **argv)
{
  buffer_p bp;                   /* Temperary buffer pointer.  */
  int firstfile;                 /* First file flag.  */
  int carg;                      /* Current arg to scan.  */
  int startflag;                 /* Startup executed flag.  */
  buffer_p firstbp = NULL;       /* Pointer to first buffer in command line.  */
  bool viewflag;                 /* Are we starting in view mode? */
  bool gotoflag;                 /* Do we need to goto a line at start? */
  int gline = 0;                 /* If so, what line? */
  int searchflag;                /* Do we need to search at start? */
  int errflag;                   /* C error processing? */
  bname_t bname;                 /* Buffer name of file to read.  */

#if PKCODE & BSD
  sleep (1); /* Time for window manager. */
#endif

#if UNIX
# ifdef SIGWINCH
  signal (SIGWINCH, sizesignal);
# endif
#endif
  if (argc == 2)
    {
      if (strcmp (argv[1], "--help") == 0)
        {
          usage ();
          exit (EXIT_SUCCESS);
        }
      if (strcmp (argv[1], "--version") == 0)
        {
          version ();
          exit (EXIT_SUCCESS);
        }
    }

  /* Initialize the editor. */
  vtinit (); /* Display */
  mloutfmt = mlwrite;
  edinit ("*scratch*"); /* Buffers, windows.  */
  varinit (); /* User variables.  */

  viewflag = FALSE;   /* View mode defaults off in command line.  */
  gotoflag = FALSE;   /* Set to off to begin with.  */
  searchflag = FALSE; /* Set to off to begin with.  */
  firstfile = TRUE;   /* No file to edit yet.  */
  startflag = FALSE;  /* Startup file not executed yet.  */
  errflag = FALSE;    /* Not doing C error parsing.  */

  /* Insure screen is initialized before startup and goto/search.  */
  update (FALSE);

  /* Parse the command line.  */
  for (carg = 1; carg < argc; carg++)
    {
      /* Process Switches.  */
#if PKCODE
      if (argv[carg][0] == '+')
        {
          gotoflag = TRUE;
          gline = atoi (&argv[carg][1]);
        }
      else
#endif
        if (argv[carg][0] == '-')
          {
            switch (argv[carg][1])
              {
                /* Process Startup macroes.  */
              case 'a': /* Process error file.  */
                errflag = TRUE;
                break;
              case 'e': /* -e for Edit file.  */
                viewflag = FALSE;
                break;
              case 'g': /* -g for initial goto.  */
                gotoflag = TRUE;
                gline = atoi (&argv[carg][2]);
                break;
              case 'r': /* -r restrictive use.  */
                restflag = TRUE;
                break;
              case 's': /* -s for initial search string.  */
                searchflag = TRUE;
                strscpy (pat, &argv[carg][2], sizeof (pat));
                break;
              case 'v': /* -v for View File.  */
                viewflag = TRUE;
                break;
              case 'x':
                if (argv[carg][2] != '\0')
                  {
                    /* -xfilename */
                    if (startup (&argv[carg][2]) == TRUE)
                      startflag = TRUE; /* Do not execute emacs.rc */
                  }
                else if (argv[carg + 1] != NULL)
                  {
                    /* -x filename */
                    if (startup (&argv[carg + 1][0]) == TRUE)
                      startflag = TRUE; /* Do not execute emacs.rc */
                    carg++;
                  }
                break;
              default: /* Unknown switch.  */
                /* Ignore this for now.  */
                break;
              }
          }
        else if (argv[carg][0] == '@')
          {
            /* Process Startup macroes.  */
            if (startup (&argv[carg][1]) == TRUE)
              /* Do not execute emacs.rc */
              startflag = TRUE;
          }
        else
          {
            /* Process an input file.  */
            /* Set up a buffer for this file.  */
            makename (bname, argv[carg]);
            unqname (bname);
            /* Set this to inactive.  */
            if ((bp = bfind (bname, 0)) == NULL && (bp = bcreate (bname, 0)) == NULL)
              {
                fputs ("Buffer creation failed!\n", stderr);
                exit (EXIT_FAILURE);
              }

            /* Max filename length limited to NFILEN - 1.  */
            strscpy (bp->b_fname, argv[carg], sizeof (bp->b_fname));

            bp->b_active = FALSE;
            if (firstfile)
              {
                firstbp = bp;
                firstfile = FALSE;
              }

            /* Set the modes appropriatly.  */
            if (viewflag)
              bp->b_mode |= MDVIEW;
          }
    }

#if UNIX
# ifdef SIGHUP
  signal (SIGHUP, emergencyexit);
# endif
  signal (SIGTERM, emergencyexit);
#endif

  /* If we are C error parsing... run it! */
  if (errflag && startup ("error.cmd") == SUCCESS)
    startflag = TRUE;

  /* If invoked with no other startup files,
     run the system startup file here.  */
  if (!startflag && startup ("") != SUCCESS)
    mloutstr ("Default startup failed!");

  discmd = TRUE; /* P.K. */

  /* if there are any files to read, read the first one! */
  if ((bp = bfind ("*scratch*", 0)) == NULL)
    {
      /* "*scratch*" buffer has been created during early initialisation */
      fputs ("Initialisation failure!\n", stderr);
      exit (EXIT_FAILURE);
    }

  if (firstfile == FALSE && readfirst_f ())
    {
      swbuffer (firstbp);
      zotbuf (bp);
    }
  else
    {
      bp->b_mode |= gmode;
      upmode ();
    }

  /* Deal with startup gotos and searches */
  if (gotoflag && searchflag)
    mloutstr ("(Can not search and goto at the same time!)");
  else if (gotoflag)
    {
      if (gotoline (TRUE, gline) == FALSE)
        mloutstr ("(Bogus goto argument)");
    }
  else if (searchflag)
    if (forwhunt (FALSE, 0))
      mloutfmt ("Found on line %d", getcline ());

  kbd_loop ();
  return EXIT_SUCCESS; /* Never reached.  */
}

/*
 * Initialize all of the buffers and windows. The buffer name is passed down
 * as an argument, because the main routine may have been told to read in a
 * file by default, and we want the buffer name to be right.
 */
static void
edinit (char *bname)
{
  buffer_p bp;
  window_p wp;

  if (NULL == (bp = bcreate (bname, 0)) /* First buffer.  */
      || NULL == (blistp = bcreate ("*List*", BFINVS)) /* Buffer list buffer.  */
      || NULL == (wp = malloc (sizeof (*wp)))) /* First window.  */
    {
      fputs ("First initialisation failed!\n", stderr);
      exit (EXIT_FAILURE);
    }

  curbp = bp; /* Make this current.  */
  bscratchp = bp;
  wheadp = wp;
  curwp = wp;
  wp->w_wndp = NULL; /* Initialize window.  */
  wp->w_bufp = bp;
  bp->b_nwnd = 1; /* Displayed.  */
  wp->w_linep = bp->b_linep;
  wp->w_dotp = bp->b_linep;
  wp->w_doto = 0;
  wp->w_markp = NULL;
  wp->w_marko = 0;
  wp->w_toprow = 0;
#if COLOR
  /* Initalize colors to global defaults.  */
  wp->w_fcolor = gfcolor;
  wp->w_bcolor = gbcolor;
#endif
  wp->w_ntrows = term.t_nrow - 1; /* "-1" for mode line.  */
  wp->w_force = 0;
  wp->w_flag = WFMODE | WFHARD; /* Full.  */
}

/***** Compiler specific Library functions ****/

#if RAMSIZE
/*  These routines will allow me to track memory usage by placing
    a layer on top of the standard system malloc() and free() calls.
    with this code defined, the environment variable, $RAM, will
    report on the number of bytes allocated via malloc.

    with SHOWRAM defined, the number is also posted on the
    end of the bottom mode line and is updated whenever it is changed.  */
static void dspram (void);

# undef malloc
# undef free
void *
allocate (size_t nbytes)
{
  char *mp;

  mp = malloc (nbytes);
  if (mp != NULL)
    {
      envram += nbytes;
# if RAMSHOW
      dspram ();
# endif
    }

  return mp;
}

void
release (void *mp)
{
  unsigned int *lp; /* Pointer to the long containing the block size.  */

  if (mp != NULL)
    {
      /* Update amount of ram currently malloced.  */
      lp = ((unsigned int *) mp) - 1;
      envram -= (long) *lp - 2;
      free (mp);
# if RAMSHOW
      dspram ();
# endif
    }
}

# if RAMSHOW
static void
dspram (void)
{ /* display the amount of RAM currently malloced */
  char mbuf[20];
  char *sp;

  TTmove (term.t_nrow - 1, 70);
#  if COLOR
  TTforg (7);
  TTbacg (0);
#  endif
  sprintf (mbuf, "[%lu]", envram);
  sp = &mbuf[0];
  while (*sp != '\0')
    TTputc (*sp++);
  TTmove (term.t_nrow, 0);
  movecursor (term.t_nrow, 0);
}
# endif
#endif

/* On some primitave operation systems, and when emacs is used as
   a subprogram to a larger project, emacs needs to dealloc its
   own used memory.  */
#if CLEAN
void
cexit (int status)
{
  buffer_p bp; /* Buffer list pointer.  */
  window_p wp; /* Window list pointer.  */

  /* Clean up the windows.  */
  wp = wheadp;
  while (wp != NULL)
    {
      window_p tmp;

      tmp = wp->w_wndp;
      free (wp);
      wp = tmp;
    }
  wheadp = NULL;

  /* Clean up the buffers.  */
  bp = bheadp;
  while (bp)
    {
      bp->b_nwnd = 0;
      bp->b_flag = 0; /* Do not say anything about a changed buffer! */
      zotbuf (bp);
      bp = bheadp;
    }

  /* Clean up the kill buffer.  */
  kdelete ();

  /* Clean up the video buffers.  */
  vtfree ();

# undef exit
  exit (status);
}
#endif
