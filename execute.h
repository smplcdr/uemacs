
#ifndef _EXECUTE_H
#define _EXECUTE_H 1

#include "defines.h"

extern int gasave;  /* global ASAVE size            */
extern int gacount; /* count until next ASAVE       */

int execute (int c, bool f, int n);
void kbd_loop (void);

#endif
