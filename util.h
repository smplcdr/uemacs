#ifndef _UTIL_H
#define _UTIL_H 1

#include <stddef.h>

#define ARRAY_SIZE(a) (sizeof (a) / sizeof ((a)[0]))

size_t strlcpy (char *__restrict dst, const char *__restrict src, size_t size);

#endif /* _UTIL_H */
