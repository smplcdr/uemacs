/* random.c -- implements random.h */
#include "random.h"

#define	NBRACE	1  /* new style brace matching command             */

/*	random.c
 *
 *      This file contains the command processing functions for a number of
 *      random commands. There is no functional grouping here, for sure.
 *
 *	Modified by Petri Kutvonen
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "basic.h"
#include "buffer.h"
#include "display.h"
#include "estruct.h"
#include "execute.h"
#include "input.h"
#include "line.h"
#include "search.h"
#include "terminal.h"
#include "window.h"


static const char *cname[] = {						/* names of colors */
	"BLACK", "RED", "GREEN", "YELLOW", "BLUE",
	"MAGENTA", "CYAN", "WHITE"
#if	PKCODE & IBMPC
	    , "HIGH"
#endif
} ;

#define NCOLORS (sizeof cname / sizeof( *cname))	/* # of supported colors */

int gfcolor = NCOLORS - 1 ;	/* global forgrnd color (white)  */
int gbcolor = 0 ;		/* global backgrnd color (black) */

static int tabsize ;		/* Tab size (0: use real tabs)   */
int fillcol = 72 ;		/* Current fill column           */

/* uninitialized global definitions */

int thisflag ;			/* Flags, this command		*/
int lastflag ;			/* Flags, last command		*/


static int adjustmode( int kind, int global) ;
static int cinsert( void) ;

/*
 * Set fill column to n.
 */
int setfillcol(int f, int n)
{
	fillcol = n;
	mlwrite("(Fill column is %d)", n);
	return TRUE;
}

/*
 * Display the current position of the cursor, in origin 1 X-Y coordinates,
 * the character that is under the cursor (in hex), and the fraction of the
 * text that is before the cursor. The displayed column is not the current
 * column, but the column that would be used on an infinite width display.
 * Normally this is bound to "C-X =".
 */
int showcpos(int f, int n)
{
	struct line *lp;	/* current line */
	long numchars;	/* # of chars in file */
	int numlines;	/* # of lines in file */
	long predchars;	/* # chars preceding point */
	int predlines;	/* # lines preceding point */
	unicode_t curchar ;	/* character under cursor */
	unsigned bytes ;	/* length of unicode sequence */
	int ratio;
	int col;
	int savepos;		/* temp save for current offset */
	int ecol;		/* column pos/end of current line */

	/* starting at the beginning of the buffer */
	lp = lforw(curbp->b_linep);

	/* start counting chars and lines */
	numchars = 0;
	numlines = 0;
	predchars = 0;
	predlines = 0;
	curchar = 0;
	bytes = 1 ;
	while (lp != curbp->b_linep) {
		/* if we are on the current line, record it */
		if (lp == curwp->w_dotp) {
			int len ;
			
			predlines = numlines;
			predchars = numchars + curwp->w_doto;
			len = llength( lp) ;				
			if( (curwp->w_doto) == len)
				curchar = (curbp->b_mode & MDDOS) ? '\r' : '\n' ;
			else
				bytes = utf8_to_unicode( lp->l_text, curwp->w_doto, len, &curchar) ;
		}
		/* on to the next line */
		++numlines;
		numchars += llength( lp) + ((curbp->b_mode & MDDOS) ? 2 : 1) ;
		lp = lforw(lp);
	}

	/* if at end of file, record it */
	if (curwp->w_dotp == curbp->b_linep) {
		predlines = numlines;
		predchars = numchars;
#if	PKCODE
		curchar = 0;
#endif
	}

	/* Get real column and end-of-line column. */
	col = getccol(FALSE);
	savepos = curwp->w_doto;
	curwp->w_doto = llength(curwp->w_dotp);
	ecol = getccol(FALSE);
	curwp->w_doto = savepos;

	ratio = 0;		/* Ratio before dot. */
	if (numchars != 0)
		ratio = (100L * predchars) / numchars;

	/* summarize and report the info */
	mlwrite("Line %d/%d Col %d/%d Char %D/%D (%d%%) char = %s%x",
		predlines + 1, numlines + 1, col, ecol,
		predchars, numchars, ratio, (bytes > 1) ? "\\u" : "0x", curchar);
	return TRUE;
}

