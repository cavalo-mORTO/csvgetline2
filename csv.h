/* Copyright (C) 1999 Lucent Technologies */
/* Excerpted from 'The Practice of Programming' */
/* by Brian W. Kernighan and Rob Pike */

/* csv.h: interface for csv library */

extern char *csvgetline(FILE *f); /* read next input line */
extern char *csvfield(int n);	  /* return field n */
extern int csvnfield(void);		  /* return number of fields */
extern char *csvkey(char *k);		/* return field with name k */
extern void csvclose(void);		  /* cleanup after we're done */
extern void csvkeys(void);      /* print all the keys */
