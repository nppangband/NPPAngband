/*
 * File: z-util.c
 * Purpose: Low-level string handling and other utilities.
 *
 * Copyright (c) 1997-2005 Ben Harrison, Robert Ruehlmann.
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband licence":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */
#include "z-util.h"
#include "z-form.h"


/*
 * Convenient storage of the program name
 */
char *argv0 = NULL;


/*
 * Case insensitive comparison between two strings
 */
int my_stricmp(const char *s1, const char *s2)
{
	char ch1 = 0;
	char ch2 = 0;

	/* Just loop */
	while (TRUE)
	{
		/* We've reached the end of both strings simultaneously */
		if ((*s1 == 0) && (*s2 == 0))
		{
			/* We're still here, so s1 and s2 are equal */
			return (0);
		}

		ch1 = toupper((unsigned char) *s1);
		ch2 = toupper((unsigned char) *s2);

		/* If the characters don't match */
		if (ch1 != ch2)
		{
			/* return the difference between them */
			return ((int)(ch1 - ch2));
		}

		/* Step on through both strings */
		s1++;
		s2++;
	}

	/*Just to avoid compiler warnings about not having a return*/
	return (TRUE);
}


/*
 * Case insensitive comparison between the first n characters of two strings
 */
int my_strnicmp(cptr a, cptr b, int n)
{
	cptr s1, s2;
	char z1, z2;

	/* Scan the strings */
	for (s1 = a, s2 = b; n > 0; s1++, s2++, n--)
	{
		z1 = toupper((unsigned char)*s1);
		z2 = toupper((unsigned char)*s2);
		if (z1 < z2) return (-1);
		if (z1 > z2) return (1);
		if (!z1) return (0);
	}

	return 0;
}


/*
 * An ANSI version of strstr() with case insensitivity.
 *
 * In the public domain; found at:
 *    http://c.snippets.org/code/stristr.c
 */
char *my_stristr(const char *string, const char *pattern)
{
	const char *pptr, *sptr;
	char *start;

	for (start = (char *)string; *start != 0; start++)
	{
		/* find start of pattern in string */
		for ( ; ((*start != 0) &&
			    (toupper((unsigned char)*start) != toupper((unsigned char)*pattern))); start++)
				;

		if (*start == 0)
			return NULL;

		pptr = (const char *)pattern;
		sptr = (const char *)start;

		while (toupper((unsigned char)*sptr) == toupper((unsigned char)*pptr))
		{
			sptr++;
			pptr++;

			/* if end of pattern then pattern was found */
			if (*pptr == 0)
				return (start);
		}
	}

	return NULL;
}


/*
 * The my_strcpy() function copies up to 'bufsize'-1 characters from 'src'
 * to 'buf' and NUL-terminates the result.  The 'buf' and 'src' strings may
 * not overlap.
 *
 * my_strcpy() returns strlen(src).  This makes checking for truncation
 * easy.  Example: if (my_strcpy(buf, src, sizeof(buf)) >= sizeof(buf)) ...;
 *
 * This function should be equivalent to the strlcpy() function in BSD.
 */
size_t my_strcpy(char *buf, const char *src, size_t bufsize)
{
	size_t len = strlen(src);
	size_t ret = len;

	/* Paranoia */
	if (bufsize == 0) return ret;

	/* Truncate */
	if (len >= bufsize) len = bufsize - 1;

	/* Copy the string and terminate it */
	(void)memcpy(buf, src, len);
	buf[len] = '\0';

	/* Return strlen(src) */
	return ret;
}


/*
 * The my_strcat() tries to append a string to an existing NUL-terminated string.
 * It never writes more characters into the buffer than indicated by 'bufsize' and
 * NUL-terminates the buffer.  The 'buf' and 'src' strings may not overlap.
 *
 * my_strcat() returns strlen(buf) + strlen(src).  This makes checking for
 * truncation easy.  Example:
 * if (my_strcat(buf, src, sizeof(buf)) >= sizeof(buf)) ...;
 *
 * This function should be equivalent to the strlcat() function in BSD.
 */
size_t my_strcat(char *buf, const char *src, size_t bufsize)
{
	size_t dlen = strlen(buf);

	/* Is there room left in the buffer? */
	if (dlen < bufsize - 1)
	{
		/* Append as much as possible  */
		return (dlen + my_strcpy(buf + dlen, src, bufsize - dlen));
	}
	else
	{
		/* Return without appending */
		return (dlen + strlen(src));
	}
}


/*
 * Determine if string "a" is equal to string "b"
 */
#undef streq
bool streq(cptr a, cptr b)
{
	return (!strcmp(a, b));
}


/*
 * Determine if string "t" is a suffix of string "s"
 */
bool suffix(cptr s, cptr t)
{
	size_t tlen = strlen(t);
	size_t slen = strlen(s);

	/* Check for incompatible lengths */
	if (tlen > slen) return (FALSE);

	/* Compare "t" to the end of "s" */
	return (!strcmp(s + slen - tlen, t));
}


/*
 * Determine if string "t" is a prefix of string "s"
 */
