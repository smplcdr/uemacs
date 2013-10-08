/*	edef.h
 *
 *	Global variable definitions
 *
 *	written by Dave G. Conroy
 *	modified by Steve Wilhite, George Jones
 *	greatly modified by Daniel Lawrence
 *	modified by Petri Kutvonen
 */
#ifndef EDEF_H_
#define EDEF_H_

#include "buffer.h"
#include "estruct.h"

#include <stdlib.h>
#include <string.h>

/* Initialized global external declarations. */

extern int fillcol;		/* Fill column                  */


extern int eolexist;		/* does clear to EOL exist?     */
extern int revexist;		/* does reverse video exist?    */
extern int flickcode;		/* do flicker supression?       */
extern int gfcolor;		/* global forgrnd color (white) */
extern int gbcolor;		/* global backgrnd color (black) */
extern int sgarbf;		/* State of screen unknown      */
extern int clexec;		/* command line execution flag  */
extern int discmd;		/* display command flag         */
extern int disinp;		/* display input characters     */

extern int metac;		/* current meta character */
extern int ctlxc;		/* current control X prefix char */
extern int reptc;		/* current universal repeat char */
extern int abortc;		/* current abort command char   */

extern int tabmask;


extern int restflag;		/* restricted use?              */
extern long envram;		/* # of bytes current in use by malloc */
extern int rval;		/* return value of a subprocess */
extern int overlap;		/* line overlap in forw/back page */
extern int scrollcount;		/* number of lines to scroll */

/* Uninitialized global external declarations. */

#define CFCPCN  0x0001		/* Last command was C-P, C-N    */
#define CFKILL  0x0002		/* Last command was a kill      */

extern int thisflag;		/* Flags, this command          */
extern int lastflag;		/* Flags, last command          */

extern int curgoal;		/* Goal for C-P, C-N            */

extern char sres[NBUFN];	        /* Current screen resolution.   */


#endif  /* EDEF_H_ */
