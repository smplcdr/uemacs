#ifndef _LINE_H
#define _LINE_H 1

#include "defines.h"

#include <stddef.h>

#include "retcode.h"
#include "utf8.h"

/*
 * All text is kept in circularly linked lists of "struct line" structures.
 * These begin at the header line (which is the blank line beyond the end of
 * the buffer). This line is pointed to by the "struct buffer". Each line
 * contains a number of bytes in the line (the "used" size), the size of the
 * text array, and the text. The end of line is not stored as a byte; it's
 * implied. Future additions will include update hints, and a list of marks
 * into the line.
 */
typedef struct line *line_p;
struct line
{
  line_p l_fp;      /* Forward link to the next line.  */
  line_p l_bp;      /* Backward link to the previous line.  */
  int l_size;       /* Allocated size.  */
  int l_used;       /* Used size.  */
  char l_text[1];   /* A bunch of characters.  */
};

#define lforw(lp)       ((lp)->l_fp)
#define lback(lp)       ((lp)->l_bp)
#define lgetc(lp, n)    ((lp)->l_text[(n)] & 0xFF)
#define lputc(lp, n, c) ((lp)->l_text[(n)] = (c))
#define llength(lp)     ((lp)->l_used)

extern int tabwidth; /* Map to $tab, default to 8, can be set to [1, .. */

extern char *getkill (void);

extern int backchar (bool f, int n);
extern int forwchar (bool f, int n);

extern void lfree (line_p lp);
extern void lchange (int flag);
extern int insspace (bool f, int n);
extern int linstr (char *instr);
extern int linsert (int n, unicode_t c);
extern int linsert_byte (int n, int c);
extern int lover (char *ostr);
extern int lnewline (void);
extern int ldelete (int n, bool kflag);
extern int ldelchar (int n, bool kflag);
extern int lgetchar (unicode_t *);
extern char *getctext (void);
extern void kdelete (void);
extern int kinsert (int c);
extern int yank (bool f, int n);
extern line_p lalloc (int used); /* Allocate a line of at least USED chars. */

extern int rdonly (void); /* Read Only error message, always returns FALSE.  */

#endif /* _LINE_H */
