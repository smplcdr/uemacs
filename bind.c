/* bind.c -- implements bind.h */
#include "bind.h"

/*  bind.c
 *
 *  This file is for functions having to do with key bindings,
 *  descriptions, help commands and startup file.
 *
 *  Written 11-feb-86 by Daniel Lawrence
 *  Modified by Petri Kutvonen
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bindable.h"
#include "buffer.h"
#include "display.h"
#include "ebind.h"
#include "estruct.h"
#include "exec.h"
#include "file.h"
#include "flook.h"
#include "input.h"
#include "line.h"
#include "names.h"
#include "util.h"
#include "window.h"

#if APROP
static int buildlist (const char *mstring);
static int strinc (char *source, const char *sub);
#endif

static void cmdstr (unsigned int c, char *seq);
static unsigned int getckey (int mflag);
static unsigned int stock (char *keyname);
static int unbindchar (unsigned c);
static char *getfname (fn_t);

int
help (bool f, int n)
{                     /* give me some help!!!!
                         bring up a fake buffer and read the help file
                         into it with view mode                 */
  buffer_p bp;  /* buffer pointer to help */
  char *fname = NULL; /* ptr to file returned by flook() */

  /* first check if we are already here */
  if ((bp = bfind (hlpfname, BFINVS)) == NULL)
    {
      if ((fname = flook (hlpfname, FALSE)) == NULL)
        {
          mlwrite ("(Help file is not online)");
          return FAILURE;
        }
    }

  /* split the current window to make room for the help stuff */
  if (splitwind (FALSE, 1) == FAILURE)
    return FAILURE;

  if (bp == NULL)
    {
      /* and read the stuff in */
      if (getfile (fname, FALSE) == FAILURE)
        return FAILURE;
    }
  else
    swbuffer (bp);

  /* make this window in VIEW mode, update all mode lines */
  curwp->w_bufp->b_mode |= MDVIEW;
  curwp->w_bufp->b_flag |= BFINVS;
  upmode ();
  return SUCCESS;
}

int
deskey (bool f, int n)
{                       /* describe the command for a certain key */
  int c;                /* key to describe */
  char *ptr;            /* string pointer to scan output strings */
  char outseq[NSTRING]; /* output buffer for command sequence */

  /* prompt the user to type us a key to describe */
  mlwrite (": describe-key ");

  /* get the command sequence to describe
     change it to something we can print as well */
  c = getckey (FALSE);
  mlwrite (": describe-key 0x%x, ", c);
  cmdstr (c, &outseq[0]);

  /* and dump it out */
  ostring (outseq);
  ostring (" ");

  /* find the right ->function */
  if ((ptr = getfname (getbind (c))) == NULL)
    ptr = "Not Bound";

  /* output the command sequence */
  ostring (ptr);
  return SUCCESS;
}

/*
 * bindtokey:
 *  add a new key to the key binding table
 *
 * int f, n;        command arguments [IGNORED]
 */
int
bindtokey (bool f, int n)
{
  unsigned int c;      /* command key to bind */
  fn_t kfunc;          /* ptr to the requested function to bind to */
  struct key_tab *ktp; /* pointer into the command table */
  int found;           /* matched command flag */
  char outseq[80];     /* output buffer for keystroke sequence */

  /* prompt the user to type in a key to bind */
  mlwrite (": bind-to-key ");

  /* get the function name to bind it to */
  kfunc = getname ();
  if (kfunc == NULL)
    {
      mlwrite ("(No such function)");
      return FAILURE;
    }
  ostring (" ");

  /* get the command sequence to bind */
  c = getckey ((kfunc == metafn) || (kfunc == cex)
               || (kfunc == unarg) || (kfunc == ctrlg));

  /* change it to something we can print as well */
  cmdstr (c, &outseq[0]);

  /* and dump it out */
  ostring (outseq);

  /* if the function is a prefix key */
  if (kfunc == metafn || kfunc == cex || kfunc == unarg || kfunc == ctrlg)
    {
      /* search for an existing binding for the prefix key */
      ktp = &keytab[0];
      found = FALSE;
      while (ktp->k_fp != NULL)
        {
          if (ktp->k_fp == kfunc)
            unbindchar (ktp->k_code);
          ktp++;
        }

      /* reset the appropriate global prefix variable */
      if (kfunc == metafn)
        metac = c;
      if (kfunc == cex)
        ctlxc = c;
      if (kfunc == unarg)
        reptc = c;
      if (kfunc == ctrlg)
        abortc = c;
    }

  /* search the table to see if it exists */
  ktp = &keytab[0];
  found = FALSE;
  while (ktp->k_fp != NULL)
    {
      if (ktp->k_code == c)
        {
          found = TRUE;
          break;
        }
      ktp++;
    }

  if (found)
    /* it exists, just change it then */
    ktp->k_fp = kfunc;
  else
    { /* otherwise we need to add it to the end */
      /* if we run out of binding room, bitch */
      if (ktp >= &keytab[NBINDS])
        {
          mlwrite ("Binding table FULL!");
          return FAILURE;
        }

      ktp->k_code = c;   /* add keycode */
      ktp->k_fp = kfunc; /* and the function pointer */
      ++ktp;             /* and make sure the next is null */
      ktp->k_code = 0;
      ktp->k_fp = NULL;
    }
  return TRUE;
}

