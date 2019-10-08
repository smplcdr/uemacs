#include "util.h"

#include <stdio.h>
#include <string.h>

/*
 * Safe strncpy, the result is always a valid
 * NUL-terminated string that fits in the buffer
 * (unless, of course, the buffer size is zero).
 */
size_t strlcpy(char *dst, const char *src, size_t size)
{
	size_t ret = strlen(src);

	if (size != 0) {
		size_t len = (ret >= size) ? size - 1 : ret;
		memcpy(dst, src, len);
		dst[len] = '\0';
	}
	return ret;
}
