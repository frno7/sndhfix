#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "sndhfix/memory.h"
#include "sndhfix/print.h"
#include "sndhfix/string.h"
#include "sndhfix/types.h"

char *xstrdup(const char *s)
{
	void *t = strdup(s);

	if (!t)
		pr_fatal_errno("strdup");

	return t;
}

char *xstrcat(const char *a, const char *b)
{
	const size_t alen = strlen(a);
	const size_t blen = strlen(b);
	char *s = xmalloc(alen + blen + 1);

	memcpy(&s[0], a, alen);
	memcpy(&s[alen], b, blen);
	s[alen + blen] = '\0';

	return s;
}

char *xstrndup(const char *s, size_t n)
{
	void *t = strndup(s, n);

	if (!t)
		pr_fatal_errno("strndup");

	return t;
}

bool strtoint(int *n, const char *s, int base)
{
	char *e;

	errno = 0;
	*n = strtoll(s, &e, base);

	return e == &s[strlen(s)] && errno != ERANGE;
}
