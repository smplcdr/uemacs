#ifndef __RETCODE_H__
#define __RETCODE_H__

#ifdef FALSE
# error "FALSE should not be defined"
# undef FALSE
#endif
#ifdef TRUE
# error "TRUE should not be defined"
# undef TRUE
#endif
#ifdef ABORT
# error "ABORT should not be defined"
# undef ABORT
#endif

#define FALSE 0 /* 0, false, no, bad, etc.  */
#define TRUE  1 /* 1, true, yes, good, etc.  */
#define ABORT 2 /* 2, death, ^G, abort, etc.  */

#endif
