/* isa.h -- isletter, islower, isupper, flipcase */

#ifndef _ISA_H
#define _ISA_H 1

#ifdef islower
# undef islower
#endif

#ifdef isupper
# undef isupper
#endif

#define NATIONL 0 /* if 1, interpret [,],\,{,},| as characters P.K.  */
#if NATIONL
# define LASTUL ']'
# define LASTLL '}'
#else
# define LASTUL 'Z'
# define LASTLL 'z'
#endif

/* DIFCASE represents the integer difference between upper and lower
   case letters. It is an xor-able value, which is fortunate, since the
   relative positions of upper to lower case letters is the opposite of
   ascii in ebcdic.  */
#define DIFCASE ('a' - 'A')
/* Toggle the case of a letter.  */
#define flipcase(c) ((c) ^ DIFCASE)

#if !defined(__GNUC__) && !defined(__clang__) && !defined(__TINYC__)
# define isletter(c) __isxletter (((c) & 0xFF))
# define islower(c)    isxlower (((c) & 0xFF))
# define isupper(c)    isxupper (((c) & 0xFF))

# define __isxletter(c) \
  (('a' <= c && LASTLL >= c) || ('A' <= c && LASTUL >= c) \
   || (192 <= c /* && c <= 255 */))
# define isxlower(c) (('a' <= c && LASTLL >= c) || (224 <= c && 252 >= c))
# define isxupper(c) (('A' <= c && LASTUL >= c) || (192 <= c && 220 >= c))
#else
#include <limits.h>

static bool
__isdigit (unsigned char c)
{
  __extension__ static const bool table[UCHAR_MAX] =
  {
    ['0' ... '9'] = TRUE
  };

  return table[c];
}
static bool
__isletter (unsigned char c)
{
  __extension__ static const bool table[UCHAR_MAX] =
  {
# if NATIONL
    [ '[', ']', '\\', '{', '}', '|'] = TRUE,
# endif
    ['A' ... 'Z'] = TRUE,
    ['a' ... 'z'] = TRUE
  };

  return table[c];
}
static bool
__islower (unsigned char c)
{
  __extension__ static const bool table[UCHAR_MAX] =
  {
# if NATIONL
    [ '[', ']', '\\', '{', '}', '|'] = TRUE,
# endif
    ['a' ... 'z'] = TRUE
  };

  return table[c];
}
static bool
__isupper (unsigned char c)
{
  __extension__ static const bool table[UCHAR_MAX] =
  {
# if NATIONL
    [ '[', ']', '\\', '{', '}', '|'] = TRUE,
# endif
    ['A' ... 'Z'] = TRUE
  };

  return table[c];
}

# define isletter(c) __isletter (c)
# define islower(c)  __islower (c)
# define isupper(c)  __isupper (c)
# define isdigit(c)  __isdigit (c)
# define isalnum(c)  (isletter (c) || isdigit (c))
#endif




#endif /* _ISA_H */

/* end of isa.h */
