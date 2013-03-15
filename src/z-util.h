#ifndef INCLUDED_Z_UTIL_H
#define INCLUDED_Z_UTIL_H

#include "h-basic.h"


/**** Available variables ****/

/**
 * The name of the program.
 */
extern char *argv0;


/* Aux functions */
extern void (*plog_aux)(cptr);
extern void (*quit_aux)(cptr);


/**** Available Functions ****/

/**
 * Case insensitive comparison between two strings
 */
extern int my_stricmp(const char *s1, const char *s2);

/**
 * Case insensitive comparison between two strings, up to n characters long.
 */
extern int my_strnicmp(cptr a, cptr b, int n);

/**
 * Case-insensitive strstr
 */
extern char *my_stristr(const char *string, const char *pattern);

/**
 * Copy up to 'bufsize'-1 characters from 'src' to 'buf' and NULL-terminate
 * the result.  The 'buf' and 'src' strings may not overlap.
 *
 * Returns: strlen(src).  This makes checking for truncation
 * easy.  Example:
 *   if (my_strcpy(buf, src, sizeof(buf)) >= sizeof(buf)) ...;
 *
 * This function should be equivalent to the strlcpy() function in BSD.
 */
extern size_t my_strcpy(char *buf, const char *src, size_t bufsize);

/**
 * Try to append a string to an existing NULL-terminated string, never writing
 * more characters into the buffer than indicated by 'bufsize', and
 * NULL-terminating the buffer.  The 'buf' and 'src' strings may not overlap.
 *
 * my_strcat() returns strlen(buf) + strlen(src).  This makes checking for
 * truncation easy.  Example:
 *   if (my_strcat(buf, src, sizeof(buf)) >= sizeof(buf)) ...;
 *
 * This function should be equivalent to the strlcat() function in BSD.
 */
extern size_t my_strcat(char *buf, const char *src, size_t bufsize);

/* Test equality, prefix, suffix */
extern bool streq(cptr s, cptr t);
extern bool prefix(cptr s, cptr t);
extern bool suffix(cptr s, cptr t);

#define streq(s, t)		(!strcmp(s, t))


/* Print an error message */
extern void plog(cptr str);

/* Exit, with optional message */
extern void quit(cptr str);

/* Sorting functions */
/* TODO: make ang_sort() take comp and swap hooks rather than use globals */
extern bool (*ang_sort_comp)(const void *u, const void *v, int a, int b);
extern void (*ang_sort_swap)(void *u, void *v, int a, int b);
extern void ang_sort(void *u, void *v, int n);

/* Mathematical functions */
int mean(int *nums, int size);
int variance(int *nums, int size);

/* Concatenate two strings (fast version) */
extern char *my_fast_strcat(char *buf, char *end, const char *src, size_t bufsize);


#endif /* INCLUDED_Z_UTIL_H */