int getcline(void)
{				/* get the current line number */
	struct line *lp;	/* current line */
	int numlines;	/* # of lines before point */

	/* starting at the beginning of the buffer */
	lp = lforw(curbp->b_linep);

	/* start counting lines */
	numlines = 0;
	while (lp != curbp->b_linep) {
		/* if we are on the current line, record it */
		if (lp == curwp->w_dotp)
			break;
		++numlines;
		lp = lforw(lp);
	}

	/* and return the resulting count */
	return numlines + 1;
}

/*
 * Return current column.  Stop at first non-blank given TRUE argument.
 */
int getccol(int bflg)
{
	int i, col;
	struct line *dlp = curwp->w_dotp;
	int byte_offset = curwp->w_doto;
	int len = llength(dlp);

	col = i = 0;
	while (i < byte_offset) {
		unicode_t c;

		i += utf8_to_unicode(dlp->l_text, i, len, &c);
		if( bflg && c != ' ' && c != '\t')	/* Request Stop at first non-blank */
			break;
		if (c == '\t')
			col += tabwidth - col % tabwidth ;
		else if (c < 0x20 || c == 0x7F)		/* displayed as ^c */
			col += 2 ;
		else if (c >= 0x80 && c <= 0xa0)	/* displayed as \xx */
			col += 3 ;
		else
			col += 1 ;
	}
	return col;
}

/*
 * Set current column.
 *
 * int pos;		position to set cursor
 */
int setccol(int pos)
{
	int c;		/* character being scanned */
	int i;		/* index into current line */
	int col;	/* current cursor column   */
	int llen;	/* length of line in bytes */

	col = 0;
	llen = llength(curwp->w_dotp);

	/* scan the line until we are at or past the target column */
	for (i = 0; i < llen; ++i) {
		/* upon reaching the target, drop out */
		if (col >= pos)
			break;

		/* advance one character */
		c = lgetc(curwp->w_dotp, i);
		if (c == '\t')
			col += tabwidth - col % tabwidth ;
		else if (c < 0x20 || c == 0x7F)
			col += 2 ;
		else
			col += 1 ;
	}

	/* set us at the new position */
	curwp->w_doto = i;

	/* and tell weather we made it */
	return col >= pos;
}

/*
 * Twiddle the two characters on either side of dot. If dot is at the end of
 * the line twiddle the two characters before it. Return with an error if dot
 * is at the beginning of line; it seems to be a bit pointless to make this
 * work. This fixes up a very common typo with a single stroke. Normally bound
 * to "C-T". This always works within a line, so "WFEDIT" is good enough.
 */
int twiddle(int f, int n)
{
	struct line *dotp;
	int doto;
	int cl;
	int cr;

	if (curbp->b_mode & MDVIEW)	/* don't allow this command if      */
		return rdonly();	/* we are in read only mode     */
	dotp = curwp->w_dotp;
	doto = curwp->w_doto;
	if (doto == llength(dotp) && --doto < 0)
		return FALSE;
	cr = lgetc(dotp, doto);
	if (--doto < 0)
		return FALSE;
	cl = lgetc(dotp, doto);
	lputc(dotp, doto + 0, cr);
	lputc(dotp, doto + 1, cl);
	lchange(WFEDIT);
	return TRUE;
}

/*
 * Quote the next character, and insert it into the buffer. All the characters
 * are taken literally, with the exception of the newline, which always has
 * its line splitting meaning. The character is always read, even if it is
 * inserted 0 times, for regularity. Bound to "C-Q"
 */
int quote(int f, int n)
{
	int s;
	int c;

	if (curbp->b_mode & MDVIEW)	/* don't allow this command if      */
		return rdonly();	/* we are in read only mode     */
	c = tgetc();
	if (n < 0)
		return FALSE;
	if (n == 0)
		return TRUE;
	if (c == '\n') {
		do {
			s = lnewline();
		} while (s == TRUE && --n);
		return s;
	}
	return linsert(n, c);
}

/*
 * Set tab size if given non-default argument (n <> 1).  Otherwise, insert a
 * tab into file.  If given argument, n, of zero, change to true tabs.
 * If n > 1, simulate tab stop every n-characters using spaces. This has to be
 * done in this slightly funny way because the tab (in ASCII) has been turned
 * into "C-I" (in 10 bit code) already. Bound to "C-I".
 */
int insert_tab(int f, int n)
{
	if (n < 0)
		return FALSE;
	if (n == 0 || n > 1) {
		tabsize = n;
		return TRUE;
	}
	if (!tabsize)
		return linsert(1, '\t');
	return linsert(tabsize - (getccol(FALSE) % tabsize), ' ');
}

