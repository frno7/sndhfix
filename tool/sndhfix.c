// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2019 Fredrik Noring
 */

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "sndhfix/build-bug.h"
#include "sndhfix/compare.h"
#include "sndhfix/file.h"
#include "sndhfix/ice.h"
#include "sndhfix/memory.h"
#include "sndhfix/print.h"
#include "sndhfix/sndh.h"
#include "sndhfix/types.h"
#include "sndhfix/version.h"

char progname[] = "sndhfix";

static struct {
	int verbose;
} option;

int option_verbosity(void)
{
	return option.verbose;
}

static void help(FILE *file)
{
	fprintf(file,
"Usage: %s [options]... <sndh-file>...\n"
"\n"
"General options:\n"
"\n"
"    -h, --help            display this help and exit\n"
"    --version             display version and exit\n"
"    -v, --verbose         increase verbosity\n"
"\n",
		progname);
}

static void NORETURN help_exit(int code)
{
	help(stdout);

	exit(code);
}

static void NORETURN version_exit(void)
{
	printf("%s version %s\n", progname, program_version());

	exit(EXIT_SUCCESS);
}

static void parse_options(int argc, char **argv)
{
	static const struct option options[] = {
{ "help",                  no_argument,       NULL,                    0 },
{ "version",               no_argument,       NULL,                    0 },
{ "verbose",               no_argument,       NULL,                    0 },

{ NULL, 0, NULL, 0 }
	};

#define OPT(option) (strcmp(options[index].name, (option)) == 0)

	argv[0] = progname;	/* For better getopt_long error messages. */

	for (;;) {
		int index = 0;

		switch (getopt_long(argc, argv, "hv:", options, &index)) {
		case -1:
			if (optind == argc)
				help_exit(EXIT_FAILURE);
			return;

		case 0:
			if (OPT("help"))
				help_exit(EXIT_SUCCESS);
			else if (OPT("version"))
				version_exit();
			else if (OPT("verbose"))
				option.verbose++;
			break;

		case 'h':
			help_exit(EXIT_SUCCESS);

		case 'v':
			option.verbose++;
			break;

		case '?':
			exit(EXIT_FAILURE);
		}
	}

#undef OPT
}

struct print_context {
	struct file file;

	int time_count;
	int subname_count;
	int subflag_count;
};

static bool printable(const char *s)
{
	for (size_t i = 0; s[i] != '\0'; i++) {
		int c = s[i] & 0xff;

		if (c < 0x20 || 0x7f == c)
			return false;
	}

	return true;
}

static void print_flags(const char *flags, struct file file)
{
	for (size_t i = 0; flags[i] != '\0'; i++)
		switch (flags[i]) {
#define PRINT_SNDH_FLAG(c, label)					\
		case c: printf(" " #label); break;
SNDH_FLAG(PRINT_SNDH_FLAG)
		default:
			pr_warn("%s: unrecognised flag '%c'\n",
				file.path, flags[i]);
		}
}

static bool print_tag(const char *name, const char *value, void *arg)
{
	struct print_context *context = arg;
	size_t length = strlen(value);

	if (!length)
		pr_warn("%s: tag %s empty\n", context->file.path, name);
	if (!printable(value))
		pr_warn("%s: tag %s contains nonprintable characters: %s\n",
			context->file.path, name, value);
	if (length && (isspace(value[0]) || isspace(value[length - 1])))
		pr_warn("%s: tag %s malformed whitespace\n",
			context->file.path, name);

	if (strcmp(name, "TIME") == 0) {
		printf("tag field %s %d %s\n",
			name, ++context->time_count, value);
	} else if (strcmp(name, "!#SN") == 0) {
		printf("tag field %s %d %s\n",
			name, ++context->subname_count, value);
	} else if (strcmp(name, "FLAG~") == 0) {	// FIXME: doc/sndhv21.txt
		printf("tag field FLAG ~ %s", value);
		print_flags(value, context->file);
		printf("\n");
	} else if (strcmp(name, "FLAG") == 0) {
		printf("tag field %s %d %s",
			name, ++context->subflag_count, value);
		print_flags(value, context->file);
		printf("\n");
	} else
		printf("tag field %s%s%s\n",
			name, name[0] != '\0' ? " " : "", value);

	return true;
}

int main(int argc, char *argv[])
{
	parse_options(argc, argv);

	for (int arg = optind; arg < argc; arg++) {
		const char *input = argv[arg];

		struct file file = file_read_or_stdin(input);
		if (!file_valid(file)) {
			pr_errno("%s\n", input);
			continue;
		}

		if (ice_identify(file.data, file.size)) {
			size_t s = ice_decrunched_size(file.data, file.size);
			void *b = xmalloc(s);

			file.size = ice_decrunch(b, file.data, file.size);
			free(file.data);
			file.data = b;
		} else if (option_verbosity())
			pr_warn("%s: not ICE packed\n", file.path);

		struct print_context context = {
			.file = file,
		};

		printf("path %s\n", file.path);
		sndh_tags(file, print_tag, &context);

		file_free(file);
	}

	return EXIT_SUCCESS;
}
