// SPDX-License-Identifier: GPL-2.0

#ifndef SNDHFIX_MEMORY_H
#define SNDHFIX_MEMORY_H

#include "sndhfix/types.h"

void *xmalloc(size_t size);

void *zalloc(size_t size);

void *xrealloc(void *ptr, size_t size);

void *xmemdup(const void *ptr, size_t size);

#endif /* SNDHFIX_MEMORY_H */