#if	AEDIT
/*
 * change tabs to spaces
 *
 * int f, n;		default flag and numeric repeat count
 */
int detab(int f, int n)
{
	int inc;	/* increment to next line [sgn(n)] */

	if (curbp->b_mode & MDVIEW)	/* don't allow this command if      */
		return rdonly();	/* we are in read only mode     */

	if (f == FALSE)
		n = 1;

	/* loop thru detabbing n lines */
	inc = ((n > 0) ? 1 : -1);
	while (n) {
		curwp->w_doto = 0;	/* start at the beginning */

		/* detab the entire current line */
		while (curwp->w_doto < llength(curwp->w_dotp)) {
			/* if we have a tab */
			if (lgetc(curwp->w_dotp, curwp->w_doto) == '\t') {
				ldelchar(1, FALSE);
				insspace( TRUE, tabwidth - curwp->w_doto % tabwidth);
			}

			forwchar(FALSE, 1);
		}

		/* advance/or back to the next line */
		forwline(TRUE, inc);
		n -= inc;
	}
	curwp->w_doto = 0;	/* to the begining of the line */
	thisflag &= ~CFCPCN;	/* flag that this resets the goal column */
	lchange(WFEDIT);	/* yes, we have made at least an edit */
	return TRUE;
}

/*
 * change spaces to tabs where posible
 *
 * int f, n;		default flag and numeric repeat count
 */
int entab(int f, int n)
{
#define	nextab(a)	(a + tabwidth - a % tabwidth)

	int inc;	/* increment to next line [sgn(n)] */
	int fspace;	/* pointer to first space if in a run */
	int ccol;	/* current cursor column */
	char cchar;	/* current character */

	if (curbp->b_mode & MDVIEW)	/* don't allow this command if      */
		return rdonly();	/* we are in read only mode     */

	if (f == FALSE)
		n = 1;

	/* loop thru entabbing n lines */
	inc = ((n > 0) ? 1 : -1);
	while (n) {
		curwp->w_doto = 0;	/* start at the beginning */

		/* entab the entire current line */
		fspace = -1;
		ccol = 0;
		while (curwp->w_doto < llength(curwp->w_dotp)) {
			/* see if it is time to compress */
			if ((fspace >= 0) && (nextab(fspace) <= ccol)) {
				if (ccol - fspace < 2)
					fspace = -1;
				else {
					/* there is a bug here dealing with mixed space/tabed
					   lines.......it will get fixed                */
					backchar(TRUE, ccol - fspace);
					ldelete((long) (ccol - fspace),
						FALSE);
					linsert(1, '\t');
					fspace = -1;
				}
			}

			/* get the current character */
			cchar = lgetc(curwp->w_dotp, curwp->w_doto);

			switch (cchar) {
			case '\t':	/* a tab...count em up */
				ccol = nextab(ccol);
				break;

			case ' ':	/* a space...compress? */
				if (fspace == -1)
					fspace = ccol;
				ccol++;
				break;

			default:	/* any other char...just count */
				ccol++;
				fspace = -1;
				break;
			}
			forwchar(FALSE, 1);
		}

		/* advance/or back to the next line */
		forwline(TRUE, inc);
		n -= inc;
	}
	curwp->w_doto = 0;	/* to the begining of the line */
	thisflag &= ~CFCPCN;	/* flag that this resets the goal column */
	lchange(WFEDIT);	/* yes, we have made at least an edit */
	return TRUE;
}

/*
 * trim trailing whitespace from the point to eol
 *
 * int f, n;		default flag and numeric repeat count
 */
int trim(int f, int n)
{
	struct line *lp;	/* current line pointer */
	int offset;	/* original line offset position */
	int length;	/* current length */
	int inc;	/* increment to next line [sgn(n)] */

	if (curbp->b_mode & MDVIEW)	/* don't allow this command if      */
		return rdonly();	/* we are in read only mode     */

	if (f == FALSE)
		n = 1;

	/* loop thru trimming n lines */
	inc = ((n > 0) ? 1 : -1);
	while (n) {
		lp = curwp->w_dotp;	/* find current line text */
		offset = curwp->w_doto;	/* save original offset */
		length = lp->l_used;	/* find current length */

		/* trim the current line */
		while (length > offset) {
			if (lgetc(lp, length - 1) != ' ' &&
			    lgetc(lp, length - 1) != '\t')
				break;
			length--;
		}
		lp->l_used = length;

		/* advance/or back to the next line */
		forwline(TRUE, inc);
		n -= inc;
	}
	lchange(WFEDIT);
	thisflag &= ~CFCPCN;	/* flag that this resets the goal column */
	return TRUE;
}
#endif