/*
 * unbindkey:
 *  delete a key from the key binding table
 *
 * int f, n;        command arguments [IGNORED]
 */
int
unbindkey (bool f, int n)
{
  int c;           /* command key to unbind */
  char outseq[80]; /* output buffer for keystroke sequence */

  /* prompt the user to type in a key to unbind */
  mlwrite (": unbind-key ");

  /* get the command sequence to unbind */
  c = getckey (FALSE); /* get a command sequence */

  /* change it to something we can print as well */
  cmdstr (c, &outseq[0]);

  /* and dump it out */
  ostring (outseq);

  /* if it isn't bound, bitch */
  if (unbindchar (c) == FAILURE)
    {
      mlwrite ("(Key not bound)");
      return FAILURE;
    }
  return SUCCESS;
}

/*
 * unbindchar()
 *
 * int c;       command key to unbind
 */
static int
unbindchar (unsigned int c)
{
  struct key_tab *ktp;  /* pointer into the command table */
  struct key_tab *sktp; /* saved pointer into the command table */
  int found;            /* matched command flag */

  /* search the table to see if the key exists */
  ktp = &keytab[0];
  found = FALSE;
  while (ktp->k_fp != NULL)
    {
      if (ktp->k_code == c)
        {
          found = TRUE;
          break;
        }
      ktp++;
    }

  /* if it isn't bound, bitch */
  if (!found)
    return FAILURE;

  /* save the pointer and scan to the end of the table */
  sktp = ktp;
  while (ktp->k_fp != NULL)
    ktp++;
  ktp--; /* backup to the last legit entry */

  /* copy the last entry to the current one */
  sktp->k_code = ktp->k_code;
  sktp->k_fp = ktp->k_fp;

  /* null out the last one */
  ktp->k_code = 0;
  ktp->k_fp = NULL;
  return TRUE;
}

/* describe bindings
 * bring up a fake buffer and list the key bindings
 * into it with view mode
 */
int
desbind (bool f, int n)
{
#if APROP
  return buildlist ("");
}

/* Apropos (List functions that match a substring) */
int
apro (bool f, int n)
{
  char *mstring; /* string to match cmd names to */
  int status;    /* status return */

  status = newmlarg (&mstring, "Apropos string: ", 0);
  if (status == SUCCESS)
    {
      status = buildlist (mstring);
      free (mstring);
    }
  else if (status == FAILURE)
    status = buildlist (""); /* build list of all commands */

  return status;
}

/*
 * build a binding list (limited or full)
 *
 * char *mstring;   match string if a partial list, "" matches all
 */
