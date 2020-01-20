#ifndef _TERMIO_H
#define _TERMIO_H 1

#include "defines.h"

#include "utf8.h"

#define TYPEAH 1 /* Type ahead causes update to be skipped.  */

#define HUGE 1000 /* Huge number (for row/col).  */

extern int ttrow; /* Row location of HW cursor.  */
extern int ttcol; /* Column location of HW cursor.  */

extern void ttopen (void);
extern void ttclose (void);
extern int ttputc (unicode_t c);
extern void ttflush (void);
extern int ttgetc (void);
extern int typahead (void);

#endif
