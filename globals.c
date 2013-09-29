#include "crypt.h"
#include "defines.h"

/* #include "estruct.h" */
#include "edef.h"

/* initialized global definitions */

int fillcol = 72;		/* Current fill column          */
char *execstr = NULL;		/* pointer to string to execute */


int eolexist = TRUE;		/* does clear to EOL exist      */
int revexist = FALSE;		/* does reverse video exist?    */
int flickcode = FALSE;		/* do flicker supression?       */
const char *modename[] = {	/* name of modes                */
	"WRAP", "CMODE", "SPELL", "EXACT", "VIEW", "OVER",
	"MAGIC",
#if CRYPT
	"CRYPT",
#else
	"",
#endif
	"ASAVE", "UTF-8", "DOS"
};

const char *mode2name[] = {	/* name of modes                */
	"Wrap", "Cmode", "Spell", "Exact", "View", "Over",
	"Magic",
#if CRYPT
	"Crypt",
#else
	"",
#endif
	"Asave", "utf-8", "Dos"
};

const char modecode[] = "WCSEVOMYAUD"; /* letters to represent modes   */

int gmode = 0;			/* global editor mode           */
int gflags = GFREAD;		/* global control flag          */
#if	PKCODE & IBMPC
int gfcolor = 8;		/* global forgrnd color (white) */
#else
int gfcolor = 7;		/* global forgrnd color (white) */
#endif
int gbcolor = 0;		/* global backgrnd color (black) */
int gasave = 256;		/* global ASAVE size            */
int gacount = 256;		/* count until next ASAVE       */
int sgarbf = TRUE;		/* TRUE if screen is garbage    */
int mpresf = FALSE;		/* TRUE if message in last line */
int clexec = FALSE;		/* command line execution flag  */
int mstore = FALSE;		/* storing text to macro flag   */
int discmd = TRUE;		/* display command flag         */
int disinp = TRUE;		/* display input characters     */

int vtrow = 0;			/* Row location of SW cursor */
int vtcol = 0;			/* Column location of SW cursor */
int lbound = 0;			/* leftmost column of current line
				   being displayed */
int taboff = 0;			/* tab offset for display       */
int metac = CONTROL | '[';	/* current meta character */
int ctlxc = CONTROL | 'X';	/* current control X prefix char */
int reptc = CONTROL | 'U';	/* current universal repeat char */
int abortc = CONTROL | 'G';	/* current abort command char   */

int quotec = 0x11;		/* quote char during mlreply() */
int tabmask = 0x07;		/* tabulator mask */


kbdstate kbdmode = STOP;		/* current keyboard macro mode  */
int kbdrep = 0;			/* number of repetitions        */
int restflag = FALSE;		/* restricted use?              */
int lastkey = 0;		/* last keystoke                */
int seed = 0;			/* random number seed           */
long envram = 0l;		/* # of bytes current in use by malloc */
int macbug = FALSE;		/* macro debuging flag          */
char errorm[] = "ERROR";	/* error literal                */
char truem[] = "TRUE";		/* true literal                 */
char falsem[] = "FALSE";	/* false litereal               */
int cmdstatus = TRUE;		/* last command status          */
char palstr[49] = "";		/* palette string               */
int saveflag = 0;		/* Flags, saved with the $target var */
int rval = 0;			/* return value of a subprocess */
int overlap = 0;		/* line overlap in forw/back page */
int scrollcount = 1;		/* number of lines to scroll */

/* uninitialized global definitions */

int currow;			/* Cursor row                   */
int curcol;			/* Cursor column                */
int thisflag;			/* Flags, this command          */
int lastflag;			/* Flags, last command          */
int curgoal;			/* Goal for C-P, C-N            */
struct window *curwp;		/* Current window               */
struct buffer *curbp;			/* Current buffer               */
struct window *wheadp;		/* Head of list of windows      */
struct buffer *bheadp;			/* Head of list of buffers      */
struct buffer *blistp;			/* Buffer for C-X C-B           */

char sres[NBUFN];		/* current screen resolution    */


#if	DEBUGM
/*	vars needed for macro debugging output	*/
char outline[NSTRING];		/* global string to hold debug line text */
#endif
