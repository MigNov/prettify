#include "config.h"

#include "prettify.h"

void add_leading_info(int line, int spaces, int tabs)
{
	if (lc == NULL) {
		lc = (tLeadingInfo *)malloc( sizeof(tLeadingInfo) );
		numLeadingInfo = 0;
	}
	else
		lc = (tLeadingInfo *)realloc( lc, (numLeadingInfo + 1) * sizeof(tLeadingInfo) );

	lc[numLeadingInfo].line = line;
	lc[numLeadingInfo].numSpaces = spaces;
	lc[numLeadingInfo].numTabs = tabs;
	numLeadingInfo++;
}

void free_leading_info(void)
{
	free(lc);
	lc = NULL;
	numLeadingInfo = 0;
}

void dump_leading_info(void)
{
	int i;

	printf("Leading information entries dump\n");
	for (i = 0; i < numLeadingInfo; i++)
		printf("\tEntry #%d:\n\t\tLine: %d\n\t\tLeading spaces: %d\n\t\tLeading tabs: %d\n",
			i + 1, lc[i].line, lc[i].numSpaces, lc[i].numTabs);

	printf("\n");
}

int is_valid_start_char(int c)
{
	return (c == ' ') || (c == '\t');
}

int parse_file(char *filename)
{
	char str[1024] = { 0 };
	int leading_spaces = 0;
	int leading_tabs = 0;
	int diffchar = 0;
	int i, line = 1;

	free_leading_info();

	DPRINTF("%s: Opening file '%s' for reading\n", __FUNCTION__, filename);
	FILE *fp = fopen(filename, "r");
	if (fp == NULL) {
		DPRINTF("%s: Cannot open file '%s' for reading\n", __FUNCTION__, filename);
		return -EIO;
	}

	while (!feof(fp)) {
		memset(str, 0, sizeof(str));
		fgets(str, 1024, fp);

		leading_spaces = 0;
		leading_tabs = 0;
		diffchar = 0;
		if ((strlen(str) > 0) && (is_valid_start_char(str[0]))) {
			for (i = 0; i < strlen(str); i++) {
				if ((str[i] == ' ') && (!diffchar))
					leading_spaces++;
				else
				if ((str[i] == '\t') && (!diffchar))
					leading_tabs++;
				else
					diffchar = 1;
			}

			DPRINTF("%s: Adding leading information to structure = { %d, %d, %d }\n",
				__FUNCTION__, line, leading_spaces, leading_tabs);
			add_leading_info(line, leading_spaces, leading_tabs);
		}

		line++;
	}

	fclose(fp);
	return 0;
}

int analyse_file(char *filename)
{
	int bad = -1;
	int numSpaces = 0;
	int numTabs = 0;
	int badSpaces = 0;
	int badTabs = 0;
	int ret = EXIT_SUCCESS;

	parse_file(filename);

	if (debug) {
		printf("Analysis results for filename %s follows\n", filename);
		dump_leading_info();
	}

	numSpaces = find_expected_leading_char_count(1, &badSpaces);
	numTabs   = find_expected_leading_char_count(0, &badTabs);

	if ((badSpaces > 0) || (badTabs > 0)) {
		if (!silent)
			fprintf(stderr, "Validation failed: Inconsistency on spaces or tabs\n");
		ret = EXIT_INCONSISTENCY;
	}
	else
	if ((numSpaces > 0) && (numTabs > 0)) {
		if (!silent)
			fprintf(stderr, "Validation failed: Mixing spaces and tabs together\n");
		ret = EXIT_FAILURE;
	}

	if ((ret == EXIT_SUCCESS) && (!silent))
		fprintf(stderr, "Validation successful\n");

	return ret;
}

int has_line_info(int line)
{
	int i;

	for (i = 0; i < numLeadingInfo; i++) {
		if (lc[i].line == line)
			return 1;
	}

	return 0;
}

int get_line_info(int line, int get_tabs)
{
	int i;

	DPRINTF("%s: Getting line %d information on %s\n", __FUNCTION__,
		line, (get_tabs ? "tabs" : "spaces") );
	for (i = 0; i < numLeadingInfo; i++) {
		if (lc[i].line == line) {
			if (get_tabs && (lc[i].numTabs > 0))
				return lc[i].numTabs;
			if (!get_tabs && (lc[i].numSpaces > 0))
				return lc[i].numSpaces;
		}
	}

	return 0;
}

