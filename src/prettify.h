#ifndef PRETTIFY_H
#define PRETTIFY_H

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>

#define DPRINTF(fmt, ...) \
do { if (debug) fprintf(stderr, "[debug] " fmt , ## __VA_ARGS__); } while (0)

#define	FLAG_ANALYZE		0x01
#define FLAG_FIX_TABS		0x02
#define FLAG_FIX_SPACES		0x04
#define FLAG_FIX_AUTO		0x08
#define	FLAG_FIX_CONSISTENCY	0x10
#define	FLAG_DEBUG		0x20
#define	FLAG_FORCE		0x40
#define	FLAG_SILENT		0x80

#define	EXIT_INCONSISTENCY	2

int debug;
int silent;

typedef struct tLeadingInfo {
	int line;
	int numSpaces;
	int numTabs;
} tLeadingInfo;

tLeadingInfo *lc;
int numLeadingInfo;
char *infile;
char *outfile;
#endif