/*
 * Open up some blank space. The basic plan is to insert a bunch of newlines,
 * and then back up over them. Everything is done by the subcommand
 * procerssors. They even handle the looping. Normally this is bound to "C-O".
 */
int openline(int f, int n)
{
	int i;
	int s;

	if (curbp->b_mode & MDVIEW)	/* don't allow this command if      */
		return rdonly();	/* we are in read only mode     */
	if (n < 0)
		return FALSE;
	if (n == 0)
		return TRUE;
	i = n;			/* Insert newlines.     */
	do {
		s = lnewline();
	} while (s == TRUE && --i);
	if (s == TRUE)		/* Then back up overtop */
		s = backchar(f, n);	/* of them all.         */
	return s;
}

/*
 * Insert a newline. Bound to "C-M". If we are in CMODE, do automatic
 * indentation as specified.
 */
int insert_newline(int f, int n)
{
	int s;

	if (curbp->b_mode & MDVIEW)	/* don't allow this command if      */
		return rdonly();	/* we are in read only mode     */
	if (n < 0)
		return FALSE;

	/* if we are in C mode and this is a default <NL> */
	if (n == 1 && (curbp->b_mode & MDCMOD) &&
	    curwp->w_dotp != curbp->b_linep)
		return cinsert();

	/*
	 * If a newline was typed, fill column is defined, the argument is non-
	 * negative, wrap mode is enabled, and we are now past fill column,
	 * and we are not read-only, perform word wrap.
	 */
	if ((curwp->w_bufp->b_mode & MDWRAP) && fillcol > 0 &&
	    getccol(FALSE) > fillcol &&
	    (curwp->w_bufp->b_mode & MDVIEW) == FALSE)
		execute(META | SPEC | 'W', FALSE, 1);

	/* insert some lines */
	while (n--) {
		if ((s = lnewline()) != TRUE)
			return s;
#if SCROLLCODE
		curwp->w_flag |= WFINS;
#endif
	}
	return TRUE;
}

static int cinsert(void)
{				/* insert a newline and indentation for C */
	char *cptr;	/* string pointer into text to copy */
	int tptr;	/* index to scan into line */
	int bracef;	/* was there a brace at the end of line? */
	int i;
	char ichar[NSTRING];	/* buffer to hold indent of last line */

	/* grab a pointer to text to copy indentation from */
	cptr = &curwp->w_dotp->l_text[0];

	/* check for a brace */
	tptr = curwp->w_doto - 1;
	bracef = (cptr[tptr] == '{');

	/* save the indent of the previous line */
	i = 0;
	while ((i < tptr) && (cptr[i] == ' ' || cptr[i] == '\t')
	       && (i < NSTRING - 1)) {
		ichar[i] = cptr[i];
		++i;
	}
	ichar[i] = 0;		/* terminate it */

	/* put in the newline */
	if (lnewline() == FALSE)
		return FALSE;

	/* and the saved indentation */
	linstr(ichar);

	/* and one more tab for a brace */
	if (bracef)
		insert_tab(FALSE, 1);

#if SCROLLCODE
	curwp->w_flag |= WFINS;
#endif
	return TRUE;
}

#if	NBRACE
/*
 * insert a brace into the text here...we are in CMODE
 *
 * int n;	repeat count
 * int c;	brace to insert (always } for now)
 */