int get_min_consistent_number(void)
{
	int i;

	for (i = 0; i < numLeadingInfo; i++) {
		if (lc[i].numSpaces > 0)
			return lc[i].numSpaces;
	}

	return -1;
}

int have_space_majority(void)
{
	int i, retS, retT;

	retS = 0;
	retT = 0;
	for (i = 0; i < numLeadingInfo; i++) {
		if ((lc[i].numSpaces > 0)
			&& (lc[i].numTabs == 0))
			retS++;

		if ((lc[i].numSpaces == 0)
			&& (lc[i].numTabs > 0))
			retT++;
	}

	return (retS > retT) ? 1 : 0;
}

int find_expected_leading_char_count(int find_spaces, int *anything_bad)
{
	int i;
	int reqNum = -1;
	int num = 0;
	int bad = 0;

	for (i = 0; i < numLeadingInfo; i++) {
		if (find_spaces) {
			if ((lc[i].numSpaces > 0)
				&& (reqNum == -1)) {
				reqNum = lc[i].numSpaces;
				continue;
			}
			if (lc[i].numSpaces > 0) {
				if (lc[i].numSpaces % reqNum != 0) {
					bad++;
					if (!silent)
						fprintf(stderr, "WARNING: Inconsistency on line %d: number of spaces is %d, should be %d\n",
							lc[i].line, lc[i].numSpaces, reqNum);
				}
			}
		}
		else {
			if ((lc[i].numTabs > 0)
				&& (reqNum == -1)) {
				reqNum = lc[i].numTabs;
				continue;
			}
			if (lc[i].numTabs > 0) {
				if (lc[i].numTabs % reqNum != 0) {
					bad++;
					if (!silent)
						fprintf(stderr, "WARNING: Inconsistency on line %d: number of spaces is %d, should be %d\n",
							lc[i].line, lc[i].numSpaces, reqNum);
				}
			}
		}
	}

	if (anything_bad != NULL)
		*anything_bad = bad;

	return reqNum;
}

char *fix_line(int line, char *str, int rnum, int spaces)
{
	int orig_cnt = -1;
	int newsize = 0;
	int prefix_size = 0;
	int min_num = 0;
	int i = 0;
	char *out = NULL;

	if ((str != NULL) && (strlen(str) > 1)) {
		if ((str[i] == ' ') && (spaces)) {
			DPRINTF("%s: Line %d already has leading spaces\n", __FUNCTION__, line);
			return str;
		}
		if ((str[i] == '\t') && (!spaces)) {
			DPRINTF("%s: Line %d already has leading tabs\n", __FUNCTION__, line);
			return str;
		}
	}

	DPRINTF("%s: Going to fix line %d, new line leader should be %d %s\n", __FUNCTION__,
		line, rnum, spaces ? "space(s)" : "tab(s)");

	orig_cnt = get_line_info(line, spaces);
	DPRINTF("%s: Original line leading character count is %d\n", __FUNCTION__, orig_cnt);
	if (spaces)
		prefix_size = orig_cnt * rnum;
	else {
		min_num = get_min_consistent_number();
		prefix_size = orig_cnt / min_num;
	}

	DPRINTF("%s: Prefix size is %d\n", __FUNCTION__, prefix_size);

	newsize = strlen(str) - orig_cnt + prefix_size;

	DPRINTF("%s: New size is %d\n", __FUNCTION__, newsize);

	out = (char *)malloc( (newsize + 1) * sizeof(char) );
	memset(out, 0, newsize + 1);

	if (prefix_size == 0) {
		if (!silent)
			fprintf(stderr, "WARNING: Prefix size equals zero for line %d. Overriding to 1\n", line);
		prefix_size = 1;
	}

	for (i = 0; i < prefix_size; i++)
		strcat(out, spaces ? " " : "\t");

	strcat(out, str + orig_cnt);

	return out;
}

