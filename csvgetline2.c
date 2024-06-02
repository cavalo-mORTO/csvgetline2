/* Copyright (C) 1999 Lucent Technologies */
/* Excerpted from 'The Practice of Programming' */
/* by Brian W. Kernighan and Rob Pike */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "csv.h"

enum { NOMEM = -2 };          /* out of memory signal */

static char *line    = NULL;  /* input chars */
static char *sline   = NULL;  /* line copy used by split */
static char *kline   = NULL;  /* line copy used by split to store keys */
static int  maxline  = 0;     /* size of line[] and sline[] and kline[] */
static char **field  = NULL;  /* field pointers */
static char **key    = NULL;  /* key pointers */
static int  maxfield = 0;     /* size of field[] */
static int  maxkey   = 0;     /* size of key[] */
static int  nfield   = 0;     /* number of fields in field[] */
static int  nkey     = 0;     /* number of keys in key[] */
static int  nrow     = 0;     /* number of rows */

static char fieldsep[] = ","; /* field separator chars */

static char *advquoted(char *);
static int split(int *, int *, char **, char ***);

/* endofline: check for and consume \r, \n, \r\n, or EOF */
static int endofline(FILE *fin, int c)
{
	int eol;

	eol = (c=='\r' || c=='\n');
	if (c == '\r') {
		c = getc(fin);
		if (c != '\n' && c != EOF)
			ungetc(c, fin);	/* read too far; put c back */
	}
	return eol;
}

/* reset: set variables back to starting values */
static void reset(void)
{
	free(line);	/* free(NULL) permitted by ANSI C */
	free(sline);
	free(kline);
	free(field);
	free(key);
	line = NULL;
	sline = NULL;
	kline = NULL;
	field = NULL;
	key = NULL;
	maxline = maxfield = nfield = nrow = nkey = maxkey = 0;
}

/* csvgetline:  get one line, grow as needed */
/* sample input: "LU",86.25,"11/4/1998","2:19PM",+4.0625 */
char *csvgetline(FILE *fin)
{	
	int i, c, err;
	char *newl, *news, *newk;

	if (line == NULL) {			/* allocate on first call */
		maxline = maxfield = maxkey = 1;
		line = (char *) malloc(maxline);
		sline = (char *) malloc(maxline);
		kline = (char *) malloc(maxline);
		field = (char **) malloc(maxfield*sizeof(field[0]));
		key = (char **) malloc(maxkey*sizeof(key[0]));
		if (line == NULL || sline == NULL || kline == NULL || field == NULL || key == NULL) {
			reset();
			return NULL;		/* out of memory */
		}
	}
	for (i=0; (c=getc(fin))!=EOF && !endofline(fin,c); i++) {
		if (i >= maxline-1) {	/* grow line */
			maxline *= 2;		/* double current size */
			newl = (char *) realloc(line, maxline);
			if (newl == NULL) {
				reset();
				return NULL;
			}
			line = newl;
			news = (char *) realloc(sline, maxline);
			if (news == NULL) {
				reset();
				return NULL;
			}
			sline = news;
			if (nrow == 0) {
				newk = (char *) realloc(kline, maxline);
				if (newk == NULL) {
					reset();
					return NULL;
				}
				kline = newk;
			}
		}
		line[i] = c;
	}
	line[i] = '\0';

	if (nrow == 0)
		err = split(&nkey, &maxkey, &kline, &key);
	else
		err = split(&nfield, &maxfield, &sline, &field);
	if (err == NOMEM) {  /* out of memory */
		reset();
		return NULL;
	}

	nrow++;
	return (c == EOF && i == 0) ? NULL : line;
}

/* split: split line into fields */
static int split(int *n, int *max, char **l, char ***ap)
{
	char *p, **new;
	char *sepp; /* pointer to temporary separator character */
	int sepc;   /* temporary separator character */

	*n = 0;
	if (line[0] == '\0')
		return 0;
	strcpy(*l, line);
	p = *l;

	do {
		if (*n >= *max) {
			*max *= 2;			/* double current size */
			new = (char **) realloc(*ap, *max * sizeof(*ap[0]));
			if (new == NULL)
				return NOMEM;
			*ap = new;
		}

		if (*p == '"')
			sepp = advquoted(++p);	/* skip initial quote */
		else
			sepp = p + strcspn(p, fieldsep);
		sepc = sepp[0];
		sepp[0] = '\0';				/* terminate field */
		(*ap)[(*n)++] = p;
		p = sepp + 1;
	} while (sepc == ',');

	return *n;
}

/* advquoted: quoted field; return pointer to next separator */
static char *advquoted(char *p)
{
	int i, j;

	for (i = j = 0; p[j] != '\0'; i++, j++) {
		if (p[j] == '"' && p[++j] != '"') {
			/* copy up to next separator or \0 */
			int k = strcspn(p+j, fieldsep);
			memmove(p+i, p+j, k);
			i += k;
			j += k;
			break;
		}
		p[i] = p[j];
	}
	p[i] = '\0';
	return p + j;
}

/* csvfield:  return pointer to n-th field */
char *csvfield(int n)
{
	if (n < 0 || n >= nfield)
		return NULL;
	return field[n];
}

/* csvkey: return pointer to field with name k */
char *csvkey(char *k)
{
	int max = nkey < nfield ? nkey : nfield;
	for (int i = 0; i < max; i++) {
		if (!strcmp(key[i], k))
			return field[i];
	}
	return NULL;
}

/* csvkeys: print all the keys */
void csvkeys(void)
{
	printf("Keys: ");
	for (int i = 0; i < nkey; i++) {
		printf("%s ", key[i]);
	}
	printf("\n");
}

/* csvnfield:  return number of fields */ 
int csvnfield(void)
{
	return nfield;
}

/* csvclose: cleanup after we're done */
void csvclose(void)
{
	reset();
}

/* csvtest main: test CSV library */
int csvtest(void)
{
	int i;
	char *line;

	while ((line = csvgetline(stdin)) != NULL) {
		printf("line = `%s'\n", line);
		for (i = 0; i < csvnfield(); i++)
			printf("field[%d] = `%s'\n", i, csvfield(i));
	}
	csvclose();
	return 0;
}
