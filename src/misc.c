/*
 * This file is part of John the Ripper password cracker,
 * Copyright (c) 1996-99,2003 by Solar Designer
 *
 * ...with changes in the jumbo patch, by JimF and magnum.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted.
 *
 * There's ABSOLUTELY NO WARRANTY, express or implied.
 */

#include <stdio.h>
#define NEED_OS_FORK
#include "os.h"
#if (!AC_BUILT || HAVE_UNISTD_H) && !_MSC_VER
#include <unistd.h>
#endif
#ifdef _MSC_VER
#include <io.h>
#pragma warning ( disable : 4996 )
#endif
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

#include "logger.h"
#include "params.h"
#include "misc.h"
#include "options.h"

#ifdef HAVE_MPI
#include "john-mpi.h"
#endif
#include "memdbg.h"

void error(void)
{
#ifndef _JOHN_MISC_NO_LOG
	log_event("Terminating on error");
	log_done();
#endif

	exit(1);
}

void pexit(char *format, ...)
{
	va_list args;

#if defined(HAVE_MPI) && !defined(_JOHN_MISC_NO_LOG)
	if (mpi_p > 1)
		fprintf(stderr, "%u@%s: ", mpi_id + 1, mpi_name);
	else
#elif OS_FORK && !defined(_JOHN_MISC_NO_LOG)
	if (options.fork)
		fprintf(stderr, "%u: ", options.node_min);
#endif

	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);

	fprintf(stderr, ": %s\n", strerror(errno));

	error();
}

int write_loop(int fd, const char *buffer, int count)
{
	int offset, block;

	offset = 0;
	while (count > 0) {
		block = write(fd, &buffer[offset], count);

/* If any write(2) fails, we consider that the entire write_loop() has
 * failed to do its job, unless we were interrupted by a signal. */
		if (block < 0) {
			if (errno == EINTR) continue;
			return block;
		}

		offset += block;
		count -= block;
	}

/* Should be equal to the requested size, unless our kernel got crazy. */
	return offset;
}

char *fgetl(char *s, int size, FILE *stream)
{
	char *res, *pos;
	int c;

	if ((res = fgets(s, size, stream))) {
		if (!*res) return res;

		pos = res + strlen(res) - 1;
		if (*pos == '\n') {
			*pos = 0;
			if (pos > res)
			if (*--pos == '\r') *pos = 0;
		} else
		if ((c = getc(stream)) == '\n') {
			if (*pos == '\r') *pos = 0;
		} else
		while (c != EOF && c != '\n')
			c = getc(stream);
	}

	return res;
}

char *strnfcpy(char *dst, const char *src, int size)
{
	char *dptr = dst;

	while (size--)
		if (!(*dptr++ = *src++)) break;

	return dst;
}

char *strnzcpy(char *dst, const char *src, int size)
{
	char *dptr = dst;

	if (size)
		while (--size)
			if (!(*dptr++ = *src++)) return dst;
	*dptr = 0;

	return dst;
}

int strnzcpyn(char *dst, const char *src, int size)
{
	char *dptr;
	if (!size) return 0;

	dptr = dst;

	while (--size)
		if (!(*dptr++ = *src++)) return (dptr-dst)-1;
	*dptr = 0;

	return (dptr-dst)-1;
}

char *strnzcat(char *dst, const char *src, int size)
{
	char *dptr = dst;

	if (size) {
		while (size && *dptr) {
			size--; dptr++;
		}
		if (size)
			while (--size)
				if (!(*dptr++ = *src++)) break;
	}
	*dptr = 0;

	return dst;
}
