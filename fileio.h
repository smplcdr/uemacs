#ifndef _FILEIO_H
#define _FILEIO_H 1

#include "defines.h"

typedef enum
{
  FIOSUC, /* File I/O, success        */
  FIOFNF, /* File I/O, file not found */
  FIOEOF, /* File I/O, end of file    */
  FIOERR, /* File I/O, error          */
  FIOMEM  /* File I/O, memory exhausted  */
} fio_code;

#define FTYPE_NONE 0
#define FTYPE_UNIX 1
#define FTYPE_DOS 2
#define FTYPE_MAC 4
/* FTYPE_MIXED [ 3, 5, 6, 7] */

#define FCODE_ASCII 0x00
#define FCODE_MASK  0x80
#define FCODE_UTF_8 0x81
#define FCODE_EXTND 0x82
#define FCODE_MIXED 0x83

extern char *fline; /* Dynamic return line.  */
extern int ftype;
extern int fcode;    /* Encoding type.  */
extern int fpayload; /* Actual length of fline content.  */

extern fio_code ffclose (void);
extern fio_code ffgetline (void);
extern fio_code ffputline (char *buf, int nbuf, bool dosflag);
extern fio_code ffropen (const char *fn);
extern fio_code ffwopen (const char *fn);

#endif