static int
buildlist (const char *mstring)
{
#endif
  window_p wp;      /* scanning pointer to windows */
  struct key_tab *ktp;    /* pointer into the command table */
  struct name_bind *nptr; /* pointer into the name binding table */
  buffer_p bp;      /* buffer to put binding list into */
  char outseq[80];        /* output buffer for keystroke sequence */

  /* split the current window to make room for the binding list */
  if (splitwind (FALSE, 1) == FAILURE)
    return FAILURE;

  /* and get a buffer for it */
  if ((bp = bfind ("*Binding list*", 0)) == NULL)
    bp = bcreate ("*Binding list*", 0);
  if (bp == NULL || bclear (bp) == FAILURE)
    {
      mlwrite ("Can not display binding list");
      return FAILURE;
    }

  /* let us know this is in progress */
  mlwrite ("(Building binding list)");

  /* disconect the current buffer */
  if (--curbp->b_nwnd == 0)
    { /* Last use.            */
      curbp->b_dotp = curwp->w_dotp;
      curbp->b_doto = curwp->w_doto;
      curbp->b_markp = curwp->w_markp;
      curbp->b_marko = curwp->w_marko;
    }

  /* connect the current window to this buffer */
  curbp = bp;     /* make this buffer current in current window */
  bp->b_mode = 0; /* no modes active in binding list */
  bp->b_nwnd++;   /* mark us as more in use */
  wp = curwp;
  wp->w_bufp = bp;
  wp->w_linep = bp->b_linep;
  wp->w_flag = WFHARD | WFFORCE;
  wp->w_dotp = bp->b_dotp;
  wp->w_doto = bp->b_doto;
  wp->w_markp = NULL;
  wp->w_marko = 0;

  /* build the contents of this window, inserting it line by line */
  for (nptr = &names[0]; nptr->n_func != NULL; nptr++)
    {
      int cpos; /* current position to use in outseq */

#if APROP
      /* if we are executing an apropos command..... */
      /* and current string doesn't include the search string */
      if (*mstring && strinc (nptr->n_name, mstring) == FALSE)
        continue;
#endif
      /* add in the command name */
      strscpy (outseq, nptr->n_name, sizeof outseq);
      cpos = strlen (outseq);

      /* search down any keys bound to this */
      ktp = &keytab[0];
      while (ktp->k_fp != NULL)
        {
          if (ktp->k_fp == nptr->n_func)
            {
              /* padd out some spaces */
              while (cpos < 28)
                outseq[cpos++] = ' ';

              /* add in the command sequence */
              cmdstr (ktp->k_code, &outseq[cpos]);
              strcat (outseq, "\n");

              /* and add it as a line into the buffer */
              if (linstr (outseq) != SUCCESS)
                return FAILURE;

              cpos = 0; /* and clear the line */
            }
          ktp++;
        }

      /* if no key was bound, we need to dump it anyway */
      if (cpos > 0)
        {
          outseq[cpos++] = '\n';
          outseq[cpos] = '\0';
          if (linstr (outseq) != SUCCESS)
            return FAILURE;
        }
    }

  bp->b_mode |= MDVIEW;             /* put this buffer view mode */
  bp->b_flag &= ~BFCHG;             /* don't flag this as a change */
  wp->w_dotp = lforw (bp->b_linep); /* back to the beginning */
  wp->w_doto = 0;
  upmode ();    /* and update ALL mode lines */
  mlwrite (""); /* clear the mode line */
  return SUCCESS;
}

#if APROP

/*
 * does source include sub?
 *
 * char *source;    string to search in
 * char *sub;       substring to look for
 */
static int
strinc (char *source, const char *sub)
{
  /* for each character in the source string */
  for (; *source; source++)
    {
      char *nxtsp;    /* next ptr into source */
      const char *tp; /* ptr into substring */

      nxtsp = source;

      /* is the substring here? */
      for (tp = sub; *tp; tp++)
        if (*nxtsp++ != *tp)
          break;

      /* yes, return a success */
      if (*tp == '\0')
        return SUCCESS;
    }

  return FAILURE;
}
#endif

/*
 * get a command key sequence from the keyboard
 *
 * int mflag;       going for a meta sequence?
 */
static unsigned int
getckey (int mflag)
{
  unsigned int c; /* character fetched */

  /* check to see if we are executing a command line */
  if (clexec)
    {
      char *tok; /* command incoming */

      tok = getnewtokval (); /* get the next token */
      if (tok == NULL)
        c = 0; /* return dummy key on failure */
      else
        {
          c = stock (tok);
          free (tok);
        }
    }
  else
    { /* or the normal way */
      if (mflag)
        c = get1key ();
      else
        c = getcmd ();
    }

  return c;
}

/*
 * execute the startup file
 *
 * char *fname;    name of startup file (null if default)
 */
