/* basic.h -- basic commands for cursor movement in active window */

#ifndef _BASIC_H_
#define _BASIC_H_

#include "retcode.h"

/*
** $overlap is the size of the line overlap when kbd calls page forw/back
** if 0, page will move by 2/3 of the window size (1/3 page overlap)
** default to 0
*/
#define DEFAULT_OVERLAP	0
extern int overlap ;		/* line overlap in forw/back page	*/


/* $target (== curgoal) is the column target when doing line move */
extern int curgoal ;		/* Goal for C-P previous-line, C-N next-line */


int gotobol( int f, int n) ;
int gotoeol( int f, int n) ;
int gotoline( int f, int n) ;
int gotobob( int f, int n) ;
int gotoeob( int f, int n) ;
int forwline( int f, int n) ;
int backline( int f, int n) ;
int forwpage( int f, int n) ;
int backpage( int f, int n) ;
int setmark( int f, int n) ;
int swapmark( int f, int n) ;

#endif

/* end of basic.h */
