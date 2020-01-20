/* basic.h -- basic commands for cursor movement in active window.  */

#ifndef _BASIC_H
#define _BASIC_H 1

#include "defines.h"

/*
 * $overlap is the size of the line overlap when kbd calls page forw/back,
 * if 0, page will move by 2/3 of the window size (1/3 page overlap)
 * default to 0
 */
#define DEFAULT_OVERLAP 0
extern int overlap; /* Line overlap in forw/back page.  */

/* $target (== curgoal) is the column target when doing line move.  */
extern int curgoal; /* Goal for C-P previous-line, C-N next-line.  */

/* Move the cursor to the beginning of the current line of active window.
   Bound to "C-a".  */
extern int gotobol (bool f, int n);
/* Move the cursor to the end of the current line of active window.
   Bound to "C-e".  */
extern int gotoeol (bool f, int n);

/*
 * Goto the beginning of the buffer. Massive adjustment of dot. This is
 * considered to be hard motion; it really isn't if the original value of dot
 * is the same as the new value of dot. Normally bound to "M-<".
 */
extern int gotobob (bool f, int n);
/*
 * Move to the end of the buffer. Dot is always put at the end of the file
 * (ZJ). The standard screen code does most of the hard parts of update.
 * Bound to "M->".
 */
extern int gotoeob (bool f, int n);

/*
 * Move to a particular line.
 *
 * @n: The specified line position at the current buffer.
 */
extern int gotoline (bool f, int n);

/*
 * Move forward by full lines. If the number of lines to move is less than
 * zero, call the backward line function to actually do it. The last command
 * controls how the goal column is set. Bound to "C-N". No errors are
 * possible.
 */
extern int forwline (bool f, int n);
/*
 * This function is like "forwline", but goes backwards. The scheme is exactly
 * the same. Check for arguments that are less than zero and call your
 * alternate. Figure out the new line and call "movedot" to perform the
 * motion. No errors are possible. Bound to "C-P".
 */
extern int backline (bool f, int n);

/*
 * Scroll forward by a specified number of lines, or by a full page if no
 * argument. Bound to "C-V". The "2" in the arithmetic on the window size is
 * the overlap; this value is the default overlap value in ITS EMACS. Because
 * this zaps the top line in the display window, we have to do a hard update.
 */
extern int forwpage (bool f, int n);
/*
 * This command is like "forwpage", but it goes backwards. The "2", like
 * above, is the overlap between the two windows. The value is from the ITS
 * EMACS manual. Bound to "M-V". We do a hard update for exactly the same
 * reason.
 */
extern int backpage (bool f, int n);

/*
 * Set the mark in the current window to the value of "." in the window. No
 * errors are possible. Bound to "M-.".
 */
extern int setmark (bool f, int n);
/*
 * Swap the values of "." and "mark" in the current window. This is pretty
 * easy, because all of the hard work gets done by the standard routine
 * that moves the mark about. The only possible error is "no mark". Bound to
 * "C-X C-X".
 */
extern int swapmark (bool f, int n);

#endif
