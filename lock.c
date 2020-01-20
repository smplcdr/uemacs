/* lock.c -- implements lock.h */

#include "estruct.h"
#include "lock.h"

/*  LOCK.C
 *
 *  File locking command routines
 *
 *  written by Daniel Lawrence
 */

#if BSD | SVR4
# include <stdio.h>
# include <stdlib.h>
# include <string.h>

# include "defines.h"
# include "display.h"
# include "input.h"
# include "retcode.h"
# include "util.h"

# if (FILOCK && BSD) || SVR4
#  include "pklock.h"
# endif

# include <sys/errno.h>

# define NLOCKS 128         /* Max # of file locks active.  */
static char *lname[NLOCKS]; /* Names of all locked files.  */
static int numlocks;        /* # of current locks active.  */

static void lckerror (char *errstr);

/* Check a file for locking and add it to the list.  */
int
lockchk (const char *fname)
{
  int i;
  int status;

  /* Check to see if that file is already locked here.  */
  if (numlocks > 0)
    for (i = 0; i < numlocks; i++)
      if (strcmp (fname, lname[i]) == 0)
        return TRUE;

  /* If we have a full locking table, bitch and leave.  */
  if (numlocks >= NLOCKS)
    {
      mlwrite ("LOCK ERROR: Lock table full");
      return ABORT;
    }

  /* Next, try to lock it.  */
  status = lock (fname);
  if (status == ABORT) /* File is locked, no override.  */
    return ABORT;
  if (status == FALSE) /* Locked, overriden, dont add to table.  */
    return TRUE;

  /* We have now locked it, add it to our table.  */
  lname[numlocks] = malloc (strlen (fname) + 1);
  if (lname[numlocks] == NULL)
    {
      undolock (fname);
      mlwrite ("Cannot lock, memory exhausted");
      return ABORT;
    }

  /* everthing is cool, add it to the table */
  strcpy (lname[numlocks], fname);
  numlocks++;
  return TRUE;
}

/*
 * lockrel:
 *  release all the file locks so others may edit
 */
int
lockrel (void)
{
  int i;      /* loop index */
  int status; /* status of locks */
  int s;      /* status of one unlock */

  status = SUCCESS;
  if (numlocks > 0)
    for (i = 0; i < numlocks; i++)
      {
        if ((s = unlock (lname[i])) != SUCCESS)
          status = s;
        free (lname[i]);
      }
  numlocks = 0;
  return status;
}

/*
 * lock:
 *  Check and lock a file from access by others
 *  returns TRUE = files was not locked and now is
 *    FALSE = file was locked and overridden
 *    ABORT = file was locked, abort command
 *
 * const char *fname;   file name to lock
 */
int
lock (const char *fname)
{
  char *locker;      /* lock error message */
  int status;        /* return status */
  char msg[NSTRING]; /* message string */

  /* attempt to lock the file */
  locker = dolock (fname);
  if (locker == NULL) /* we win */
    return TRUE;

  /* file failed...abort */
  if (strncmp (locker, "LOCK", 4) == 0)
    {
      lckerror (locker);
      return ABORT;
    }

  /* someone else has it....override? */
  strcpy (msg, "File in use by ");
  strcat (msg, locker);
  strcat (msg, ", override?");
  status = mlyesno (msg); /* ask them */
  if (status == TRUE)
    return FALSE;
  else
    return ABORT;
}

/*
 * unlock:
 *  Unlock a file
 *  this only warns the user if it fails
 *
 * const char *fname;   file to unlock
 */
int
unlock (const char *fname)
{
  char *locker; /* undolock return string */

  /* unclock and return */
  locker = undolock (fname);
  if (locker == NULL)
    return TRUE;

  /* report the error and come back */
  lckerror (locker);
  return FALSE;
}

/*
 * report a lock error
 *
 * char *errstr;  lock error string to print out
 */
static void
lckerror (char *errstr)
{
  char obuf[NSTRING]; /* Output buffer for error message.  */

  strscpy (obuf, errstr, sizeof (obuf));
  strscat (obuf, " -- ", sizeof (obuf));
  strscat (obuf, strerror (errno), sizeof (obuf));
  mlwrite (obuf);
}

#else
typedef int dummy;
#endif

/* end of lock.c */