int
startup (const char *fname)
{
  if (!fname || *fname == '\0') /* use default if empty parameter */
    fname = rcfname;

  fname = flook (fname, TRUE); /* look up the startup file */
  if (fname == NULL)           /* if it isn't around, don't sweat it */
    return SUCCESS;

  return dofile (fname); /* otherwise, execute the sucker */
}

/*
 * change a key command to a string we can print out
 *
 * int c;       sequence to translate
 * char *seq;       destination string for sequence
 */
static void
cmdstr (unsigned int c, char *seq)
{
  char *ptr; /* pointer into current position in sequence */

  ptr = seq;

  /* apply meta sequence if needed */
  if (c & META)
    {
      *ptr++ = 'M';
      *ptr++ = '-';
    }

  /* apply ^X sequence if needed */
  if (c & CTRLX)
    {
      *ptr++ = '^';
      *ptr++ = 'X';
    }

  /* apply SPEC sequence if needed */
  if (c & SPEC)
    {
      *ptr++ = 'F';
      *ptr++ = 'N';
    }

  /* apply control sequence if needed */
  if (c & CONTROL)
    *ptr++ = '^';

  /* Output the final sequence.  */
  *ptr++ = c & 255; /* Strip the prefixes.  */
  *ptr = '\0'; /* Terminate the string.  */
}

/*
 * This function looks a key binding up in the binding table
 *
 * int c;       key to find what is bound to it
 */
fn_t
getbind (unsigned int c)
{
  struct key_tab *ktp;

  ktp = &keytab[0]; /* Look in key table. */
  while (ktp->k_fp != NULL)
    {
      if (ktp->k_code == c)
        return ktp->k_fp;
      ktp++;
    }

  /* no such binding */
  return NULL;
}

/*
 * getfname:
 *  This function takes a ptr to function and gets the name
 *  associated with it.
 */
static char *
getfname (fn_t func)
{
  struct name_bind *nptr; /* pointer into the name binding table */

  /* skim through the table, looking for a match */
  nptr = &names[0];
  while (nptr->n_func != NULL)
    {
      if (nptr->n_func == func)
        return nptr->n_name;
      nptr++;
    }
  return NULL;
}

/*
 * match fname to a function in the names table
 * and return any match or NULL if none
 *
 * char *fname;     name to attempt to match
 */
fn_t fncmatch (char *fname)
{
  struct name_bind *ffp; /* pointer to entry in name binding table */

  /* scan through the table, returning any match */
  ffp = &names[0];
  while (ffp->n_func != NULL)
    {
      if (strcmp (fname, ffp->n_name) == 0)
        return ffp->n_func;
      ffp++;
    }
  return NULL;
}

/*
 * stock:
 *  String key name TO Command Key
 *
 * char *keyname;   name of key to translate to Command key form
 */
static unsigned int
stock (char *keyname)
{
  unsigned int c; /* key sequence to return */

  /* parse it up */
  c = 0;

  /* first, the META prefix */
  if (*keyname == 'M' && *(keyname + 1) == '-')
    {
      c = META;
      keyname += 2;
    }

  /* next the function prefix */
  if (*keyname == 'F' && *(keyname + 1) == 'N')
    {
      c |= SPEC;
      keyname += 2;
    }

  /* control-x as well... (but not with FN) */
  if (*keyname == '^' && *(keyname + 1) == 'X' && !(c & SPEC))
    {
      c |= CTRLX;
      keyname += 2;
    }

  /* a control char? */
  if (*keyname == '^' && *(keyname + 1) != 0)
    {
      c |= CONTROL;
      ++keyname;
    }
  if (*keyname < 32)
    {
      c |= CONTROL;
      *keyname += 'A';
    }

  /* make sure we are not lower case (not with function keys) */
  if (*keyname >= 'a' && *keyname <= 'z' && !(c & SPEC))
    *keyname -= 32;

  /* the final sequence... */
  c |= *keyname & 0xFFU;
  return c;
}

/*
 * string key name to binding name....
 *
 * char *skey;      name of keey to get binding for
 */
char *
transbind (char *skey)
{
  char *bindname;

  bindname = getfname (getbind (stock (skey)));
  if (bindname == NULL)
    bindname = "ERROR";

  return bindname;
}
