/* defines.h */

#ifndef _DEFINES_H
#define _DEFINES_H 1

/* Must define one of
        USG | BSD
*/
#define USG 1

#define PKCODE 1
#define SCROLLCODE 1 /* scrolling code P.K. */
#define ENVFUNC 1

#define NSTRING 128 /* # of bytes, string buffers.  */

#define CONTROL 0x01000000 /* Control flag, or'ed in.  */
#define META    0x02000000 /* Meta flag, or'ed in.  */
#define CTRLX   0x04000000 /* ^X flag, or'ed in.  */
#define SPEC    0x08000000 /* Special key (function keys).  */

typedef char bool;

#define TRUE 1
#define FALSE 0

#endif

/* end of defines.h */
