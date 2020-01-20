/* mlout.c -- implements mlout.h */

#include "mlout.h"

/* By default just use a stub.  */
static void
mloutstub (const char *buf, ...)
{
  (void) buf;
  return;
}

void (*mloutfmt) (const char *, ...) = mloutstub;

void
mloutstr (const char *str)
{
  /* Call this function always to allow to clean
     line using `mloutstr ("");` */
  mloutfmt ("%s", str);
}

/* end of mlout.c */