int insbrace(int n, int c)
{
	int ch;	/* last character before input */
	int oc;	/* caractere oppose a c */
	int i, count;
	int target;	/* column brace should go after */
	struct line *oldlp;
	int oldoff;

	/* if we aren't at the beginning of the line... */
	if (curwp->w_doto != 0)

		/* scan to see if all space before this is white space */
		for (i = curwp->w_doto - 1; i >= 0; --i) {
			ch = lgetc(curwp->w_dotp, i);
			if (ch != ' ' && ch != '\t')
				return linsert(n, c);
		}

	/* chercher le caractere oppose correspondant */
	switch (c) {
	case '}':
		oc = '{';
		break;
	case ']':
		oc = '[';
		break;
	case ')':
		oc = '(';
		break;
	default:
		return FALSE;
	}

	oldlp = curwp->w_dotp;
	oldoff = curwp->w_doto;

	count = 1;
	backchar(FALSE, 1);

	while (count > 0) {
		if (curwp->w_doto == llength(curwp->w_dotp))
			ch = '\n';
		else
			ch = lgetc(curwp->w_dotp, curwp->w_doto);

		if (ch == c)
			++count;
		if (ch == oc)
			--count;

		backchar(FALSE, 1);
		if (boundry(curwp->w_dotp, curwp->w_doto, REVERSE))
			break;
	}

	if (count != 0) {	/* no match */
		curwp->w_dotp = oldlp;
		curwp->w_doto = oldoff;
		return linsert(n, c);
	}

	curwp->w_doto = 0;	/* debut de ligne */
	/* aller au debut de la ligne apres la tabulation */
	while ((ch = lgetc(curwp->w_dotp, curwp->w_doto)) == ' '
	       || ch == '\t')
		forwchar(FALSE, 1);

	/* delete back first */
	target = getccol(FALSE);	/* c'est l'indent que l'on doit avoir */
	curwp->w_dotp = oldlp;
	curwp->w_doto = oldoff;

	while (target != getccol(FALSE)) {
		if (target < getccol(FALSE))	/* on doit detruire des caracteres */
			while (getccol(FALSE) > target)
				backdel(FALSE, 1);
		else {		/* on doit en inserer */
			while (target - getccol(FALSE) >= 8)
				linsert(1, '\t');
			linsert(target - getccol(FALSE), ' ');
		}
	}

	/* and insert the required brace(s) */
	return linsert(n, c);
}

#else

/*
 * insert a brace into the text here...we are in CMODE
 *
 * int n;		repeat count
 * int c;		brace to insert (always { for now)
 */
int insbrace(int n, int c)
{
	int ch;	/* last character before input */
	int i;
	int target;	/* column brace should go after */

	/* if we are at the beginning of the line, no go */
	if (curwp->w_doto == 0)
		return linsert(n, c);

	/* scan to see if all space before this is white space */
	for (i = curwp->w_doto - 1; i >= 0; --i) {
		ch = lgetc(curwp->w_dotp, i);
		if (ch != ' ' && ch != '\t')
			return linsert(n, c);
	}

	/* delete back first */
	target = getccol(FALSE);	/* calc where we will delete to */
	target -= 1;
	target -= target % (tabsize == 0 ? 8 : tabsize);
	while (getccol(FALSE) > target)
		backdel(FALSE, 1);

	/* and insert the required brace(s) */
	return linsert(n, c);
}
#endif

int inspound(void)
{				/* insert a # into the text here...we are in CMODE */
	int ch;	/* last character before input */
	int i;

	/* if we are at the beginning of the line, no go */
	if (curwp->w_doto == 0)
		return linsert(1, '#');

	/* scan to see if all space before this is white space */
	for (i = curwp->w_doto - 1; i >= 0; --i) {
		ch = lgetc(curwp->w_dotp, i);
		if (ch != ' ' && ch != '\t')
			return linsert(1, '#');
	}

	/* delete back first */
	while (getccol(FALSE) >= 1)
		backdel(FALSE, 1);

	/* and insert the required pound */
	return linsert(1, '#');
}

/*
 * Delete blank lines around dot. What this command does depends if dot is
 * sitting on a blank line. If dot is sitting on a blank line, this command
 * deletes all the blank lines above and below the current line. If it is
 * sitting on a non blank line then it deletes all of the blank lines after
 * the line. Normally this command is bound to "C-X C-O". Any argument is
 * ignored.
 */
int deblank(int f, int n)
{
	struct line *lp1;
	struct line *lp2;
	long nld;

	if (curbp->b_mode & MDVIEW)	/* don't allow this command if      */
		return rdonly();	/* we are in read only mode     */
	lp1 = curwp->w_dotp;
	while (llength(lp1) == 0 && (lp2 = lback(lp1)) != curbp->b_linep)
		lp1 = lp2;
	lp2 = lp1;
	nld = 0;
	while ((lp2 = lforw(lp2)) != curbp->b_linep && llength(lp2) == 0)
		++nld;
	if (nld == 0)
		return TRUE;
	curwp->w_dotp = lforw(lp1);
	curwp->w_doto = 0;
	return ldelete(nld, FALSE);
}