bool prefix(cptr s, cptr t)
{
	/* Scan "t" */
	while (*t)
	{
		/* Compare content and length */
		if (*t++ != *s++) return (FALSE);
	}

	/* Matched, we have a prefix */
	return (TRUE);
}


/*
 * Redefinable "plog" action
 */
void (*plog_aux)(cptr) = NULL;


/*
 * Print (or log) a "warning" message (ala "perror()")
 * Note the use of the (optional) "plog_aux" hook.
 */
void plog(cptr str)
{
	/* Use the "alternative" function if possible */
	if (plog_aux) (*plog_aux)(str);

	/* Just do a labeled fprintf to stderr */
	else (void)(fprintf(stderr, "%s: %s\n", argv0 ? argv0 : "?", str));
}


/*
 * Redefinable "quit" action
 */
void (*quit_aux)(cptr) = NULL;

/*
 * Exit (ala "exit()").  If 'str' is NULL, do "exit(EXIT_SUCCESS)".
 * Otherwise, plog() 'str' and exit with an error code of -1.
 * But always use 'quit_aux', if set, before anything else.
 */
void quit(cptr str)
{
	/* Attempt to use the aux function */
	if (quit_aux) (*quit_aux)(str);

	/* Success */
	if (!str)
	{
		exit(EXIT_SUCCESS);
	}

	/* Send the string to plog() */
	plog(str);

	/* Failure */
	exit(EXIT_FAILURE);
}


/* Compare and swap hooks */
bool (*ang_sort_comp)(const void *u, const void *v, int a, int b);
void (*ang_sort_swap)(void *u, void *v, int a, int b);


/*
 * Angband sorting algorithm -- quick sort in place
 *
 * Note that the details of the data we are sorting is hidden,
 * and we rely on the "ang_sort_comp()" and "ang_sort_swap()"
 * function hooks to interact with the data, which is given as
 * two pointers, and which may have any user-defined form.
 */
static void ang_sort_aux(void *u, void *v, int p, int q)
{
	int z, a, b;

	/* Done sort */
	if (p >= q) return;

	/* Pivot */
	z = p;

	/* Begin */
	a = p;
	b = q;

	/* Partition */
	while (TRUE)
	{
		/* Slide i2 */
		while (!(*ang_sort_comp)(u, v, b, z)) b--;

		/* Slide i1 */
		while (!(*ang_sort_comp)(u, v, z, a)) a++;

		/* Done partition */
		if (a >= b) break;

		/* Swap */
		(*ang_sort_swap)(u, v, a, b);

		/* Advance */
		a++, b--;
	}

	/* Recurse left side */
	ang_sort_aux(u, v, p, b);

	/* Recurse right side */
	ang_sort_aux(u, v, b+1, q);
}


/*
 * Angband sorting algorithm -- quick sort in place
 *
 * Note that the details of the data we are sorting is hidden,
 * and we rely on the "ang_sort_comp()" and "ang_sort_swap()"
 * function hooks to interact with the data, which is given as
 * two pointers, and which may have any user-defined form.
 */
void ang_sort(void *u, void *v, int n)
{
	/* Sort the array */
	ang_sort_aux(u, v, 0, n-1);
}

/* Arithmetic mean of the first 'size' entries of the array 'nums' */
int mean(int *nums, int size)
{
	int i, total = 0;

	for(i = 0; i < size; i++) total += nums[i];

	return total / size;
}


/* Variance of the first 'size' entries of the array 'nums'  */
int variance(int *nums, int size)
{
	int i, avg, total = 0;

	avg = mean(nums, size);

	for(i = 0; i < size; i++)
	{
		int delta = nums[i] - avg;
		total += delta * delta;
	}

	return total / size;
}


/*
 * Fast string concatenation.
 * Append the "src" string to "buf" given the address of the trailing null
 * character of "buf" in "end". "end" can be NULL, in which the trailing null
 * character is fetched from the beginning of "buf".
 * "bufsize" is the maximum size of "buf" (including the trailing null character).
 * It returns the -new- address of the trailing null character of "buf".
 *
 * Example of usage:
 *
 * char buf[100] = "", *end;
 * int i;
 *
 * end = my_fast_strcat(buf, NULL, "START", sizeof(buf));
 *
 * for (i = 0; i < 5; i++)
 * {
 * 	end = my_fast_strcat(buf, end, "_", sizeof(buf));
 * }
 *
 * end = my_fast_strcat(buf, end, "END", sizeof(buf));
 *
 * buf ==> "START_____END"
 */
char *my_fast_strcat(char *buf, char *end, const char *src, size_t bufsize)
{
	/* No end, go to the beginning of "buf" */
	if (end == NULL) end = buf;

	/* Find the trailing null character, if necessary */
	while (*end) ++end;

	/* Make room for the trailing null character, if possible */
	if (bufsize > 0) --bufsize;

	/* Append "str" to "buf", if possible */
	while (*src && ((size_t)(end - buf) < bufsize)) *end++ = *src++;

	/* Terminate the string */
	*end = '\0';

	/* Return the new end of "buf" */
	return end;
}

