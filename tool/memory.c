// SPDX-License-Identifier: GPL-2.0

#include <stdlib.h>
#include <string.h>

#include "sndhfix/memory.h"
#include "sndhfix/print.h"
#include "sndhfix/types.h"

void *xmalloc(size_t size)
{
	void *p = malloc(size);

	if (!p)
		pr_fatal_errno("malloc\n");

	return p;
}

void *zalloc(size_t size)
{
	void *p = xmalloc(size);

	memset(p, 0, size);

	return p;
}

void *xrealloc(void *ptr, size_t size)
{
	void *p = realloc(ptr, size);

	if (!p)
		pr_fatal_errno("realloc\n");

	return p;
}

void *xmemdup(const void *ptr, size_t size)
{
	void *p = xmalloc(size);

	memcpy(p, ptr, size);

	return p;
}