/*
 * Insert a newline, then enough tabs and spaces to duplicate the indentation
 * of the previous line. Assumes tabs are every tabwidth characters.
 * Figure out the indentation of the current line. Insert a newline by calling
 * the standard routine. Insert the indentation by inserting the right number
 * of tabs and spaces. Return TRUE if all ok. Return FALSE if one of the
 * subcomands failed. Normally bound to "C-J".
 */
int indent( int f, int n) {
	int nicol ;
	int i ;

	if( curbp->b_mode & MDVIEW)	/* don't allow this command if	*/
		return rdonly() ;		/* we are in read only mode		*/

	if( n < 0)
		return FALSE ;

/* number of columns to indent */
	nicol = 0 ;
	for( i = 0 ; i < llength( curwp->w_dotp) ; i += 1) {
		int c ;

		c = lgetc( curwp->w_dotp, i) ;
		if( c == '\t')
			nicol += tabwidth - nicol % tabwidth ;
		else if( c == ' ')
			nicol += 1 ;
		else
			break ;
	}

	i = nicol / tabwidth ;	/* # of tab to insert */
	nicol %= tabwidth ;		/* # of space to insert */
	while( n--)
		if( lnewline() == FALSE
		|| ( i != 0 && linsert( i, '\t') == FALSE)
		|| ( nicol  != 0 && linsert( nicol, ' ') == FALSE))
			return FALSE ;

	return TRUE ;
}

/*
 * Delete forward. This is real easy, because the basic delete routine does
 * all of the work. Watches for negative arguments, and does the right thing.
 * If any argument is present, it kills rather than deletes, to prevent loss
 * of text if typed with a big argument. Normally bound to "C-D".
 */
int forwdel(int f, int n)
{
	if (curbp->b_mode & MDVIEW)	/* don't allow this command if      */
		return rdonly();	/* we are in read only mode     */
	if (n < 0)
		return backdel(f, -n);
	if (f != FALSE) {	/* Really a kill.       */
		if ((lastflag & CFKILL) == 0)
			kdelete();
		thisflag |= CFKILL;
	}
	return ldelchar((long) n, f);
}

/*
 * Delete backwards. This is quite easy too, because it's all done with other
 * functions. Just move the cursor back, and delete forwards. Like delete
 * forward, this actually does a kill if presented with an argument. Bound to
 * both "RUBOUT" and "C-H".
 */
int backdel(int f, int n)
{
	int s;

	if (curbp->b_mode & MDVIEW)	/* don't allow this command if      */
		return rdonly();	/* we are in read only mode     */
	if (n < 0)
		return forwdel(f, -n);
	if (f != FALSE) {	/* Really a kill.       */
		if ((lastflag & CFKILL) == 0)
			kdelete();
		thisflag |= CFKILL;
	}
	if ((s = backchar(f, n)) == TRUE)
		s = ldelchar(n, f);
	return s;
}

/*
 * Kill text. If called without an argument, it kills from dot to the end of
 * the line, unless it is at the end of the line, when it kills the newline.
 * If called with an argument of 0, it kills from the start of the line to dot.
 * If called with a positive argument, it kills from dot forward over that
 * number of newlines. If called with a negative argument it kills backwards
 * that number of newlines. Normally bound to "C-K".
 */
int killtext(int f, int n)
{
	struct line *nextp;
	long chunk;

	if (curbp->b_mode & MDVIEW)	/* don't allow this command if      */
		return rdonly();	/* we are in read only mode     */
	if ((lastflag & CFKILL) == 0)	/* Clear kill buffer if */
		kdelete();	/* last wasn't a kill.  */
	thisflag |= CFKILL;
	if (f == FALSE) {
		chunk = llength(curwp->w_dotp) - curwp->w_doto;
		if (chunk == 0)
			chunk = 1;
	} else if (n == 0) {
		chunk = curwp->w_doto;
		curwp->w_doto = 0;
	} else if (n > 0) {
		chunk = llength(curwp->w_dotp) - curwp->w_doto + 1;
		nextp = lforw(curwp->w_dotp);
		while (--n) {
			if (nextp == curbp->b_linep)
				return FALSE;
			chunk += llength(nextp) + 1;
			nextp = lforw(nextp);
		}
	} else {
		mlwrite("neg kill");
		return FALSE;
	}
	return ldelete(chunk, TRUE);
}

