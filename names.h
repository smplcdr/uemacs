
#ifndef _NAMES_H
#define _NAMES_H 1

#include "defines.h"

/* Structure for the name binding table.  */
struct name_bind
{
  char *n_name;             /* Name of function key.  */
  int (*n_func) (bool, int); /* Function name is bound to.  */
};

extern struct name_bind names[]; /* Name to function table.  */

#endif