int fix_file(char *filename1, char *filename2, int to_spaces)
{
	int rc;
	FILE *fp1;
	FILE *fp2;
	int line = 1;
	int num  = 0;
	int rnum = 0;
	char str[1024];

	DPRINTF("%s: Fixing file %s to %s. Converting to %s.\n", __FUNCTION__, filename1, filename2,
		(to_spaces == -1) ? "<autodetect>" : ((to_spaces == 1) ? "spaces" : "tabs"));

	if ((rc = parse_file(filename1)) != 0)
		return rc;

	DPRINTF("%s: File parsing phase done successfully\n", __FUNCTION__);

	if ((have_space_majority()) && (to_spaces == -1)) {
		DPRINTF("%s: Majority of the code seems to be using spaces. Overriding to use them too\n",
			__FUNCTION__);
		to_spaces = 1;
	}
	else
	if ((!have_space_majority()) && (to_spaces == -1)) {
		DPRINTF("%s: Majority of the code seems to be using tabs. Overriding to use them too\n",
			__FUNCTION__);
		to_spaces = 0;
	}

	/* If the code writer mixed for spaces and tabs then we override to -1 for autodetection */
	if ((find_expected_leading_char_count(0, NULL) > 0)
		&& (find_expected_leading_char_count(1, NULL) > 0)) {
		DPRINTF("%s: Code writer mixed both spaces and tabs, overriding to autodetection\n",
			__FUNCTION__);
		rnum = -1;
	}
	else {
		rnum = find_expected_leading_char_count(to_spaces, NULL);
		DPRINTF("%s: Found expected number of leading characters, it's %d\n", __FUNCTION__, rnum);
	}

	/* Override to 8 spaces or 1 tab if none set */
	if ((rnum == -1) && (to_spaces)) {
		rnum = 8;
		DPRINTF("%s: Overriding number of characters to %d\n", __FUNCTION__, rnum);
	}
	if ((rnum == -1) && (!to_spaces)) {
		rnum = 1;
		DPRINTF("%s: Overriding number of leading tabs to %d\n", __FUNCTION__, rnum);
	}

	fp1 = fopen(filename1, "r");
	if (fp1 == NULL) {
		DPRINTF("%s: Cannot open file '%s' for reading\n", __FUNCTION__, filename1);
		return -EIO;
	}

	fp2 = fopen(filename2, "w");
	if (fp2 == NULL) {
		DPRINTF("%s: Cannot open file '%s' for writing\n", __FUNCTION__, filename2);
		return -EPERM;
	}

	while (!feof(fp1)) {
		memset(str, 0, sizeof(str));
		fgets(str, sizeof(str), fp1);

		if (has_line_info(line)) {
			char *tmp = fix_line(line, str, rnum, to_spaces);
			if (tmp != NULL)
				strncpy(str, tmp, sizeof(str));
			else
				fprintf(stderr, "WARNING: Cannot fix line %d\n", line);
		}

		fputs(str, fp2);

		line++;
	}

	fclose(fp2);
	fclose(fp1);

	DPRINTF("%s: All done successfully\n", __FUNCTION__);
	return 0;
}

void usage(char *name)
{
	fprintf(stderr, "C Code Prettifier\n");
	fprintf(stderr, "Written by Michal Novotny in 2012\n\n");
	fprintf(stderr, "Syntax: %s --input <filename.c> [--output <filename-out.c>] [--analyze] [--fix-type <type>] [--debug] [--force]\n",
			name);

	fprintf(stderr, "Type could be either 'spaces', 'tabs', 'consistency' or 'auto' for automatic detection of format. Defaults to 'auto'.\n");
	fprintf(stderr, "Force parameter will force overwriting the outfile if the file exists\n\n");
}