/*
 * prompt and set an editor mode
 *
 * int f, n;		default and argument
 */
int setemode(int f, int n)
{
#if 	PKCODE
	return adjustmode(TRUE, FALSE);
#else
	adjustmode(TRUE, FALSE);
#endif
}

/*
 * prompt and delete an editor mode
 *
 * int f, n;		default and argument
 */
int delmode(int f, int n)
{
#if	PKCODE
	return adjustmode(FALSE, FALSE);
#else
	adjustmode(FALSE, FALSE);
#endif
}

/*
 * prompt and set a global editor mode
 *
 * int f, n;		default and argument
 */
int setgmode(int f, int n)
{
#if	PKCODE
	return adjustmode(TRUE, TRUE);
#else
	adjustmode(TRUE, TRUE);
#endif
}

/*
 * prompt and delete a global editor mode
 *
 * int f, n;		default and argument
 */
int delgmode(int f, int n)
{
#if	PKCODE
	return adjustmode(FALSE, TRUE);
#else
	adjustmode(FALSE, TRUE);
#endif
}

/*
 * change the editor mode status
 *
 * int kind;		true = set,          false = delete
 * int global;		true = global flag,  false = current buffer flag
 */
static int adjustmode( int kind, int global) {
	unsigned i ;	/* loop index */
	int status;	/* error return on input */
	char prompt[50];	/* string to prompt user with */
	char *cbuf ;	/* buffer to recieve mode name into */

	/* build the proper prompt string */
	if (global)
		strcpy(prompt, "Global mode to ");
	else
		strcpy(prompt, "Mode to ");

	if (kind == TRUE)
		strcat(prompt, "add: ");
	else
		strcat(prompt, "delete: ");

	/* prompt the user and get an answer */

	status = newmlarg( &cbuf, prompt, 0) ;
	if (status != TRUE)
		return status;

	/* test it first against the colors we know */
	for (i = 0; i < NCOLORS; i++) {
		if( strcasecmp( cbuf, cname[ i]) == 0) {
			/* finding the match, we set the color */
#if	COLOR
			if( *cbuf >= 'A' && *cbuf <= 'Z') {
				if (global)
					gfcolor = i;
#if	PKCODE == 0
				else
#endif
					curwp->w_fcolor = i;
			} else {
				if (global)
					gbcolor = i;
#if	PKCODE == 0
				else
#endif
					curwp->w_bcolor = i;
			}

			curwp->w_flag |= WFCOLR;
#endif
			mlerase();
			free( cbuf) ;
			return TRUE;
		}
	}

	/* test it against the modes we know */

	for (i = 0; i < NUMMODES; i++) {
		if( strcasecmp( cbuf, modename[ i]) == 0) {
			/* finding a match, we process it */
			if (kind == TRUE)
				if (global)
					gmode |= (1 << i);
				else
					curbp->b_mode |= (1 << i);
			else if (global)
				gmode &= ~(1 << i);
			else
				curbp->b_mode &= ~(1 << i);
			/* display new mode line */
			if (global == 0)
				upmode();
			mlerase();	/* erase the junk */
			free( cbuf) ;
			return TRUE;
		}
	}

	mlwrite("No such mode!");
	free( cbuf) ;
	return FALSE;
}

#if	CFENCE
/*
 * the cursor is moved to a matching fence
 *
 * int f, n;		not used
 */
