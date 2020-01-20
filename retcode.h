#ifndef __RETCODE_H__
#define __RETCODE_H__

#ifdef FAILURE
# error "FAILURE should not be defined"
# undef FAILURE
#endif
#ifdef SUCCESS
# error "SUCCESS should not be defined"
# undef SUCCESS
#endif
#ifdef ABORT
# error "ABORT should not be defined"
# undef ABORT
#endif

#define FAILURE 0 /* 0, false, no, bad, etc.  */
#define SUCCESS 1 /* 1, true, yes, good, etc.  */
#define ABORT   2 /* 2, death, ^G, abort, etc.  */

#endif
