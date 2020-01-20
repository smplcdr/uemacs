#include "util.h"

#include <string.h>

/*
 * Safe strncpy, the result is always a valid
 * NUL-termintated string that fits int the buffer
 * (unless, of course, the buffer size is zero).
 */
size_t
strscpy (char *dst, const char *src, size_t size)
{
  char *ptr;
  size_t len; /* How many bytes we have to copy.  */

  ptr = memchr (src, '\0', size);
  len = ptr != NULL ? ptr - src : size;

  *((char *) memcpy (dst, src, len) + len) = '\0';

  return ptr != NULL ? len : strlen (src);
}

size_t
strscat (char *dst, const char *src, size_t size)
{
  char *ptr;
  size_t len;

  ptr = memchr (dst, '\0', size);
  len = ptr != NULL ? ptr - dst : size;

  if (size == len)
    return (ptr != NULL ? len : strlen (dst)) + strlen (src);
  else
    return (ptr != NULL ? len : strlen (dst)) + strscpy (dst + len, src, size - len);
}
