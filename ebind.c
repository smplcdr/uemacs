/* ebind.c -- implements ebind.h */
#include "ebind.h"

/*  ebind.c
 *
 *  Initial default key to function bindings
 *
 *  Modified by Petri Kutvonen
 */

#include <stdlib.h>

#include "basic.h"
#include "bind.h"
#include "bindable.h"
#include "buffer.h"
#include "estruct.h"
#include "eval.h"
#include "exec.h"
#include "file.h"
#include "isearch.h"
#include "line.h"
#include "random.h"
#include "region.h"
#include "search.h"
#include "spawn.h"
#include "window.h"
#include "word.h"

/*
 * Command table.
 * This table  is *roughly* in ASCII order, left to right across the
 * characters of the command. This explains the funny location of the
 * control-X commands.
 */
struct key_tab keytab[NBINDS] =
{
  { CONTROL | '?', backdel },
  { CONTROL | 'A', gotobol },
  { CONTROL | 'B', backchar },
  { CONTROL | 'C', insspace },
  { CONTROL | 'D', forwdel },
  { CONTROL | 'E', gotoeol },
  { CONTROL | 'F', forwchar },
  { CONTROL | 'G', ctrlg },
  { CONTROL | 'H', backdel },
  { CONTROL | 'I', insert_tab },
  { CONTROL | 'J', indent },
  { CONTROL | 'K', killtext },
  { CONTROL | 'L', redraw },
  { CONTROL | 'M', insert_newline },
  { CONTROL | 'N', forwline },
  { CONTROL | 'O', openline },
  { CONTROL | 'P', backline },
  { CONTROL | 'Q', quote },
  { CONTROL | 'R', backsearch },
  { CONTROL | 'S', forwsearch },
  { CONTROL | 'T', twiddle },
  { CONTROL | 'U', unarg },
  { CONTROL | 'V', forwpage },
  { CONTROL | 'W', killregion },
  { CONTROL | 'X', cex },
  { CONTROL | 'Y', yank },
  { CONTROL | 'Z', backpage },
  { CONTROL | ']', metafn },
  { CTRLX | CONTROL | 'B', listbuffers },
  { CTRLX | CONTROL | 'C', quit }, /* Hard quit.  */
#if PKCODE & AEDIT
  { CTRLX | CONTROL | 'A', detab },
#endif
#if PKCODE
  { CTRLX | CONTROL | 'D', filesave }, /* Alternative.  */
#else
# if AEDIT
  { CTRLX | CONTROL | 'D', detab },
# endif
#endif
#if AEDIT
  { CTRLX | CONTROL | 'E', entab },
#endif
  { CTRLX | CONTROL | 'F', filefind },
  { CTRLX | CONTROL | 'I', insfile },
  { CTRLX | CONTROL | 'L', lowerregion },
  { CTRLX | CONTROL | 'M', delmode },
  { CTRLX | CONTROL | 'N', mvdnwind },
  { CTRLX | CONTROL | 'O', deblank },
  { CTRLX | CONTROL | 'P', mvupwind },
  { CTRLX | CONTROL | 'R', fileread },
  { CTRLX | CONTROL | 'S', filesave },
#if AEDIT
  { CTRLX | CONTROL | 'T', trim },
#endif
  { CTRLX | CONTROL | 'U', upperregion },
  { CTRLX | CONTROL | 'V', viewfile },
  { CTRLX | CONTROL | 'W', filewrite },
  { CTRLX | CONTROL | 'X', swapmark },
  { CTRLX | CONTROL | 'Z', shrinkwind },
  { CTRLX | '?', deskey },
  { CTRLX | '!', spawn },
  { CTRLX | '@', pipecmd },
  { CTRLX | '#', filter_buffer },
  { CTRLX | '$', execprg },
  { CTRLX | '=', showcpos },
  { CTRLX | '(', ctlxlp },
  { CTRLX | ')', ctlxrp },
  { CTRLX | '^', enlargewind },
  { CTRLX | '0', delwind },
  { CTRLX | '1', onlywind },
  { CTRLX | '2', splitwind },
  { CTRLX | 'A', setvar },
  { CTRLX | 'B', usebuffer },
  { CTRLX | 'C', spawncli },
#if BSD | SVR4
  { CTRLX | 'D', bktoshell },
#endif
  { CTRLX | 'E', ctlxe },
  { CTRLX | 'F', setfillcol },
  { CTRLX | 'K', killbuffer },
  { CTRLX | 'M', setemode },
  { CTRLX | 'N', filename },
  { CTRLX | 'O', nextwind },
  { CTRLX | 'P', prevwind },
#if PKCODE
  { CTRLX | 'Q', quote }, /* Alternative.  */
#endif
#if ISRCH
  { CTRLX | 'R', risearch },
  { CTRLX | 'S', fisearch },
#endif
  { CTRLX | 'W', resize },
  { CTRLX | 'X', nextbuffer },
  { CTRLX | 'Z', enlargewind },
  { META | CONTROL | '?', delbword },
#if WORDPRO
  { META | CONTROL | 'C', wordcount },
#endif
#if PKCODE
  { META | CONTROL | 'D', newsize },
#endif
#if PROC
  { META | CONTROL | 'E', execproc },
#endif
#if CFENCE
  { META | CONTROL | 'F', getfence },
#endif
  { META | CONTROL | 'H', delbword },
  { META | CONTROL | 'K', unbindkey },
  { META | CONTROL | 'L', reposition },
  { META | CONTROL | 'M', delgmode },
  { META | CONTROL | 'N', namebuffer },
  { META | CONTROL | 'R', qreplace },
  { META | CONTROL | 'S', newsize },
  { META | CONTROL | 'T', newwidth },
  { META | CONTROL | 'V', scrnextdw },
#if WORDPRO
  { META | CONTROL | 'W', killpara },
#endif
  { META | CONTROL | 'Z', scrnextup },
  { META | ' ', setmark },
  { META | '?', help },
  { META | '!', reposition },
  { META | '.', setmark },
  { META | '>', gotoeob },
  { META | '<', gotobob },
  { META | '~', unmark },
#if APROP
  { META | 'A', apro },
#endif
  { META | 'B', backword },
  { META | 'C', capword },
  { META | 'D', delfword },
  { META | 'F', forwword },
  { META | 'G', gotoline },
#if PKCODE
# if WORDPRO
  { META | 'J', justpara },
# endif
#endif
  { META | 'K', bindtokey },
  { META | 'L', lowerword },
  { META | 'M', setgmode },
#if WORDPRO
  { META | 'N', gotoeop },
  { META | 'P', gotobop },
  { META | 'Q', fillpara },
#endif
  { META | 'R', sreplace },
#if PKCODE
  { META | 'S', forwhunt },
#else
# if BSD
  { META | 'S', bktoshell },
# endif
#endif
  { META | 'U', upperword },
  { META | 'V', backpage },
  { META | 'W', copyregion },
  { META | 'X', namedcmd },
  { META | 'Z', quickexit },

#if VT220
  { SPEC | '1', gotobob /* Fisearch.  */ }, /* VT220 keys.  */
  { SPEC | '2', yank },
  { SPEC | '3', forwdel /* Killregion.  */ },
  { SPEC | '4', gotoeob /* Setmark.  */ },
  { SPEC | '5', backpage },
  { SPEC | '6', forwpage },
  { SPEC | 'A', backline },
  { SPEC | 'B', forwline },
  { SPEC | 'C', forwchar },
  { SPEC | 'D', backchar },
  { SPEC | 'c', metafn },
  { SPEC | 'd', backchar },
  { SPEC | 'e', forwline },
  { SPEC | 'f', gotobob },
  { SPEC | 'h', help },
  { SPEC | 'i', cex },
#endif

  /* Special internal bindings.  */
  { SPEC | META | 'W', wrapword }, /* Called on word wrap.  */
  { SPEC | META | 'C', nullproc }, /* Called on every command input.  */
  { SPEC | META | 'R', nullproc }, /* Called on file read.  */
  { SPEC | META | 'X', nullproc }, /* Called on window change P.K.  */

  { 0, NULL }
};
