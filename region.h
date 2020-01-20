#ifndef _REGION_H_
#define _REGION_H_

#include "line.h"

/*
 * The starting position of a region, and the size of the region in
 * characters, is kept in a region structure.  Used by the region commands.
 */
struct region
{
  line_p r_linep; /* Origin struct line address.  */
  int r_offset;   /* Origin struct line offset.  */
  int r_size;     /* Length in characters.  */
};

int killregion (bool f, int n);
int copyregion (bool f, int n);
int lowerregion (bool f, int n);
int upperregion (bool f, int n);
int getregion (struct region *rp);

#endif