int getfence(int f, int n)
{
	struct line *oldlp;	/* original line pointer */
	int oldoff;	/* and offset */
	int sdir;	/* direction of search (1/-1) */
	int count;	/* current fence level count */
	char ch;	/* fence type to match against */
	char ofence;	/* open fence */
	char c;	/* current character in scan */

	/* save the original cursor position */
	oldlp = curwp->w_dotp;
	oldoff = curwp->w_doto;

	/* get the current character */
	if (oldoff == llength(oldlp))
		ch = '\n';
	else
		ch = lgetc(oldlp, oldoff);

	/* setup proper matching fence */
	switch (ch) {
	case '(':
		ofence = ')';
		sdir = FORWARD;
		break;
	case '{':
		ofence = '}';
		sdir = FORWARD;
		break;
	case '[':
		ofence = ']';
		sdir = FORWARD;
		break;
	case ')':
		ofence = '(';
		sdir = REVERSE;
		break;
	case '}':
		ofence = '{';
		sdir = REVERSE;
		break;
	case ']':
		ofence = '[';
		sdir = REVERSE;
		break;
	default:
		TTbeep();
		return FALSE;
	}

	/* set up for scan */
	count = 1;
	if (sdir == REVERSE)
		backchar(FALSE, 1);
	else
		forwchar(FALSE, 1);

	/* scan until we find it, or reach the end of file */
	while (count > 0) {
		if (curwp->w_doto == llength(curwp->w_dotp))
			c = '\n';
		else
			c = lgetc(curwp->w_dotp, curwp->w_doto);
		if (c == ch)
			++count;
		if (c == ofence)
			--count;
		if (sdir == FORWARD)
			forwchar(FALSE, 1);
		else
			backchar(FALSE, 1);
		if (boundry(curwp->w_dotp, curwp->w_doto, sdir))
			break;
	}

	/* if count is zero, we have a match, move the sucker */
	if (count == 0) {
		if (sdir == FORWARD)
			backchar(FALSE, 1);
		else
			forwchar(FALSE, 1);
		curwp->w_flag |= WFMOVE;
		return TRUE;
	}

	/* restore the current position */
	curwp->w_dotp = oldlp;
	curwp->w_doto = oldoff;
	TTbeep();
	return FALSE;
}
#endif

/*
 * Close fences are matched against their partners, and if
 * on screen the cursor briefly lights there
 *
 * char ch;			fence type to match against
 */
int fmatch(int ch)
{
	struct line *oldlp;	/* original line pointer */
	int oldoff;	/* and offset */
	struct line *toplp;	/* top line in current window */
	int count;	/* current fence level count */
	char opench;	/* open fence */
	char c;	/* current character in scan */
	int i;

	/* first get the display update out there */
	update(FALSE);

	/* save the original cursor position */
	oldlp = curwp->w_dotp;
	oldoff = curwp->w_doto;

	/* setup proper open fence for passed close fence */
	if (ch == ')')
		opench = '(';
	else if (ch == '}')
		opench = '{';
	else
		opench = '[';

	/* find the top line and set up for scan */
	toplp = curwp->w_linep->l_bp;
	count = 1;
	backchar(FALSE, 2);

	/* scan back until we find it, or reach past the top of the window */
	while (count > 0 && curwp->w_dotp != toplp) {
		if (curwp->w_doto == llength(curwp->w_dotp))
			c = '\n';
		else
			c = lgetc(curwp->w_dotp, curwp->w_doto);
		if (c == ch)
			++count;
		if (c == opench)
			--count;
		backchar(FALSE, 1);
		if (curwp->w_dotp == curwp->w_bufp->b_linep->l_fp &&
		    curwp->w_doto == 0)
			break;
	}

	/* if count is zero, we have a match, display the sucker */
	/* there is a real machine dependant timing problem here we have
	   yet to solve......... */
	if (count == 0) {
		forwchar(FALSE, 1);
		for (i = 0; i < term.t_pause; i++)
			update(FALSE);
	}

	/* restore the current position */
	curwp->w_dotp = oldlp;
	curwp->w_doto = oldoff;
	return TRUE;
}

static int iovstring( int f, int n, const char *prompt, int (*fun)( char *)) {
	int status ;	/* status return code */
	char *tstring ;	/* string to add */

	/* ask for string to insert */
	status = newmlargt( &tstring, prompt, 0) ; /* grab as big a token as screen allow */
	if( tstring == NULL)
		return status ;

	if( f == FALSE)
		n = 1 ;
	else if( n < 0)
		n = -n ;

	/* insert it */
	while( n-- && status == TRUE)
		status = fun( tstring) ;

	free( tstring) ;
	return status ;
}

/*
 * ask for and insert a string into the current
 * buffer at the current point
 *
 * int f, n;		ignored arguments
 */
int istring( int f, int n) {
	return iovstring( f, n, "String to insert<META>: ", linstr) ;
}

/*
 * ask for and overwite a string into the current
 * buffer at the current point
 *
 * int f, n;		ignored arguments
 */
int ovstring( int f, int n) {
	return iovstring( f, n, "String to overwrite<META>: ", lover) ;
}

/* end of random.c */
