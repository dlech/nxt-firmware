/*
 * Copyright (C) 2010 Nicolas Schodet
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/* NXT source code is using sscanf to parse a float.  Newlib sscanf will pull
 * too many code, so here is a stub which implement just what is used. */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

int
sscanf (const char *str, const char *fmt, ...)
{
    va_list ap;
    float *f;
    char *tailptr;
    /* Only support use in NXT source code. */
    if (fmt[0] != '%' || fmt[1] != 'f' || fmt[2] != '\0')
	return 0;
    /* Retrieve float pointer. */
    va_start (ap, fmt);
    f = va_arg (ap, float *);
    va_end (ap);
    /* Parse using the nice strtod. */
    *f = strtod (str, &tailptr);
    if (str == tailptr)
	return 0;
    else
	return 1;
}

