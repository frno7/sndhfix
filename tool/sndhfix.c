// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2019 Fredrik Noring
 */

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
#include "sndhfix/sndh-print.h"
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

		sndh_print(file);

		file_free(file);
	}

	return EXIT_SUCCESS;
}
