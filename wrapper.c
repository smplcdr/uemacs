/* wrapper.c -- implements wrapper.h */

#include "wrapper.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#ifdef MINGW32
int
mkstemp (char *template)
{
  return -1;
}
#endif

static void
die (const char *err)
{
  fputs ("fatal: ", stderr);
  fputs (err, stderr);
  fputs ("\n", stderr);
  exit (EXIT_FAILURE);
}

/* Function copyright: git */
void
xmkstemp (char *template)
{
  int fd;
  mode_t o_mask;

  o_mask = umask (0177);
  fd = mkstemp (template);
  if (fd < 0)
    die ("Unable to create temporary file");

  umask (o_mask);
  close (fd);
}

void *
xmalloc (size_t size)
{
  void *ret = malloc (size);
  if (!ret)
    die ("Out of memory");

  return ret;
}

/* end of wrapper.c */
