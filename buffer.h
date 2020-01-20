#ifndef _BUFFER_H
#define _BUFFER_H 1

#include "defines.h"

#include <stddef.h>

#include "line.h"

typedef char fname_t[256]; /* file name type */
typedef char bname_t[16];  /* buffer name type */

/*
 * Text is kept in buffers.  A buffer header, described below, exists for every
 * buffer in the system.  The buffers are kept in a big list, so that commands
 * that search for a buffer by name can find the buffer header.  There is a
 * safe store for the dot and mark in the header, but this is only valid if
 * the buffer is not being displayed (that is, if "b_nwnd" is 0).  The text for
 * the buffer is kept in a circularly linked list of lines, with a pointer to
 * the header line in "b_linep".
 *
 * Buffers may be "Inactive" which means the files associated with them
 * have not been read in yet.  These get read in at "use buffer" time.
 */
typedef struct buffer *buffer_p;
struct buffer
{
  buffer_p b_bufp;         /* Link to next struct buffer.  */
  line_p b_dotp;           /* Link to "." struct line structure.  */
  line_p b_markp;          /* The same as the above two.  */
  line_p b_linep;          /* Link to the header struct line.  */
  int b_doto;              /* Offset of "." in above struct line.  */
  int b_marko;             /* Offset for the "mark".  */
  unsigned int b_mode;     /* Editor mode of this buffer.  */
  unsigned int b_active:1; /* Window activated flag.  */
  unsigned int b_nwnd:7;    /* Count of windows on buffer.  */
  unsigned int b_flag;     /* Flags.  */
  fname_t b_fname;         /* File name.  */
  bname_t b_bname;         /* Buffer name.  */
};

extern buffer_p curbp;  /* Current buffer.  */
extern buffer_p bheadp; /* Head of list of buffers.  */
extern buffer_p blistp; /* Buffer for "C-X C-B".  */
extern buffer_p bscratchp; /* *scratch* */

#define BFINVS  0x01 /* Internal invisable buffer.  */
#define BFCHG   0x02 /* Changed since last write.  */
#define BFTRUNC 0x04 /* Buffer was truncated when read.  */

/* Mode flags.  */
#define MDWRAP  (1 << 0) /* Word wrap.  */
#define MDCMOD  (1 << 1) /* C indentation and fence match.  */
#define MDEXACT (1 << 2) /* Exact matching for searches.  */
#define MDVIEW  (1 << 3) /* Read-only buffer.  */
#define MDOVER  (1 << 4) /* Overwrite mode.  */
#define MDMAGIC (1 << 5) /* Regular expresions in search.  */
#define MDASAVE (1 << 6) /* Auto-save mode.  */
#define MDUTF8  (1 << 7) /* UTF-8 mode.  */
#define MDDOS   (1 << 8) /* CRLF EOL mode.  */

#define NUMMODES 9 /* # of defined modes.  */

extern const char *modename[]; /* Text names of modes.  */
extern int gmode; /* Global editor mode.  */

extern int usebuffer (bool f, int n);
extern int nextbuffer (bool f, int n);
extern int swbuffer (buffer_p bp);
extern int killbuffer (bool f, int n);
extern int zotbuf (buffer_p bp);
extern int namebuffer (bool f, int n);
extern int listbuffers (bool f, int n);
extern int anycb (void);
extern int bclear (buffer_p bp);
extern int unmark (bool f, int n);
extern buffer_p bcreate (const char *bname, unsigned int bflag);
/* Lookup a buffer by name.  */
extern buffer_p bfind (const char *bname, unsigned int bflag);

#endif