long parseArgs(int argc, char * const argv[]) {
	int option_index = 0, c;
	unsigned int retVal = 0;
	struct option long_options[] = {
		{"input", 1, 0, 'i'},
		{"output", 1, 0, 'o'},
		{"analyze", 0, 0, 'a'},
		{"fix-type", 1, 0, 't'},
		{"debug", 0, 0, 'g'},
		{"force", 0, 0, 'f'},
		{"silent", 0, 0, 's'},
		{0, 0, 0, 0}
	};

	char *optstring = "i:o:at:gs";

	while (1) {
		c = getopt_long(argc, argv, optstring,
			long_options, &option_index);
		if (c == -1)
			break;

		switch (c) {
			case 'i':
				infile = optarg;
				break;
			case 'o':
				outfile = optarg;
				break;
			case 'a':
				retVal |= FLAG_ANALYZE;
				break;
			case 't':
				if (strcmp(optarg, "tabs") == 0)
					retVal |= FLAG_FIX_TABS;
				else
				if (strcmp(optarg, "spaces") == 0)
					retVal |= FLAG_FIX_SPACES;
				else
				if (strcmp(optarg, "auto") == 0)
					retVal |= FLAG_FIX_AUTO;
				else
				if (strcmp(optarg, "consistency") == 0)
					retVal |= FLAG_FIX_CONSISTENCY;
				else {
					usage(argv[0]);
					fprintf(stderr, "ERROR: Invalid fix-type option '%s'\n", optarg);
					fprintf(stderr, "Valid options are: tabs, spaces, consistency, auto\n");
					return 1;
				}
				break;
			case 'f':
				retVal |= FLAG_FORCE;
				break;
			case 'g':
				retVal |= FLAG_DEBUG;
				break;
			case 's':
				retVal |= FLAG_SILENT;
				break;
			default:
				usage(argv[0]);
		}
	}

	return retVal;
}

void initialize(void)
{
	debug=0;
	silent=0;
	numLeadingInfo = 0;
	lc = NULL;
	infile = NULL;
	outfile = NULL;
}

int main(int argc, char *argv[])
{
	unsigned int flags = 0;
	int val = -1;

	initialize();

	flags = parseArgs(argc, argv);
	if (((infile != NULL) && ((outfile == NULL) && (flags & FLAG_ANALYZE == 0))) || (infile == NULL)) {
		usage(argv[0]);
		return EXIT_FAILURE;
	}

	debug = flags & FLAG_DEBUG;
	silent = flags & FLAG_SILENT;

	if (debug) {
		printf("Parameters:\n\tInput file: %s\n\tOutput file: %s\n\tAnalyze: %s\n\tFix type: %s\n\tForce: %s\n\n",
			infile, outfile, flags & FLAG_ANALYZE ? "True" : "False",
			(flags & FLAG_FIX_AUTO) ? "auto" : ((flags & FLAG_FIX_TABS) ? "tabs" :
				((!(flags & FLAG_FIX_AUTO)) && (!(flags & FLAG_FIX_TABS)) && (!(flags & FLAG_FIX_SPACES))) ? "auto" :
				((flags & FLAG_FIX_CONSISTENCY) ? "consistency" : "spaces") ),
			flags & FLAG_FORCE ? "True" : "False");
	}

	if ((outfile != NULL) && (access(outfile, F_OK) == 0)) {
		if (!(flags & FLAG_FORCE)) {
			int c;

			printf("File '%s' already exists. Do you want to overwrite it? (y/N) ",
				outfile);

			c = getchar();
			if ((c != 'Y') && (c != 'y'))
				return EXIT_SUCCESS;
		}
	}

	if (flags & FLAG_FIX_CONSISTENCY) {
		int orig_spaces;
		char tmpfile[] = "/tmp/ccpXXXXXX";

		parse_file(infile);
		orig_spaces = have_space_majority();

		DPRINTF("%s: File '%s' have %s used\n", __FUNCTION__, infile, (orig_spaces ? "spaces" : "tabs"));
		mkstemp(tmpfile);
		DPRINTF("%s: Converting %s to %s\n", __FUNCTION__, infile, tmpfile);
		fix_file(infile, tmpfile, orig_spaces ? 0 : 1);

		DPRINTF("%s: Reverse converting %s to %s\n", __FUNCTION__, tmpfile, outfile);
		fix_file(tmpfile, outfile, orig_spaces);

		unlink(tmpfile);
		return 0;
	}

	if (flags & FLAG_ANALYZE)
		return analyse_file(infile);

	if (flags & FLAG_FIX_AUTO)
		val = -1;
	else
	if (flags & FLAG_FIX_TABS)
		val = 0;
	else
		val = 1;

	return fix_file(infile, outfile, val);
}

