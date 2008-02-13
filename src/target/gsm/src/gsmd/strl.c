/* safe strcpy and strcat versions
 *
 * Copyright (C) 1991, 1992  Linus Torvalds
 * Copyright (C) 2008 by Paulius Zaleckas, JSC Teltonika
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <string.h>

#include "gsmd.h"
#include <gsmd/gsmd.h>

/**
 * strlcpy - Copy a %NUL terminated string into a sized buffer
 * @dest: Where to copy the string to
 * @src: Where to copy the string from
 * @size: size of destination buffer
 *
 * Compatible with *BSD: the result is always a valid
 * NUL-terminated string that fits in the buffer (unless,
 * of course, the buffer size is zero). It does not pad
 * out the result like strncpy() does.
 */
size_t strlcpy(char *dest, const char *src, size_t size)
{
	size_t ret = strlen(src);

	if (size) {
		size_t len;
		if (ret >= size) {
			len = size - 1;
			DEBUGP("\"%s\" was truncated by %i characters\n", src,
			       ret - len);
		}
		else
			len = ret;
		memcpy(dest, src, len);
		dest[len] = '\0';
	}
	return ret;
}

/**
 * strlcat - Append a length-limited, %NUL-terminated string to another
 * @dest: The string to be appended to
 * @src: The string to append to it
 * @count: The size of the destination buffer.
 */
size_t strlcat(char *dest, const char *src, size_t count)
{
	size_t dsize = strlen(dest);
	size_t len = strlen(src);
	size_t res = dsize + len;

	/* This would be a bug */
	if (dsize >= count) {
		DEBUGP("Length of destination string > provided buffer size!\n");
		return 0;
	}

	dest += dsize;
	count -= dsize;
	if (len >= count) {
		len = count - 1;
		DEBUGP("\"%s\" was truncated by %i characters\n", src,
		       res - len);
	}
	memcpy(dest, src, len);
	dest[len] = 0;
	return res;
}

