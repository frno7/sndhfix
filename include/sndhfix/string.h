// SPDX-License-Identifier: GPL-2.0

#ifndef SNDHFIX_STRING_H
#define SNDHFIX_STRING_H

#include "sndhfix/types.h"

char *xstrdup(const char *s);

char *xstrcat(const char *a, const char *b);

char *xstrndup(const char *s, size_t n);

bool strtoint(int *n, const char *s, int base);

#endif /* SNDHFIX_STRING_H */
