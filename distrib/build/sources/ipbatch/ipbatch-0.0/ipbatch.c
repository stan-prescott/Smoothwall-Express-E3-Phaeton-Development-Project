/*
 * Author: Ludwig Nussel <ludwig.nussel@suse.de>
 * Update for iptables 1.4.3.x: Petr Uzel <petr.uzel@suse.cz>
 *
 * Based on the ipchains code by Paul Russell and Michael Neuling
 *
 * (C) 2000-2002 by the netfilter coreteam <coreteam@netfilter.org>:
 * 		    Paul 'Rusty' Russell <rusty@rustcorp.com.au>
 * 		    Marc Boucher <marc+nf@mbsi.ca>
 * 		    James Morris <jmorris@intercode.com.au>
 * 		    Harald Welte <laforge@gnumonks.org>
 * 		    Jozsef Kadlecsik <kadlec@blackhole.kfki.hu>
 *
 *	iptables-batch -- iptables batch processor
 *
 *	See the accompanying manual page iptables(8) for information
 *	about proper usage of this program.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#ifdef IP6T
#include <ip6tables.h>
#else
#include <iptables.h>
#endif
#include <xtables.h>

#ifdef IP6T
#define prog_name ip6tables_globals.program_name
#define prog_ver ip6tables_globals.program_version
#else
#define prog_name iptables_globals.program_name
#define prog_ver iptables_globals.program_version
#endif

static char* errstr = NULL;

static unsigned current_line = 0;

static char*
skipspace(char* ptr)
{
	while(*ptr && isspace(*ptr))
		++ptr;
	return ptr;
}

static char*
getliteral(char** ptr)
{
	char* start = *ptr;
	char* p = start;

	while(*p && !isspace(*p))
		++p;

	if(*p)
	{
		*p = '\0';
		++p;
	}

	*ptr = p;
	return start;
}

static char*
getstring(char** ptr)
{
	char* start = *ptr+1; // skip leading "
	char* p = start;
	char* o = start;
	int backslash = 0;
	int done = 0;

	while(*p && !done)
	{
		if(backslash)
		{
			backslash = 0;
			// no escapes supported, just eat the backslash
			*o++ = *p++;
		}
		else if(*p == '\\')
		{
			backslash = 1;
			p++;
		}
		else if(*p == '"')
		{
			done = 1;
		}
		else
		{
			*o++ = *p++;
		}
	}

	if(done)
	{
		*o = '\0';
		*p = '\0';
		++p;
		*ptr = p;
	}
	else
	{
		errstr = "missing \" at end of string";
		start = NULL;
	}
	return start;
}

// this is just a very basic method, not 100% shell compatible
static char*
getword(char** ptr)
{
	*ptr = skipspace(*ptr);
	if(**ptr == '"')
		return getstring(ptr);
	return getliteral(ptr);
}

// destructive
static int
tokenize(int* argc, char* argv[], size_t nargvsize, char* iline)
{
	char* ptr = skipspace(iline);
	int ret = 0;
	char* word;

	if (strncmp(ptr, "/sbin/", 6) == 0)
		ptr += 6;

	while(ptr && *ptr)
	{
		if(*ptr == '#')
			break;
		if(*argc >= nargvsize)
		{
			errstr = "too many arguments";
			ret = -1;
			break;
		}
		word = getword(&ptr);
		if(!word)
		{
			ret = -1;
			break;
		}
		argv[(*argc)++] = word;
		++ret;
	}
	return ret;
}

#ifdef DEBUG
static void
dumpargv(int argc, char* argv[])
{
	int i;
	for(i=0; i < argc; ++i)
	{
		printf("%s\"%s\"",i?" ":"", argv[i]);
	}
	puts("");
}
#endif

struct table_handle
{
	char* name;
#ifdef IP6T
	struct ip6tc_handle *handle;
#else
	struct iptc_handle *handle;
#endif
};

static struct table_handle* tables = NULL;
static unsigned num_tables;
struct table_handle* current_table;

static void
alloc_tables(void)
{
	tables = realloc(tables, sizeof(struct table_handle) * num_tables);
}

static void
set_current_table(const char* name)
{
	unsigned i;

	if(!strcmp(name, current_table->name)) // same as last time?
		return;

	for(i = 0; i < num_tables; ++i) // find already known table
	{
		if(!strcmp(name, tables[i].name))
		{
			current_table = &tables[i];
			return;
		}
	}

	// table name not known, create new
	i = num_tables++;
	alloc_tables();
	current_table = &tables[i];
	current_table->name = strdup(name);
	current_table->handle = NULL;
}

static int
find_table(int argc, char* argv[])
{
	int i;
	for(i = 0; i < argc; ++i)
	{
		if(!strcmp(argv[i], "-t") || !strcmp(argv[i], "--table"))
		{
			++i;
			if(i >= argc)
			{
				fprintf(stderr, "line %d: missing table name after %s\n",
						current_line, argv[i]);
				return 0;
			}
			set_current_table(argv[i]);
			return 1;
		}
	}

	// no -t specified
	set_current_table("filter");

	return 1;
}

static int
do_iptables(int argc, char* argv[])
{
	char *table = "filter";
	int ret = 0;

	if(!find_table(argc, argv))
		return 0;

#ifdef IP6T
	ret = do_command6(argc, argv, &table, &current_table->handle);

	if (!ret)
	{
		fprintf(stderr, "line %d: %s\n", current_line, ip6tc_strerror(errno));
	}
	else
	{
		if(!table || strcmp(table, current_table->name))
		{
			fprintf(stderr, "line %d: expected table %s, got %s\n",
					current_line, current_table->name, table);
			exit(1);
		}
	}
#else
	ret = do_command(argc, argv, &table, &current_table->handle);

	if (!ret)
	{
		fprintf(stderr, "line %d: %s\n", current_line, iptc_strerror(errno));
	}
	else
	{
		if(!table || strcmp(table, current_table->name))
		{
			fprintf(stderr, "line %d: expected table %s, got %s\n",
					current_line, current_table->name, table);
			exit(1);
		}
	}
#endif

	return ret;
}

static int
do_commit(void)
{
	unsigned i;
	int ret = 1;

	for(i = 0; i < num_tables; ++i)
	{
		if(tables[i].handle)
		{
#ifdef IP6T
			ret = ip6tc_commit(tables[i].handle);
			if (!ret)
				fprintf(stderr, "commit failed on table %s: %s\n", tables[i].name, ip6tc_strerror(errno));
			ip6tc_free(tables[i].handle);
			tables[i].handle = NULL;
#else
			ret = iptc_commit(tables[i].handle);
			if (!ret)
				fprintf(stderr, "commit failed on table %s: %s\n", tables[i].name, iptc_strerror(errno));
			iptc_free(tables[i].handle);
			tables[i].handle = NULL;
#endif
		}
	}

	return ret;
}

static void
help(void)
{
	fprintf(stderr, "Usage: %s [FILE]\n\n", prog_name);
	puts("Read iptables commands from FILE, commit them at EOF\n");
	puts("In addition to normal iptables calls the commands");
	puts("'commit' and 'exit' are understood.");
	exit(0);
}

int
main(int argc, char *argv[])
{
	int ret = 1;
	int c;
	int numtok;
	size_t llen = 0;
	char* iline = NULL;
	ssize_t r = -1;
	int nargc = 0;
	char* nargv[256];
	FILE* fp = stdin;

#ifdef IP6T
	prog_name = "ip6tables-batch";
#else
	prog_name = "iptables-batch";
#endif

#ifdef IP6T
	c = xtables_init_all(&ip6tables_globals, NFPROTO_IPV6);
#else
	c = xtables_init_all(&iptables_globals, NFPROTO_IPV4);
#endif

	if(c < 0) {
		fprintf(stderr, "%s/%s Failed to initialize xtables\n",
				prog_name,
				prog_ver);
		exit(1);
	}

#ifdef NO_SHARED_LIBS
	init_extensions();
#endif
	if(argc > 1)
	{
		if(!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h"))
		{
			help();
		}
		else if(strcmp(argv[1], "-"))
		{
			fp = fopen(argv[1], "r");
			if(!fp)
			{
				perror("fopen");
				exit(1);
			}
		}
	}

	num_tables = 4;
	alloc_tables();
	tables[0].name = "filter";
	tables[0].handle = NULL;
	tables[1].name = "mangle";
	tables[1].handle = NULL;
	tables[2].name = "nat";
	tables[2].handle = NULL;
	tables[3].name = "raw";
	tables[3].handle = NULL;
	current_table = &tables[0];

	while((r = getline(&iline, &llen, fp)) != -1)
	{
		if(llen < 1 || !*iline)
			continue;
		if (strncmp(iline, "end", 3) == 0)
			break;
		if(iline[strlen(iline)-1] == '\n')
			iline[strlen(iline) -1 ] = '\0';

		++current_line;
		nargc = 0;
		errstr = NULL;
		numtok = tokenize(&nargc, nargv, (sizeof(nargv)/sizeof(nargv[0])), iline);
		if(numtok == -1)
		{
		}
		else if (numtok == 0)
		{
			continue;
		}
		else if(nargc < 1)
		{
			errstr = "insufficient number of arguments";
		}

		if(errstr)
		{
			fprintf(stderr, "parse error in line %d: %s\n", current_line, errstr);
			ret = 0;
			break;
		}

#ifdef DEBUG
		dumpargv(nargc, nargv);
#endif

#ifdef IP6T
		if(!strcmp(nargv[0], "ip6tables"))
#else
		if(!strcmp(nargv[0], "iptables"))
#endif
		{
			ret = do_iptables(nargc, nargv);
			if(!ret) break;
		}
		else if(!strcmp(nargv[0], "exit"))
		{
			break;
		}
		else if(!strcmp(nargv[0], "commit"))
		{
			/* do nothing - see bnc#500990, comment #16 */
		}
		else
		{
			fprintf(stderr, "line %d: invalid command '%s'\n", current_line, nargv[0]);
		}
	}

	if(ret)
		ret = do_commit();

	exit(!ret);
}
