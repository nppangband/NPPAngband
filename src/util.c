/*
 * File: util.c
 * Purpose: Macro code, gamma correction, some high-level UI functions, inkey()
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 * 						Jeff Greene, Diego Gonzalez
 *
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
#include "angband.h"
#include "game-event.h"


/*
 * Convert a decimal to a single digit hex number
 */
static char hexify(int i)
{
	return (hexsym[i % 16]);
}


/*
 * Convert a hexadecimal-digit into a decimal
 */
static int dehex(char c)
{
	if (isdigit((unsigned char)c)) return (D2I(c));
	if (isalpha((unsigned char)c)) return (A2I(tolower((unsigned char)c)) + 10);
	return (0);
}


/*
 * Transform macro trigger name ('\[alt-D]' etc..)
 * into macro trigger key code ('^_O_64\r' or etc..)
 */
static size_t trigger_text_to_ascii(char *buf, size_t max, cptr *strptr)
{
	cptr str = *strptr;
	bool mod_status[MAX_MACRO_MOD];

	int i, len = 0;
	int shiftstatus = 0;
	cptr key_code;

	size_t current_len = strlen(buf);

	/* No definition of trigger names */
	if (macro_template == NULL) return 0;

	/* Initialize modifier key status */
	for (i = 0; macro_modifier_chr[i]; i++)
		mod_status[i] = FALSE;

	str++;

	/* Examine modifier keys */
	while (TRUE)
	{
		/* Look for modifier key name */
		for (i = 0; macro_modifier_chr[i]; i++)
		{
			len = strlen(macro_modifier_name[i]);

			if (!my_strnicmp(str, macro_modifier_name[i], len))
				break;
		}

		/* None found? */
		if (!macro_modifier_chr[i]) break;

		/* Proceed */
		str += len;

		/* This modifier key is pressed */
		mod_status[i] = TRUE;

		/* Shift key might be going to change keycode */
		if (macro_modifier_chr[i] == 'S')
			shiftstatus = 1;
	}

	/* Look for trigger name */
	for (i = 0; i < max_macrotrigger; i++)
	{
		len = strlen(macro_trigger_name[i]);

		/* Found it and it is ending with ']' */
		if (!my_strnicmp(str, macro_trigger_name[i], len) && (']' == str[len]))
			break;
	}

	/* Invalid trigger name? */
	if (i == max_macrotrigger)
	{
		/*
		 * If this invalid trigger name is ending with ']',
		 * skip whole of it to avoid defining strange macro trigger
		 */
		str = strchr(str, ']');

		if (str)
		{
			strnfcat(buf, max, &current_len, "\x1F\r");

			*strptr = str; /* where **strptr == ']' */
		}

		return current_len;
	}

	/* Get keycode for this trigger name */
	key_code = macro_trigger_keycode[shiftstatus][i];

	/* Proceed */
	str += len;

	/* Begin with '^_' */
	strnfcat(buf, max, &current_len, "\x1F");

	/* Write key code style trigger using template */
	for (i = 0; macro_template[i]; i++)
	{
		char ch = macro_template[i];

		switch (ch)
		{
			/* Modifier key character */
			case '&':
			{
				size_t j;
				for (j = 0; macro_modifier_chr[j]; j++)
				{
					if (mod_status[j])
						strnfcat(buf, max, &current_len, "%c", macro_modifier_chr[j]);
				}
				break;
			}

			/* Key code */
			case '#':
			{
				strnfcat(buf, max, &current_len, "%s", key_code);
				break;
			}

			/* Fixed string */
			default:
			{
				strnfcat(buf, max, &current_len, "%c", ch);
				break;
			}
		}
	}

	/* End with '\r' */
	strnfcat(buf, max, &current_len, "\r");

	/* Succeed */
	*strptr = str; /* where **strptr == ']' */

	return current_len;
}


/*
 * Hack -- convert a printable string into real ascii
 *
 * This function will not work on non-ascii systems.
 *
 * To be safe, "buf" should be at least as large as "str".
 */
void text_to_ascii(char *buf, size_t len, cptr str)
{
	char *s = buf;

	/* Analyze the "ascii" string */
	while (*str)
	{
		/* Check if the buffer is long enough */
		if (s >= buf + len - 1) break;

		/* Backslash codes */
		if (*str == '\\')
		{
			str++;
			if (*str == '\0') break;

			switch (*str)
			{
				/* Macro trigger */
				case '[':
				{
					/* Terminate before appending the trigger */
					*s = '\0';
					s += trigger_text_to_ascii(buf, len, &str);
					break;
				}

				/* Hex-mode */
				case 'x':
				{
					if (isxdigit((unsigned char)(*(str + 1))) &&
					    isxdigit((unsigned char)(*(str + 2))))
					{
						*s = 16 * dehex(*++str);
						*s++ += dehex(*++str);
					}
					else
					{
						/* HACK - Invalid hex number */
						*s++ = '?';
					}
					break;
				}

				case 'e':
					*s++ = ESCAPE;
					break;
				case 's':
					*s++ = ' ';
					break;
				case 'b':
					*s++ = '\b';
					break;
				case 'n':
					*s++ = '\n';
					break;
				case 'r':
					*s++ = '\r';
					break;
				case 't':
					*s++ = '\t';
					break;
				case 'a':
					*s++ = '\a';
					break;
				case '\\':
					*s++ = '\\';
					break;
				case '^':
					*s++ = '^';
					break;

				default:
					*s = *str;
					break;
			}

			/* Skip the final char */
			str++;
		}

		/* Normal Control codes */
		else if (*str == '^')
		{
			str++;
			if (*str == '\0') break;

			*s++ = KTRL(*str);
			str++;
		}

		/* Normal chars */
		else
		{
			*s++ = *str++;
		}
	}

	/* Terminate */
	*s = '\0';
}


/*
 * Transform macro trigger key code ('^_O_64\r' or etc..)
 * into macro trigger name ('\[alt-D]' etc..)
 */
static size_t trigger_ascii_to_text(char *buf, size_t max, cptr *strptr)
{
	cptr str = *strptr;
	char key_code[100];
	int i;
	cptr tmp;
	size_t current_len = strlen(buf);

	/* No definition of trigger names */
	if (macro_template == NULL) return 0;

	/* Trigger name will be written as '\[name]' */
	strnfcat(buf, max, &current_len, "\\[");

	/* Use template to read key-code style trigger */
	for (i = 0; macro_template[i]; i++)
	{
		char ch = macro_template[i];

		switch (ch)
		{
			/* Read modifier */
			case '&':
			{
				size_t j;
				while ((tmp = strchr(macro_modifier_chr, *str)) != 0)
				{
					j = tmp - macro_modifier_chr;
					strnfcat(buf, max, &current_len, "%s", macro_modifier_name[j]);
					str++;
				}
				break;
			}

			/* Read key code */
			case '#':
			{
				size_t j;
				for (j = 0; *str && (*str != '\r') && (j < sizeof(key_code) - 1); j++)
					key_code[j] = *str++;
				key_code[j] = '\0';
				break;
			}

			/* Skip fixed strings */
			default:
			{
				if (ch != *str) return 0;
				str++;
				break;
			}
		}
	}

	/* Key code style triggers always end with '\r' */
	if (*str++ != '\r') return 0;

	/* Look for trigger name with given keycode (normal or shifted keycode) */
	for (i = 0; i < max_macrotrigger; i++)
	{
		if (!my_stricmp(key_code, macro_trigger_keycode[0][i]) ||
		    !my_stricmp(key_code, macro_trigger_keycode[1][i]))
			break;
	}

	/* Not found? */
	if (i == max_macrotrigger) return 0;

	/* Write trigger name + "]" */
	strnfcat(buf, max, &current_len, "%s]", macro_trigger_name[i]);

	/* Succeed */
	*strptr = str;
	return current_len;
}


/*
 * Hack -- convert a string into a printable form
 *
 * This function will not work on non-ascii systems.
 */
void ascii_to_text(char *buf, size_t len, cptr str)
{
	char *s = buf;

	/* Analyze the "ascii" string */
	while (*str)
	{
		byte i = (byte)(*str++);

		/* Check if the buffer is long enough */
		/* HACK - always assume worst case (hex-value + '\0') */
		if (s >= buf + len - 5) break;

		if (i == ESCAPE)
		{
			*s++ = '\\';
			*s++ = 'e';
		}
		else if (i == ' ')
		{
			*s++ = '\\';
			*s++ = 's';
		}
		else if (i == '\b')
		{
			*s++ = '\\';
			*s++ = 'b';
		}
		else if (i == '\t')
		{
			*s++ = '\\';
			*s++ = 't';
		}
		else if (i == '\a')
		{
			*s++ = '\\';
			*s++ = 'a';
		}
		else if (i == '\n')
		{
			*s++ = '\\';
			*s++ = 'n';
		}
		else if (i == '\r')
		{
			*s++ = '\\';
			*s++ = 'r';
		}
		else if (i == '\\')
		{
			*s++ = '\\';
			*s++ = '\\';
		}
		else if (i == '^')
		{
			*s++ = '\\';
			*s++ = '^';
		}
		/* Macro Trigger */
		else if (i == 31)
		{
			size_t offset;

			/* Terminate before appending the trigger */
			*s = '\0';

			offset = trigger_ascii_to_text(buf, len, &str);

			if (offset == 0)
			{
				/* No trigger found */
				*s++ = '^';
				*s++ = '_';
			}
			else
				s += offset;
		}
		else if (i < 32)
		{
			*s++ = '^';
			*s++ = UN_KTRL(i);
		}
		else if (i < 127)
		{
			*s++ = i;
		}
		else
		{
			*s++ = '\\';
			*s++ = 'x';
			*s++ = hexify((int)i / 16);
			*s++ = hexify((int)i % 16);
		}
	}

	/* Terminate */
	*s = '\0';
}


/*----- Roman numeral functions  ------*/

/*
 * Find the start of a possible Roman numerals suffix by going back from the
 * end of the string to a space, then checking that all the remaining chars
 * are valid Roman numerals.
 *
 * Return the start position, or NULL if there isn't a valid suffix.
 */
char *find_roman_suffix_start(cptr buf)
{
	const char *start = strrchr(buf, ' ');
	const char *p;

	if (start)
	{
		start++;
		p = start;
		while (*p)
		{
			if (*p != 'I' && *p != 'V' && *p != 'X' && *p != 'L' &&
			    *p != 'C' && *p != 'D' && *p != 'M')
			{
				start = NULL;
				break;
			}
			++p;
		}
	}
	return (char *)start;
}


/*
 * Converts an arabic numeral (int) to a roman numeral (char *).
 *
 * An arabic numeral is accepted in parameter `n`, and the corresponding
 * upper-case roman numeral is placed in the parameter `roman`.  The
 * length of the buffer must be passed in the `bufsize` parameter.  When
 * there is insufficient room in the buffer, or a roman numeral does not
 * exist (e.g. non-positive integers) a value of 0 is returned and the
 * `roman` buffer will be the empty string.  On success, a value of 1 is
 * returned and the zero-terminated roman numeral is placed in the
 * parameter `roman`.
 */
int int_to_roman(int n, char *roman, size_t bufsize)
{
	/* Roman symbols */
	char roman_symbol_labels[13][3] =
		{"M", "CM", "D", "CD", "C", "XC", "L", "XL", "X", "IX",
		 "V", "IV", "I"};
	int  roman_symbol_values[13] =
		{1000, 900, 500, 400, 100, 90, 50, 40, 10, 9, 5, 4, 1};

	/* Clear the roman numeral buffer */
	roman[0] = '\0';

	/* Roman numerals have no zero or negative numbers */
	if (n < 1)
		return 0;

	/* Build the roman numeral in the buffer */
	while (n > 0)
	{
		int i = 0;

		/* Find the largest possible roman symbol */
		while (n < roman_symbol_values[i])
			i++;

		/* No room in buffer, so abort */
		if (strlen(roman) + strlen(roman_symbol_labels[i]) + 1
			> bufsize)
			break;

		/* Add the roman symbol to the buffer */
		my_strcat(roman, roman_symbol_labels[i], bufsize);

		/* Decrease the value of the arabic numeral */
		n -= roman_symbol_values[i];
	}

	/* Ran out of space and aborted */
	if (n > 0)
	{
		/* Clean up and return */
		roman[0] = '\0';

		return 0;
	}

	return 1;
}


/*
 * Converts a roman numeral (char *) to an arabic numeral (int).
 *
 * The null-terminated roman numeral is accepted in the `roman`
 * parameter and the corresponding integer arabic numeral is returned.
 * Only upper-case values are considered. When the `roman` parameter
 * is empty or does not resemble a roman numeral, a value of -1 is
 * returned.
 *
 * XXX This function will parse certain non-sense strings as roman
 *     numerals, such as IVXCCCVIII
 */
int roman_to_int(const char *roman)
{
	size_t i;
	int n = 0;
	char *p;

	char roman_token_chr1[] = "MDCLXVI";
	const char* roman_token_chr2[] = {0, 0, "DM", 0, "LC", 0, "VX"};

	int roman_token_vals[7][3] = {{1000},
	                              {500},
	                              {100, 400, 900},
	                              {50},
	                              {10, 40, 90},
	                              {5},
	                              {1, 4, 9}};

	if (strlen(roman) == 0)
		return -1;

	/* Check each character for a roman token, and look ahead to the
	   character after this one to check for subtraction */
	for (i = 0; i < strlen(roman); i++)
	{
		char c1, c2;
		int c1i, c2i;

		/* Get the first and second chars of the next roman token */
		c1 = roman[i];
		c2 = roman[i + 1];

		/* Find the index for the first character */
		p = strchr(roman_token_chr1, c1);
		if (p)
		{
			c1i = p - roman_token_chr1;
		} else {
			return -1;
		}

		/* Find the index for the second character */
		c2i = 0;
		if (roman_token_chr2[c1i] && c2)
		{
			p = strchr(roman_token_chr2[c1i], c2);
			if (p)
			{
				c2i = (p - roman_token_chr2[c1i]) + 1;
				/* Two-digit token, so skip a char on the next pass */
				i++;
			}
		}

		/* Increase the arabic numeral */
		n += roman_token_vals[c1i][c2i];
	}

	return n;
}


/*
 * The "macro" package
 *
 * Functions are provided to manipulate a collection of macros, each
 * of which has a trigger pattern string and a resulting action string
 * and a small set of flags.
 */


/*
 * Determine if any macros have ever started with a given character.
 */
static bool macro__use[256];


/*
 * Find the macro (if any) which exactly matches the given pattern
 */
int macro_find_exact(cptr pat)
{
	int i;

	/* Nothing possible */
	if (!macro__use[(byte)(pat[0])])
		return -1;

	/* Scan the macros */
	for (i = 0; i < macro__num; ++i)
	{
		if (streq(macro__pat[i], pat))
			return i;
	}

	/* No matches */
	return -1;
}


/*
 * Find the first macro (if any) which contains the given pattern
 */
static int macro_find_check(cptr pat)
{
	int i;

	/* Nothing possible */
	if (!macro__use[(byte)(pat[0])])
		return -1;

	/* Scan the macros */
	for (i = 0; i < macro__num; ++i)
	{
		if (prefix(macro__pat[i], pat))
			return i;
	}

	/* Nothing */
	return -1;
}


/*
 * Find the first macro (if any) which contains the given pattern and more
 */
static int macro_find_maybe(cptr pat)
{
	int i;

	/* Nothing possible */
	if (!macro__use[(byte)(pat[0])])
		return -1;

	/* Scan the macros */
	for (i = 0; i < macro__num; ++i)
	{
		if (prefix(macro__pat[i], pat) && !streq(macro__pat[i], pat))
			return i;
	}

	/* Nothing */
	return -1;
}


/*
 * Find the longest macro (if any) which starts with the given pattern
 */
static int macro_find_ready(cptr pat)
{
	int i, t, n = -1, s = -1;

	/* Nothing possible */
	if (!macro__use[(byte)(pat[0])])
		return -1;

	/* Scan the macros */
	for (i = 0; i < macro__num; ++i)
	{
		/* Skip macros which are not contained by the pattern */
		if (!prefix(pat, macro__pat[i])) continue;

		/* Obtain the length of this macro */
		t = strlen(macro__pat[i]);

		/* Only track the "longest" pattern */
		if ((n >= 0) && (s > t)) continue;

		/* Track the entry */
		n = i;
		s = t;
	}

	/* Result */
	return n;
}


/*
 * Add a macro definition (or redefinition).
 *
 * We should use "act == NULL" to "remove" a macro, but this might make it
 * impossible to save the "removal" of a macro definition.  XXX XXX XXX
 *
 * We should consider refusing to allow macros which contain existing macros,
 * or which are contained in existing macros, because this would simplify the
 * macro analysis code.  XXX XXX XXX
 *
 * We should consider removing the "command macro" crap, and replacing it
 * with some kind of "powerful keymap" ability, but this might make it hard
 * to change the "roguelike" option from inside the game.  XXX XXX XXX
 */
errr macro_add(cptr pat, cptr act)
{
	int n;

	if (!pat || !act) return (-1);

	/* Look for any existing macro */
	n = macro_find_exact(pat);

	/* Replace existing macro */
	if (n >= 0)
	{
		string_free(macro__act[n]);
	}

	/* Create a new macro */
	else
	{
		/* Get a new index */
		n = macro__num++;
		if (macro__num >= MACRO_MAX) quit("Too many macros!");

		/* Save the pattern */
		macro__pat[n] = string_make(pat);
	}

	/* Save the action */
	macro__act[n] = string_make(act);

	/* Efficiency */
	macro__use[(byte)(pat[0])] = TRUE;

	/* Success */
	return (0);
}


/*
 * Initialize the "macro" package
 */
errr macro_init(void)
{
	/* Macro patterns */
	macro__pat = C_ZNEW(MACRO_MAX, char *);

	/* Macro actions */
	macro__act = C_ZNEW(MACRO_MAX, char *);

	/* Success */
	return (0);
}


/*
 * Free the macro package
 */
errr macro_free(void)
{
	int i;
	size_t j;

	/* Free the macros */
	for (i = 0; i < macro__num; ++i)
	{
		string_free(macro__pat[i]);
		string_free(macro__act[i]);
	}

	FREE(macro__pat);
	FREE(macro__act);

	/* Free the keymaps */
	for (i = 0; i < KEYMAP_MODES; ++i)
	{
		for (j = 0; j < N_ELEMENTS(keymap_act[i]); ++j)
		{
			string_free(keymap_act[i][j]);
			keymap_act[i][j] = NULL;
		}
	}

	/* Success */
	return (0);
}


/*
 * Free the macro trigger package
 */
errr macro_trigger_free(void)
{
	int i;
	int num;

	if (macro_template != NULL)
	{
		/* Free the template */
		string_free(macro_template);
		macro_template = NULL;

		/* Free the trigger names and keycodes */
		for (i = 0; i < max_macrotrigger; i++)
		{
			string_free(macro_trigger_name[i]);

			string_free(macro_trigger_keycode[0][i]);
			string_free(macro_trigger_keycode[1][i]);
		}

		/* No more macro triggers */
		max_macrotrigger = 0;

		/* Count modifier-characters */
		num = strlen(macro_modifier_chr);

		/* Free modifier names */
		for (i = 0; i < num; i++)
			string_free(macro_modifier_name[i]);

		/* Free modifier chars */
		string_free(macro_modifier_chr);
	}

	/* Success */
	return (0);
}


/*
 * Flush all pending input.
 *
 * Actually, remember the flush, using the "inkey_xtra" flag, and in the
 * next call to "inkey()", perform the actual flushing, for efficiency,
 * and correctness of the "inkey()" function.
 */
void flush(void)
{
	/* Do it later */
	inkey_xtra = TRUE;
}


/*
 * Flush all pending input if the OPT(flush_failure) option is set.
 */
void flush_fail(void)
{
	if (flush_failure) flush();
}


/*
 * Local variable -- we are inside a "macro action"
 *
 * Do not match any macros until "ascii 30" is found.
 */
static bool parse_macro = FALSE;


/*
 * Local variable -- we are inside a "macro trigger"
 *
 * Strip all keypresses until a low ascii value is found.
 */
static bool parse_under = FALSE;


/*
 * Helper function called only from "inkey()"
 *
 * This function does almost all of the "macro" processing.
 *
 * We use the "Term_key_push()" function to handle "failed" macros, as well
 * as "extra" keys read in while choosing the proper macro, and also to hold
 * the action for the macro, plus a special "ascii 30" character indicating
 * that any macro action in progress is complete.  Embedded macros are thus
 * illegal, unless a macro action includes an explicit "ascii 30" character,
 * which would probably be a massive hack, and might break things.
 *
 * Only 500 (0+1+2+...+29+30) milliseconds may elapse between each key in
 * the macro trigger sequence.  If a key sequence forms the "prefix" of a
 * macro trigger, 500 milliseconds must pass before the key sequence is
 * known not to be that macro trigger.  XXX XXX XXX
 */
static ui_event_data inkey_aux(int scan_cutoff)
{
	int k = 0, n, p = 0, w = 0;

	ui_event_data ke, ke0;
	char ch;

	cptr pat, act;

	char buf[1024];

	/* Initialize the no return */
	ke0.type = EVT_NONE;
	ke0.key = 0;
	ke0.index = 0; 
	ke0.mousey = 0;
	ke0.mousex = 0;

	/* Wait for a keypress */
	if (scan_cutoff == SCAN_OFF)
	{
		(void)(Term_inkey(&ke, TRUE, TRUE));
		ch = ke.key;
	}
	else
	{
		w = 0;

		/* Wait only as long as macro activation would wait*/
		while (Term_inkey(&ke, FALSE, TRUE) != 0)
		{
			/* Increase "wait" */
			w++;

			/* Excessive delay */
			if (w >= scan_cutoff)
			{
				ke0.type = EVT_KBRD;
				return ke0;
			}

			/* Delay */
			Term_xtra(TERM_XTRA_DELAY, 10);
		}
		ch = ke.key;
	}

	/* End "macro action" */
	if ((ch == 30) || (ch == DEFINED_XFF))
	{
		parse_macro = FALSE;
		return (ke);
	}

	/* Inside "macro action" */
	if (parse_macro) return (ke);

	/* Inside "macro trigger" */
	if (parse_under) return (ke);


	/* Save the first key, advance */
	buf[p++] = ch;
	buf[p] = '\0';

	/* Check for possible macro */
	k = macro_find_check(buf);

	/* No macro pending */
	if (k < 0) return (ke);

	/* Wait for a macro, or a timeout */
	while (TRUE)
	{
		/* Check for pending macro */
		k = macro_find_maybe(buf);

		/* No macro pending */
		if (k < 0) break;

		/* Check for (and remove) a pending key */
		if (0 == Term_inkey(&ke, FALSE, TRUE))
		{
			/* Append the key */
			buf[p++] = ke.key;
			buf[p] = '\0';

			/* Restart wait */
			w = 0;
		}

		/* No key ready */
		else
		{
			/* Increase "wait" */
			w ++;

			/* Excessive delay */
			if (w >= SCAN_MACRO) break;

			/* Delay */
			Term_xtra(TERM_XTRA_DELAY, 10);
		}
	}

	/* Check for available macro */
	k = macro_find_ready(buf);

	/* No macro available */
	if (k < 0)
	{
		/* Push all the "keys" back on the queue */
		/* The most recent event may not be a keypress. */
		if(p)
		{
			if(Term_event_push(&ke)) return (ke0);
			p--;
		}
		while (p > 0)
		{
			/* Push the key, notice over-flow */
			if (Term_key_push(buf[--p])) return (ke0);
		}

		/* Wait for (and remove) a pending key */
		(void)Term_inkey(&ke, TRUE, TRUE);

		/* Return the key */
		return (ke);
	}

	/* Get the pattern */
	pat = macro__pat[k];

	/* Get the length of the pattern */
	n = strlen(pat);

	/* Push the "extra" keys back on the queue */
	while (p > n)
	{
		/* Push the key, notice over-flow */
		if (Term_key_push(buf[--p])) return (ke0);
	}

	/* Begin "macro action" */
	parse_macro = TRUE;

	/* Push the "end of macro action" key */
	if (Term_key_push(30)) return (ke0);


	/* Access the macro action */
	act = macro__act[k];

	/* Get the length of the action */
	n = strlen(act);

	/* Push the macro "action" onto the key queue */
	while (n > 0)
	{
		/* Push the key, notice over-flow */
		if (Term_key_push(act[--n])) return (ke0);
	}

	/* Hack -- Force "inkey()" to call us again */
	return (ke0);
}


/*
 * Mega-Hack -- special "inkey_next" pointer.  XXX XXX XXX
 *
 * This special pointer allows a sequence of keys to be "inserted" into
 * the stream of keys returned by "inkey()".  This key sequence will not
 * trigger any macros, and cannot be bypassed by the Borg.  It is used
 * in Angband to handle "keymaps".
 */
static cptr inkey_next = NULL;


/*
 * Get a keypress from the user.
 *
 * This function recognizes a few "global parameters".  These are variables
 * which, if set to TRUE before calling this function, will have an effect
 * on this function, and which are always reset to FALSE by this function
 * before this function returns.  Thus they function just like normal
 * parameters, except that most calls to this function can ignore them.
 *
 * If "inkey_xtra" is TRUE, then all pending keypresses will be flushed,
 * and any macro processing in progress will be aborted.  This flag is
 * set by the "flush()" function, which does not actually flush anything
 * itself, but rather, triggers delayed input flushing via "inkey_xtra".
 *
 * If "inkey_scan" is TRUE, then we will immediately return "zero" if no
 * keypress is available, instead of waiting for a keypress.
 *
 * If "inkey_base" is TRUE, then all macro processing will be bypassed.
 * If "inkey_base" and "inkey_scan" are both TRUE, then this function will
 * not return immediately, but will wait for a keypress for as long as the
 * normal macro matching code would, allowing the direct entry of macro
 * triggers.  The "inkey_base" flag is extremely dangerous!
 *
 * If "inkey_flag" is TRUE, then we will assume that we are waiting for a
 * normal command, and we will only show the cursor if "OPT(highlight_player)" is
 * TRUE (or if the player is in a store), instead of always showing the
 * cursor.  The various "main-xxx.c" files should avoid saving the game
 * in response to a "menu item" request unless "inkey_flag" is TRUE, to
 * prevent savefile corruption.
 *
 * If we are waiting for a keypress, and no keypress is ready, then we will
 * refresh (once) the window which was active when this function was called.
 *
 * Note that "back-quote" is automatically converted into "escape" for
 * convenience on machines with no "escape" key.  This is done after the
 * macro matching, so the user can still make a macro for "backquote".
 *
 * Note the special handling of "ascii 30" (ctrl-caret, aka ctrl-shift-six)
 * and "ascii 31" (ctrl-underscore, aka ctrl-shift-minus), which are used to
 * provide support for simple keyboard "macros".  These keys are so strange
 * that their loss as normal keys will probably be noticed by nobody.  The
 * "ascii 30" key is used to indicate the "end" of a macro action, which
 * allows recursive macros to be avoided.  The "ascii 31" key is used by
 * some of the "main-xxx.c" files to introduce macro trigger sequences.
 *
 * Hack -- we use "ascii 29" (ctrl-right-bracket) as a special "magic" key,
 * which can be used to give a variety of "sub-commands" which can be used
 * any time.  These sub-commands could include commands to take a picture of
 * the current screen, to start/stop recording a macro action, etc.
 *
 * If "angband_term[0]" is not active, we will make it active during this
 * function, so that the various "main-xxx.c" files can assume that input
 * is only requested (via "Term_inkey()") when "angband_term[0]" is active.
 *
 * Mega-Hack -- This function is used as the entry point for clearing the
 * "signal_count" variable, and of the "character_saved" variable.
 *
 * Hack -- Note the use of "inkey_next" to allow "keymaps" to be processed.
 *
 * Mega-Hack -- Note the use of "inkey_hack" to allow the "Borg" to steal
 * control of the keyboard from the user.
 */
ui_event_data inkey_ex(void)
{
	bool cursor_state;
	ui_event_data kk;
	ui_event_data ke;

	bool done = FALSE;

	term *old = Term;

	/* Initialise keypress */
	ke.key = 0;
	ke.type = EVT_NONE;

	/* Hack -- Use the "inkey_next" pointer */
	if (inkey_next && *inkey_next && !inkey_xtra)
	{
		/* Get next character, and advance */
		ke.key = *inkey_next++;
		ke.type = EVT_KBRD;

		/* Cancel the various "global parameters" */
		inkey_base = inkey_xtra = inkey_flag = FALSE;
		inkey_scan = 0;

		/* Accept result */
		return (ke);
	}

	/* Forget pointer */
	inkey_next = NULL;

	/* Hack -- handle delayed "flush()" */
	if (inkey_xtra)
	{
		/* End "macro action" */
		parse_macro = FALSE;

		/* End "macro trigger" */
		parse_under = FALSE;

		/* Forget old keypresses */
		Term_flush();
	}

	/* Get the cursor state */
	(void)Term_get_cursor(&cursor_state);

	/* Show the cursor if waiting, except sometimes in "command" mode */
	if (!inkey_scan && (!inkey_flag || (hilight_player) || character_icky))
	{
		/* Show the cursor */
		(void)Term_set_cursor(TRUE);
	}

	/* Hack -- Activate main screen */
	Term_activate(term_screen);

	/* Get a key */
	while (ke.type == EVT_NONE)
	{
		/* Hack -- Handle "inkey_scan == SCAN_INSTANT */
		if (!inkey_base && inkey_scan == SCAN_INSTANT &&
		   (0 != Term_inkey(&kk, FALSE, FALSE)))
		{
			ke.type = EVT_KBRD;
			break;
		}

		/* Hack -- Flush output once when no key ready */
		if (!done && (0 != Term_inkey(&kk, FALSE, FALSE)))
		{

			/* Hack -- activate proper term */
			Term_activate(old);

			/* Flush output */
			Term_fresh();

			/* Hack -- activate main screen */
			Term_activate(term_screen);

			/* Mega-Hack -- reset saved flag */
			character_saved = FALSE;

			/* Mega-Hack -- reset signal counter */
			signal_count = 0;

			/* Only once */
			done = TRUE;
		}

		/* Hack -- Handle "inkey_base" */
		if (inkey_base)
		{
			int w = 0;

			/* Wait forever */
			if (!inkey_scan)
			{
				/* Wait for (and remove) a pending key */
				if (0 == Term_inkey(&ke, TRUE, TRUE))
				{
					/* Done */
					ke.type = EVT_KBRD;
					break;
				}

				/* Oops */
				break;
			}

			/* Wait only as long as macro activation would wait*/
			while (TRUE)
			{
				/* Check for (and remove) a pending key */
				if (0 == Term_inkey(&ke, FALSE, TRUE))
				{
					/* Done */
					ke.type = EVT_KBRD;
					break;
				}

				/* No key ready */
				else
				{
					/* Increase "wait" */
					w ++;

					/* Excessive delay */
					if (w >= SCAN_MACRO) break;

					/* Delay */
					Term_xtra(TERM_XTRA_DELAY, 10);
				}
			}

			/* Done */
			ke.type = EVT_KBRD;
			break;
		}

		/* Get a key (see above) */
		ke = inkey_aux(inkey_scan);

		/* Handle mouse buttons */
		if ((ke.type == EVT_MOUSE) && (mouse_buttons))
		{
			/* Check to see if we've hit a button */
			/* Assuming text buttons here for now - this would have to
			 * change for GUI buttons */
			char key = button_get_key(ke.mousex, ke.mousey);

			if (key)
			{
				/* Rewrite the event */
				ke.type = EVT_BUTTON;
				ke.key = key;
				ke.index = 0;
				ke.mousey = 0;
				ke.mousex = 0;

				/* Done */
				break;
			}
		}

		/* Handle "control-right-bracket" */
		if (ke.key == 29)
		{
			/* Strip this key */
			ke.type = EVT_NONE;

			/* Continue */
			continue;
		}

		/* Treat back-quote as escape */
		if (ke.key == '`') ke.key = ESCAPE;

		/* End "macro trigger" */
		if (parse_under && (ke.key >=0 && ke.key <= 32))
		{
			/* Strip this key */
			ke.type = EVT_NONE;
			ke.key = 0;

			/* End "macro trigger" */
			parse_under = FALSE;
		}

		/* Handle "control-caret" */
		if (ke.key == 30)
		{
			/* Strip this key */
			ke.type = EVT_NONE;
			ke.key = 0;
		}

		/* Handle "control-underscore" */
		else if (ke.key == 31)
		{
			/* Strip this key */
			ke.type = EVT_NONE;
			ke.key = 0;

			/* Begin "macro trigger" */
			parse_under = TRUE;
		}

		/* Inside "macro trigger" */
		else if (parse_under)
		{
			/* Strip this key */
			ke.type = EVT_NONE;
			ke.key = 0;
		}
	}

	/* Hack -- restore the term */
	Term_activate(old);

	/* Restore the cursor */
	Term_set_cursor(cursor_state);

	/* Cancel the various "global parameters" */
	inkey_base = inkey_xtra = inkey_flag = FALSE;
	inkey_scan = 0;

	/* Return the keypress */
	return (ke);
}


/*
 * Get a keypress or mouse click from the user.
 */
char anykey(void)
{
	ui_event_data ke = EVENT_EMPTY;

	/* Only accept a keypress or mouse click */
	do
	{
		ke = inkey_ex();
	} while ((ke.type != EVT_MOUSE) && (ke.type != EVT_KBRD));

	return ke.key;
}


/*
 * Get a "keypress" from the user.
 */
char inkey(void)
{
	ui_event_data ke = EVENT_EMPTY;

	/* Only accept a keypress */
	do
	{
		ke = inkey_ex();
	} while ((ke.type != EVT_ESCAPE) && (ke.type != EVT_KBRD));

	/* Paranoia */
	if (ke.type == EVT_ESCAPE) ke.key = ESCAPE;
	return ke.key;
}


/*
 * Flush the screen, make a noise
 */
void bell(cptr reason)
{
	/* Mega-Hack -- Flush the output */
	Term_fresh();

	/* Hack -- memorize the reason if possible */
	if (character_generated && reason)
	{
		message_add(reason, MSG_BELL);

		/* Window stuff */
		p_ptr->redraw |= (PR_MESSAGE);
		redraw_stuff();
	}

	/* Make a bell noise (if allowed) */
	if (ring_bell) Term_xtra(TERM_XTRA_NOISE, 0);

	/* Flush the input (later!) */
	flush();
}


/*
 * Hack -- Make a (relevant?) sound
 */
void sound(int val)
{
	/* No sound */
	if ((!use_sound) || (!sound_hook)) return;

	sound_hook(val);
}


/*
 * Hack -- flush
 */
static void msg_flush(int x)
{
	byte a = TERM_L_BLUE;

	/* Pause for response */
	Term_putstr(x, 0, -1, a, "-more-");

	if (!auto_more)
	{
		/* Get an acceptable keypress */
		while (1)
		{
			char ch;
			ch = inkey();
			if (quick_messages) break;
			if ((ch == ESCAPE) || (ch == ' ')) break;
			if ((ch == '\n') || (ch == '\r')) break;
			bell("Illegal response to a 'more' prompt!");
		}
	}

	/* Clear the line */
	Term_erase(0, 0, 255);
}


static int message_column = 0;


/*
 * Output a message to the top line of the screen.
 *
 * Break long messages into multiple pieces (40-72 chars).
 *
 * Allow multiple short messages to "share" the top line.
 *
 * Prompt the user to make sure he has a chance to read them.
 *
 * These messages are memorized for later reference (see above).
 *
 * We could do a "Term_fresh()" to provide "flicker" if needed.
 *
 * The global "msg_flag" variable can be cleared to tell us to "erase" any
 * "pending" messages still on the screen, instead of using "msg_flush()".
 * This should only be done when the user is known to have read the message.
 *
 * We must be very careful about using the "msg_print()" functions without
 * explicitly calling the special "msg_print(NULL)" function, since this may
 * result in the loss of information if the screen is cleared, or if anything
 * is displayed on the top line.
 *
 * Hack -- Note that "msg_print(NULL)" will clear the top line even if no
 * messages are pending.
 */
static void msg_print_aux(MessageType type, cptr msg)
{
	int n;
	char *t;
	char buf[1024];
	byte color;
	int w, h;

	/* Obtain the size */
	(void)Term_get_size(&w, &h);

	/* Hack -- Reset */
	if (!msg_flag) message_column = 0;

	/* Message Length */
	n = (msg ? strlen(msg) : 0);

	/* Hack -- flush when requested or needed */
	if (message_column && (!msg || ((message_column + n) > (w - 8))))
	{
		/* Flush */
		msg_flush(message_column);

		/* Forget it */
		msg_flag = FALSE;

		/* Reset */
		message_column = 0;
	}

	/* No message */
	if (!msg) return;

	/* Paranoia */
	if (n > 1000) return;

	/* Memorize the message (if legal) */
	if (character_generated && !(p_ptr->is_dead))
		message_add(msg, type);

	/* Window stuff */
	p_ptr->redraw |= (PR_MESSAGE);

	/* Copy it */
	my_strcpy(buf, msg, sizeof(buf));

	/* Analyze the buffer */
	t = buf;

	/* Get the color of the message */
	color = message_type_color(type);

	/* Split message */
	while (n > (w - 8))
	{
		char oops;

		int check, split;

		/* Default split */
		split = (w - 8);

		/* Find the "best" split point */
		for (check = (w / 2); check < (w - 8); check++)
		{
			/* Found a valid split point */
			if (t[check] == ' ') split = check;
		}

		/* Save the split character */
		oops = t[split];

		/* Split the message */
		t[split] = '\0';

		/* Display part of the message */
		Term_putstr(0, 0, split, color, t);

		/* Flush it */
		msg_flush(split + 1);

		/* Restore the split character */
		t[split] = oops;

		/* Insert a space */
		t[--split] = ' ';

		/* Prepare to recurse on the rest of "buf" */
		t += split; n -= split;
	}

	/* Display the tail of the message */
	Term_putstr(message_column, 0, n, color, t);

	/* Remember the message */
	msg_flag = TRUE;

	/* Remember the position */
	message_column += n + 1;

	/* Send refresh event */
	event_signal(EVENT_MESSAGE);
}


/*
 * Print a message in the default color (white)
 * Note this function does not handle accented characters.
 */
void msg_print(cptr msg)
{
	msg_print_aux(MSG_GENERIC, msg);
}


/*
 * Print message and flush (used for de-bugging)
 */
void playtesting(cptr msg)
{
	msg_print(msg);
	message_flush();
}


/*
 * Display a formatted message of a given type, using "vstrnfmt()" and "msg_print()".
 */
void msg_c_format(const MessageType mtype, const char *fmt, ...)
{
	va_list vp;

	char buf[1024];

	/* Begin the Varargs Stuff */
	va_start(vp, fmt);

	/* Format the args, save the length */
	(void)vstrnfmt(buf, sizeof(buf), fmt, vp);

	/* End the Varargs Stuff */
	va_end(vp);

	/* Display */
	msg_print_aux(mtype, buf);
}


/*
 * Display a generic, formatted message, using "vstrnfmt()" and "msg_print()".
 */
void msg_format(cptr fmt, ...)
{
	va_list vp;

	char buf[1024];

	/* Begin the Varargs Stuff */
	va_start(vp, fmt);

	/* Format the args, save the length */
	(void)vstrnfmt(buf, sizeof(buf), fmt, vp);

	/* End the Varargs Stuff */
	va_end(vp);

	/* Display */
	msg_print_aux(MSG_GENERIC, buf);
}


/*
 * Display a message and play the associated sound.
 *
 * The "extra" parameter is currently unused.
 */
void message(const MessageType message_type, s16b extra, cptr message)
{
	/* Unused parameter */
	(void)extra;

	sound(message_type);

	msg_print_aux(message_type, message);
}


/*
 * Display a formatted message and play the associated sound.
 *
 * The "extra" parameter is currently unused.
 */
void message_format(const MessageType message_type, s16b extra, cptr fmt, ...)
{
	va_list vp;

	char buf[1024];

	/* Begin the Varargs Stuff */
	va_start(vp, fmt);

	/* Format the args, save the length */
	(void)vstrnfmt(buf, sizeof(buf), fmt, vp);

	/* End the Varargs Stuff */
	va_end(vp);

	/* Display */
	message(message_type, extra, buf);
}


/*
 * Print the queued messages.
 */
void message_flush(void)
{
	/* Hack -- Reset */
	if (!msg_flag) message_column = 0;

	/* Flush when needed */
	if (message_column)
	{
		/* Print pending messages */
		msg_flush(message_column);

		/* Forget it */
		msg_flag = FALSE;

		/* Reset */
		message_column = 0;
	}
}


/*
 * Clears top line, re-set message column without a -more- prompt
 */
void clear_message_line(void)
{
	prt("", 0, 0);
	message_column = 0;
	msg_flag = FALSE;
}


/*
 * Save the screen, and increase the "icky" depth.
 *
 * This function must match exactly one call to "screen_load()".
 */
void screen_save(void)
{
	/* Hack -- Flush messages */
	message_flush();

	/* Save the screen (if legal) */
	Term_save();

	/* Increase "icky" depth */
	character_icky++;
}


/*
 * Load the screen, and decrease the "icky" depth.
 *
 * This function must match exactly one call to "screen_save()".
 */
void screen_load(void)
{
	/* Hack -- Flush messages */
	message_flush();

	/* Load the screen (if legal) */
	Term_load();

	/* Decrease "icky" depth */
	character_icky--;
}


/*
 * Display a string on the screen using an attribute.
 *
 * At the given location, using the given attribute, if allowed,
 * add the given string.  Do not clear the line.
 */
void c_put_str(byte attr, cptr str, int row, int col)
{
	/* Position cursor, Dump the attr/text */
	Term_putstr(col, row, -1, attr, str);
}


/*
 * As above, but in "white"
 */
void put_str(cptr str, int row, int col)
{
	/* Spawn */
	Term_putstr(col, row, -1, TERM_WHITE, str);
}


/*
 * Display a string on the screen using an attribute, and clear
 * to the end of the line.
 */
void c_prt(byte attr, cptr str, int row, int col)
{
	/* Clear line, position cursor */
	Term_erase(col, row, 255);

	/* Dump the attr/text */
	Term_addstr(-1, attr, str);
}


/*
 * As above, but in "white"
 */
void prt(cptr str, int row, int col)
{
	/* Spawn */
	c_prt(TERM_WHITE, str, row, col);
}


/*
 * Print some (colored) text to the screen at the current cursor position,
 * automatically "wrapping" existing text (at spaces) when necessary to
 * avoid placing any text into the last column, and clearing every line
 * before placing any text in that line.  Also, allow "newline" to force
 * a "wrap" to the next line.  Advance the cursor as needed so sequential
 * calls to this function will work correctly.
 *
 * Once this function has been called, the cursor should not be moved
 * until all the related "text_out()" calls to the window are complete.
 *
 * This function will correctly handle any width up to the maximum legal
 * value of 256, though it works best for a standard 80 character width.
 */
void text_out_to_screen(byte a, cptr str)
{
	int x, y;

	int wid, h;

	int wrap;

	cptr s;
	char buf[1024];

	/* We use either ascii or system-specific encoding */
	 int encoding = (xchars_to_file) ? SYSTEM_SPECIFIC : ASCII;

	/* Obtain the size */
	(void)Term_get_size(&wid, &h);

	/* Obtain the cursor */
	(void)Term_locate(&x, &y);

	/* Copy to a rewriteable string */
	 my_strcpy(buf, str, 1024);

	 /* Translate it to 7-bit ASCII or system-specific format */
	 xstr_trans(buf, encoding);

	/* Use special wrapping boundary? */
	if ((text_out_wrap > 0) && (text_out_wrap < wid))
		wrap = text_out_wrap;
	else
		wrap = wid;

	/* Process the string */
	for (s = buf; *s; s++)
	{
		char ch;

		/* Force wrap */
		if (*s == '\n')
		{
			/* Wrap */
			x = text_out_indent;
			y++;

			/* Clear line, move cursor */
			Term_erase(x, y, 255);

			continue;
		}

		/* Clean up the char */
		ch = (my_isprint((unsigned char)*s) ? *s : ' ');

		/* Wrap words as needed */
		if ((x >= wrap - 1) && (ch != ' '))
		{
			int i, n = 0;

			byte av[256];
			char cv[256];

			/* Wrap word */
			if (x < wrap)
			{
				/* Scan existing text */
				for (i = wrap - 2; i >= 0; i--)
				{
					/* Grab existing attr/char */
					Term_what(i, y, &av[i], &cv[i]);

					/* Break on space */
					if (cv[i] == ' ') break;

					/* Track current word */
					n = i;
				}
			}

			/* Special case */
			if (n == 0) n = wrap;

			/* Clear line */
			Term_erase(n, y, 255);

			/* Wrap */
			x = text_out_indent;
			y++;

			/* Clear line, move cursor */
			Term_erase(x, y, 255);

			/* Wrap the word (if any) */
			for (i = n; i < wrap - 1; i++)
			{
				/* Dump */
				Term_addch(av[i], cv[i]);

				/* Advance (no wrap) */
				if (++x > wrap) x = wrap;
			}
		}

		/* Dump */
		Term_addch(a, ch);

		/* Advance */
		if (++x > wrap) x = wrap;
	}
}


/*
 * Write text to the given file and apply line-wrapping.
 *
 * Hook function for text_out(). Make sure that text_out_file points
 * to an open text-file.
 *
 * Long lines will be wrapped at text_out_wrap, or at column 75 if that
 * is not set; or at a newline character.  Note that punctuation can
 * sometimes be placed one column beyond the wrap limit.
 *
 * You must be careful to end all file output with a newline character
 * to "flush" the stored line position.
 */
void text_out_to_file(byte a, cptr str)
{
	cptr s;
	char buf[1024];

	/* Current position on the line */
	static int pos = 0;

	/* Wrap width */
	int wrap = (text_out_wrap ? text_out_wrap : 75);

	/* Output line buffer */
	char *out;

	/* Current position in output buffer */
	int out_pos = 0;

	/* We use either ascii or system-specific encoding */
 	int encoding = (xchars_to_file) ? SYSTEM_SPECIFIC : ASCII;

	/* Unused parameter */
	(void)a;

	/* Copy to a rewriteable string */
 	my_strcpy(buf, str, 1024);

 	out = (char *)mem_alloc((wrap + 1) * sizeof(char));

 	/* Translate it to 7-bit ASCII or system-specific format */
 	xstr_trans(buf, encoding);

	/* Current location within "buf" */
 	s = buf;

	/* Process the string */
	while (*s)
	{
		char ch;
		int n = 0;
		int len = wrap - pos;
		int l_space = -1;

		/* If we are at the start of the line... */
		if (pos == 0)
		{
			int i;

			/* Output the indent */
			for (i = 0; i < text_out_indent; i++)
			{
				out[out_pos++] = ' ';
				pos++;
			}
		}

		/* Find length of line up to next newline or end-of-string */
		while ((n < len) && !((s[n] == '\n') || (s[n] == '\0')))
		{
			/* Mark the most recent space in the string */
			if (s[n] == ' ') l_space = n;

			/* Increment */
			n++;
		}

		/* If we have encountered no spaces */
		if ((l_space == -1) && (n == len))
		{
			/* If we are at the start of a new line */
			if (pos == text_out_indent)
			{
				len = n;
			}
			/* HACK - Output punctuation at the end of the line */
			else if ((s[0] == ' ') || (s[0] == ',') || (s[0] == '.'))
			{
				len = 1;
			}
			else
			{
				/* Begin a new line */
				file_writec(text_out_file, '\n');

				/* Reset */
				pos = 0;
				out_pos = 0;

				continue;
			}
		}
		else
		{
			/* Wrap at the newline */
			if ((s[n] == '\n') || (s[n] == '\0')) len = n;

			/* Wrap at the last space */
			else len = l_space;
		}

		/* Write that line to file */
		for (n = 0; n < len; n++)
		{
			/* Ensure the character is printable */
			ch = (my_isprint((unsigned char) s[n]) ? s[n] : ' ');

			/* Add to the buffer */
			out[out_pos++] = ch;

			/* Increment */
			pos++;
		}

		/* Write out the buffer */
		file_write(text_out_file, out, out_pos);

		/* Move 's' past the stuff we've written */
		s += len;

		/* If we are at the end of the string, end */
		if (*s == '\0')
		{
			mem_free(out);
			return;
		}

		/* Skip newlines */
		if (*s == '\n') s++;

		/* Begin a new line */
		file_writec(text_out_file, '\n');

		/* Reset */
		pos = 0;
		out_pos = 0;

		/* Skip whitespace */
		while (*s == ' ') s++;
	}

	/* free output buffer */
	mem_free(out);

	/* We are done */
	return;
}


/*
 * Output text to the screen or to a file depending on the selected
 * text_out hook.
 */
void text_out(const char *fmt, ...)
{
	char buf[1024];
	va_list vp;

	/* Begin the Varargs Stuff */
	va_start(vp, fmt);

	/* Do the va_arg fmt to the buffer */
	(void)vstrnfmt(buf, sizeof(buf), fmt, vp);

	/* End the Varargs Stuff */
	va_end(vp);

	/* Output now */
	text_out_hook(TERM_WHITE, buf);
}


/*
 * Output text to the screen (in color) or to a file depending on the
 * selected hook.
 */
void text_out_c(byte a, const char *fmt, ...)
{
	char buf[1024];
	va_list vp;

	/* Begin the Varargs Stuff */
	va_start(vp, fmt);

	/* Do the va_arg fmt to the buffer */
	(void)vstrnfmt(buf, sizeof(buf), fmt, vp);

	/* End the Varargs Stuff */
	va_end(vp);

	/* Output now */
	text_out_hook(a, buf);
}


/*
 * Given a "formatted" chunk of text (i.e. one including tags like {red}{/})
 * in 'source', with starting point 'init', this finds the next section of
 * text and any tag that goes with it, return TRUE if it finds something to
 * print.
 *
 * If it returns TRUE, then it also fills 'text' with a pointer to the start
 * of the next printable section of text, and 'len' with the length of that
 * text, and 'end' with a pointer to the start of the next section.  This
 * may differ from "text + len" because of the presence of tags.  If a tag
 * applies to the section of text, it returns a pointer to the start of that
 * tag in 'tag' and the length in 'taglen'.  Otherwise, 'tag' is filled with
 * NULL.
 *
 * See text_out_e for an example of its use.
 */
static bool next_section(const char *source, size_t init, const char **text, size_t *len, const char **tag, size_t *taglen, const char **end)
{
	const char *next;

	*tag = NULL;
	*text = source + init;
	if (*text[0] == '\0') return FALSE;

	next = strchr(*text, '{');
	while (next)
	{
		const char *s = next + 1;

		while (*s && isalpha((unsigned char) *s)) s++;

		/* Woo!  valid opening tag thing */
		if (*s == '}')
		{
			const char *close = strstr(s, "{/}");

			/* There's a closing thing, so it's valid. */
			if (close)
			{
				/* If this tag is at the start of the fragment */
				if (next == *text)
				{
					*tag = *text + 1;
					*taglen = s - *text - 1;
					*text = s + 1;
					*len = close - *text;
					*end = close + 3;
					return TRUE;
				}
				/* Otherwise return the chunk up to this */
				else
				{
					*len = next - *text;
					*end = *text + *len;
					return TRUE;
				}
			}
			/* No closing thing, therefore all one lump of text. */
			else
			{
				*len = strlen(*text);
				*end = *text + *len;
				return TRUE;
			}
		}
		/* End of the string, that's fine. */
		else if (*s == '\0')
		{
				*len = strlen(*text);
				*end = *text + *len;
				return TRUE;
		}
		/* An invalid tag, skip it. */
		else
		{
			next = next + 1;
		}

		next = strchr(next, '{');
	}

	/* Default to the rest of the string */
	*len = strlen(*text);
	*end = *text + *len;

	return TRUE;
}


/*
 * Output text to the screen or to a file depending on the
 * selected hook.  Takes strings with "embedded formatting",
 * such that something within {red}{/} will be printed in red.
 *
 * Note that such formatting will be treated as a "breakpoint"
 * for the printing, so if used within words may lead to part of the
 * word being moved to the next line.
 */
void text_out_e(const char *fmt, ...)
{
	char buf[1024];
	char smallbuf[1024];
	va_list vp;

	const char *start, *next, *text, *tag;
	size_t textlen, taglen = 0;

	/* Begin the Varargs Stuff */
	va_start(vp, fmt);

	/* Do the va_arg fmt to the buffer */
	(void)vstrnfmt(buf, sizeof(buf), fmt, vp);

	/* End the Varargs Stuff */
	va_end(vp);

	start = buf;
	while (next_section(start, 0, &text, &textlen, &tag, &taglen, &next))
	{
		int a = -1;

		memcpy(smallbuf, text, textlen);
		smallbuf[textlen] = 0;

		if (tag)
		{
			char tagbuffer[11];

			/* Colour names are less than 11 characters long. */
			assert(taglen < 11);

			memcpy(tagbuffer, tag, taglen);
			tagbuffer[taglen] = '\0';

			a = color_text_to_attr(tagbuffer);
		}

		if (a == -1)
			a = TERM_WHITE;

		/* Output now */
		text_out_hook(a, smallbuf);

		start = next;
	}
}


/*
 * Clear part of the screen
 */
void clear_from(int row)
{
	int y;

	/* Erase requested rows */
	for (y = row; y < Term->hgt; y++)
	{
		/* Erase part of the screen */
		Term_erase(0, y, 255);
	}
}


/*
 * The default "keypress handling function" for askfor_aux, this takes the
 * given keypress, input buffer, length, etc, and does the appropriate action
 * for each keypress, such as moving the cursor left or inserting a character.
 *
 * It should return TRUE when editing of the buffer is "complete" (e.g. on
 * the press of RETURN).
 */
bool askfor_aux_keypress(char *buf, size_t buflen, size_t *curs, size_t *len, char keypress, bool firsttime)
{
	switch (keypress)
	{
		case ESCAPE:
		{
			*curs = 0;
			return TRUE;
		}

		case '\n':
		case '\r':
		{
			*curs = *len;
			return TRUE;
		}

		case ARROW_LEFT:
		{
			if (firsttime) *curs = 0;
			if (*curs > 0) (*curs)--;
			break;
		}

		case ARROW_RIGHT:
		{
			if (firsttime) *curs = *len - 1;
			if (*curs < *len) (*curs)++;
			break;
		}

		case 0x7F:
		case '\010':
		{
			/* If this is the first time round, backspace means "delete all" */
			if (firsttime)
			{
				buf[0] = '\0';
				*curs = 0;
				*len = 0;

				break;
			}

			/* Refuse to backspace into oblivion */
			if (*curs == 0) break;

			/* Move the string from k to nul along to the left by 1 */
			memmove(&buf[*curs - 1], &buf[*curs], *len - *curs);

			/* Decrement */
			(*curs)--;
			(*len)--;

			/* Terminate */
			buf[*len] = '\0';

			break;
		}

		default:
		{
			bool atnull = (buf[*curs] == 0);


			if (!my_isprint((unsigned char)keypress))
			{
				bell("Illegal edit key!");
				break;
			}

			/* Clear the buffer if this is the first time round */
			if (firsttime)
			{
				buf[0] = '\0';
				*curs = 0;
				*len = 0;
				atnull = 1;
			}

			if (atnull)
			{
				/* Make sure we have enough room for a new character */
				if ((*curs + 1) >= buflen) break;
			}
			else
			{
				/* Make sure we have enough room to add a new character */
				if ((*len + 1) >= buflen) break;

				/* Move the rest of the buffer along to make room */
				memmove(&buf[*curs+1], &buf[*curs], *len - *curs);
			}

			/* Insert the character */
			buf[(*curs)++] = keypress;
			(*len)++;

			/* Terminate */
			buf[*len] = '\0';

			break;
		}
	}

	/* By default, we aren't done. */
	return FALSE;
}


/*
 * Get some input at the cursor location.
 *
 * The buffer is assumed to have been initialized to a default string.
 * Note that this string is often "empty" (see below).
 *
 * The default buffer is displayed in yellow until cleared, which happens
 * on the first keypress, unless that keypress is Return.
 *
 * Normal chars clear the default and append the char.
 * Backspace clears the default or deletes the final char.
 * Return accepts the current buffer contents and returns TRUE.
 * Escape clears the buffer and the window and returns FALSE.
 *
 * Note that 'len' refers to the size of the buffer.  The maximum length
 * of the input is 'len-1'.
 *
 * 'keypress_h' is a pointer to a function to handle keypresses, altering
 * the input buffer, cursor position and suchlike as required.  See
 * 'askfor_aux_keypress' (the default handler if you supply NULL for
 * 'keypress_h') for an example.
 */
bool askfor_aux(char *buf, size_t len, bool keypress_h(char *, size_t, size_t *, size_t *, char, bool))
{
	int y, x;

	size_t k = 0;		/* Cursor position */
	size_t nul = 0;		/* Position of the null byte in the string */

	ui_event_data ke = EVENT_EMPTY;

	bool done = FALSE;
	bool firsttime = TRUE;

	if (keypress_h == NULL)
	{
		keypress_h = askfor_aux_keypress;
	}

	/* Locate the cursor */
	Term_locate(&x, &y);

	/* Paranoia */
	if ((x < 0) || (x >= 80)) x = 0;


	/* Restrict the length */
	if (x + len > 80) len = 80 - x;

	/* Truncate the default entry */
	buf[len-1] = '\0';

	/* Get the position of the null byte */
	nul = strlen(buf);

	/* Display the default answer */
	Term_erase(x, y, (int)len);
	Term_putstr(x, y, -1, TERM_YELLOW, buf);

	/* Process input */
	while (!done)
	{
		/* Place cursor */
		Term_gotoxy(x + k, y);

		/* Get a key */
		ke = inkey_ex();

		/* Let the keypress handler deal with the keypress */
		done = keypress_h(buf, len, &k, &nul, ke.key, firsttime);

		/* Player used one of the statusline buttons */
		if ((ke.type & (EVT_BUTTON)) && (firsttime)) done = TRUE;

		/* Update the entry */
		Term_erase(x, y, (int)len);
		Term_putstr(x, y, -1, TERM_WHITE, buf);

		/* Not the first time round anymore */
		firsttime = FALSE;
	}

	/* Done */
	return (ke.key != ESCAPE);
}


/*
 * A "keypress" handling function for askfor_aux, that handles the special
 * case of '*' for a new random "name" and passes any other "keypress"
 * through to the default "editing" handler.
 */
static bool get_name_keypress(char *buf, size_t buflen, size_t *curs, size_t *len, char keypress, bool firsttime)
{
	bool result;

	switch (keypress)
	{
		case '*':
		{
			make_random_name(buf, 4, 12);
			buf[0] = toupper((unsigned char) buf[0]);
			*curs = 0;
			result = FALSE;
			break;
		}

		default:
		{
			result = askfor_aux_keypress(buf, buflen, curs, len, keypress, firsttime);
			break;
		}
	}

	return result;
}


/*
 * Gets a name for the character, reacting to name changes.
 *
 * If sf is TRUE, we change the savefile name depending on the character name.
 *
 * What a horrible name for a global function.  XXX XXX XXX
 */
bool get_name(char *buf, size_t buflen)
{
	bool res;

	/* Paranoia XXX XXX XXX */
	message_flush();
	/* add a button for random name */

	button_backup_all();
	button_kill_all();
	button_add("[ESC]", ESCAPE);
	button_add("[ENTER]", '\r');
	button_add("[RANDOM_NAME]", '*');
	event_signal(EVENT_MOUSEBUTTONS);

	/* Display prompt */
	prt("Enter a name for your character (* for a random name): ", 0, 0);

	/* Save the player name */
	my_strcpy(buf, op_ptr->full_name, buflen);

	/* Ask the user for a string */
	res = askfor_aux(buf, buflen, get_name_keypress);

	/* Clear prompt */
	prt("", 0, 0);

	/* Revert to the old name if the player doesn't pick a new one. */
	if (!res)
	{
		my_strcpy(buf, op_ptr->full_name, buflen);
	}

	button_restore();
	event_signal(EVENT_MOUSEBUTTONS);

	return res;
}


/*
 * Prompt for a string from the user.
 *
 * The "prompt" should take the form "Prompt: ".
 *
 * See "askfor_aux" for some notes about "buf" and "len", and about
 * the return value of this function.
 */
bool get_string(cptr prompt, char *buf, size_t len)
{
	bool res;

	/* Paranoia XXX XXX XXX */
	message_flush();

	/* Display prompt */
	prt(prompt, 0, 0);

	/* Ask the user for a string */
	res = askfor_aux(buf, len, NULL);

	/* Translate it to 8-bit (Latin-1) */
 	xstr_trans(buf, LATIN1);

	/* Clear prompt */
	prt("", 0, 0);

	/* Result */
	return (res);
}


/*
 * Request a "quantity" from the user
 *
 * Allow "p_ptr->command_arg" to specify a quantity
 */
s16b get_quantity(cptr prompt, int max)
{
	int amt = 1;

	button_add("[ALL]", 'a');
	event_signal(EVENT_MOUSEBUTTONS);

	/* Use "command_arg" */
	if (p_ptr->command_arg)
	{
		/* Extract a number */
		amt = p_ptr->command_arg;

		/* Clear "command_arg" */
		p_ptr->command_arg = 0;
	}

	/* Prompt if needed */
	else if ((max != 1))
	{
		char tmp[80];

		char buf[80];

		/* Build a prompt if needed */
		if (!prompt)
		{
			/* Build a prompt */
			strnfmt(tmp, sizeof(tmp), "Quantity (0-%d, '*' or 'a' = all): ", max);

			/* Use that prompt */
			prompt = tmp;
		}

		/* Build the default */
		strnfmt(buf, sizeof(buf), "%d", amt);

		/* Ask for a quantity */
		if (!get_string(prompt, buf, 7)) return (0);

		/* Extract a number */
		amt = atoi(buf);

		/* A star or letter means "all" */
		if ((buf[0] == '*') || (buf[0] == 'a') ||(buf[0] == 'A') ||
				isalpha((unsigned char)buf[0])) amt = max;
	}

	/* Enforce the maximum */
	if (amt > max) amt = max;

	/* Enforce the minimum */
	if (amt < 0) amt = 0;

	button_kill('a');
	event_signal(EVENT_MOUSEBUTTONS);

	/* Return the result */
	return (amt);
}


/*
 * Hack - duplication of get_check prompt to give option of setting destroyed
 * option to squelch.
 *
 * 0 - No
 * 1 = Yes
 * 2 = third option
 *
 * The "prompt" should take the form "Query? "
 *
 * Note that "[y/n/{char}]" is appended to the prompt, along with an explanation.
 */
int get_check_other(cptr prompt, cptr other_text, char other, cptr explain)
{
	ui_event_data ke;

	char buf[160];

	/* Paranoia XXX XXX XXX */
	message_flush();

	/* Hack -- Build a "useful" prompt */
	strnfmt(buf, sizeof(buf), "%s [y/n/%c] %s ", prompt, other, explain);

	/* Make some buttons */
	button_backup_all();
	button_kill_all();
	button_add("[YES]", 'y');
	button_add("[NO]", 'n');
	button_add(other_text, other);
	event_signal(EVENT_MOUSEBUTTONS);

	/* Prompt for it */
	prt(buf, 0, 0);

	/* Get an acceptable answer */
	while (TRUE)
	{
		ke = inkey_ex();
		if (quick_messages) break;
		if (ke.key == ESCAPE) break;
		if (ke.key == '\r') break;
		if (strchr("YyNn", ke.key))	break;
		if ((ke.key == tolower((int)other)) || (ke.key == toupper((int)other))) break;
		bell("Illegal response to a 'yes/no' question!");
	}

	/* Restore the old buttons */
	button_restore();
	event_signal(EVENT_MOUSEBUTTONS);

	/* Erase the prompt */
	prt("", 0, 0);

	/* Normal negation */
	if ((ke.key == 'Y') || (ke.key == 'y') || ke.key == '\r')	return (TRUE);
	/*other option*/
	else if ((ke.key == toupper((int)other)) || (ke.key == tolower((int)other))) return (2);
	/*all else default to no*/

	/* Negative result */
	return (FALSE);
}


/*
 * Verify something with the user
 *
 * The "prompt" should take the form "Query? "
 *
 * Note that "[y/n]" is appended to the prompt.
 */
bool get_check(cptr prompt)
{
	ui_event_data ke;

	char buf[80];

	/* Paranoia XXX XXX XXX */
	message_flush();

	/* Hack -- Build a "useful" prompt */
	strnfmt(buf, 78, "%.70s[y/n] ", prompt);

	/* Make some buttons */
	button_backup_all();
	button_kill_all();
	button_add("[YES]", 'y');
	button_add("[NO]", 'n');
	event_signal(EVENT_MOUSEBUTTONS);

	/* Prompt for it */
	prt("", 0, 0);
	prt(buf, 0, 0);

	/* Get an acceptable answer */
	while (TRUE)
	{
		ke = inkey_ex();
		if (quick_messages) break;
		if (ke.key == ESCAPE) break;
		if (strchr("YyNn", ke.key)) break;
		bell("Illegal response to a 'yes/no' question!");
	}

	/* Kill the buttons */
	/* Restore the old buttons */
	button_restore();
	event_signal(EVENT_MOUSEBUTTONS);

	/* Erase the prompt */
	prt("", 0, 0);

	/* Normal negation */
	if ((ke.key != 'Y') && (ke.key != 'y')) return (FALSE);

	/* Success */
	return (TRUE);
}


/* TODO: refactor get_check() in terms of get_char() */
/*
 * Ask the user to respond with a character. Options is a constant string,
 * e.g. "yns"; len is the length of the constant string, and fallback should
 * be the default answer if the user hits escape or an invalid key.
 *
 * Example: get_char("Study? ", "yns", 3, 'n')
 *     This prompts "Study? [yns]" and defaults to 'n'.
 *
 */
char get_char(cptr prompt, const char *options, size_t len, char fallback)
{
	size_t i;
	char button[4], buf[80], key;
	bool repeat = FALSE;

	/* Paranoia XXX XXX XXX */
	message_flush();

	/* Hack -- Build a "useful" prompt */
	strnfmt(buf, 78, "%.70s[%s] ", prompt, options);

	/* Hack - kill the repeat button */
	if (button_kill('n')) repeat = TRUE;

	/* Make some buttons */
	for (i=0; i < len; i++)
	{
		strnfmt(button, 4, "[%c]", options[i]);
		button_add(button, options[i]);
	}
	handle_stuff();

	/* Prompt for it */
	prt(buf, 0, 0);

	/* Get an acceptable answer */
	while (TRUE)
	{
		key = inkey_ex().key;

		/* Lowercase answer if necessary */
		if (key >= 'A' && key <= 'Z') key += 32;

		/* See if key is in our options string */
		if (strchr(options, key)) break;

		/* If we want to escape, return the fallback */
		if ((key == ESCAPE) || (quick_messages))
		{
			key = fallback;
			break;
		}
		bell("Illegal response!");
	}

	/* Kill the buttons */
	for (i=0; i < len; i++) button_kill(options[i]);

	/* Hack - restore the repeat button */
	if (repeat) button_add("[RPT]", 'n');
	handle_stuff();

	/* Erase the prompt */
	prt("", 0, 0);

	/* Success */
	return key;
}


/**
 * Text-native way of getting a filename.
 */
static bool get_file_text(const char *suggested_name, char *path, size_t len)
{
	char buf[160];

	/* Get filename */
	my_strcpy(buf, suggested_name, sizeof buf);
	if (!get_string("File name: ", buf, sizeof buf)) return FALSE;

	/* Make sure it's actually a filename */
	if (buf[0] == '\0' || buf[0] == ' ') return FALSE;

	/* Build the path */
	path_build(path, len, ANGBAND_DIR_USER, buf);

	/* Check if it already exists */
	if (file_exists(buf))
	{
		char buf2[160];
		strnfmt(buf2, sizeof(buf2), "Replace existing file %s?", buf);

		if (get_check(buf2) == FALSE)
			return FALSE;
	}

	return TRUE;
}


/*
 * Give a prompt, then get a choice within a certain range.
 */
int get_menu_choice(s16b max, char *prompt)
{
	int choice = -1;

	char ch;

	bool done = FALSE;

	prt(prompt, 0, 0);

	while (!done)
	{
		ch = inkey();

		/* Letters are used for selection */
		if (isalpha((int)ch))
		{
			if (islower((int)ch))
			{
				choice = A2I(ch);
			}
			else
			{
				choice = ch - 'A' + 26;
			}

			/* Validate input */
			if ((choice > -1) && (choice < max))
			{
				done = TRUE;
			}

			else
			{
				bell("Illegal response to question!");
			}
		}

		/* Allow user to exit the function */
		else if (ch == ESCAPE)
		{
			/* Mark as no choice made */
			choice = -1;

			done = TRUE;
		}

		else if (ch == '\r')
		{
			/* Hitting return is the same as 'A' */
			choice = A2I('a');
			done = TRUE;
		}

		/* Invalid input */
		else bell("Illegal response to question!");
	}

	/* Clear the prompt */
	prt("", 0, 0);

	/* Return */
	return (choice);
}


/**
 * Get a pathname to save a file to, given the suggested name.  Returns the
 * result in "path".
 */
bool (*get_file)(const char *suggested_name, char *path, size_t len) = get_file_text;


/*
 * Prompts for a keypress
 *
 * The "prompt" should take the form "Command: "
 *
 * Returns TRUE unless the character is "Escape"
 */
bool get_com(cptr prompt, char *command)
{
	ui_event_data ke;
	bool result;

	result = get_com_ex(prompt, &ke);
	*command = ke.key;

	return result;
}


bool get_com_ex(cptr prompt, ui_event_data *command)
{
	ui_event_data ke;

	/* Paranoia XXX XXX XXX */
	message_flush();

	/* Display a prompt */
	prt(prompt, 0, 0);

	/* Get a key */
	ke = inkey_ex();

	/* Clear the prompt */
	prt("", 0, 0);

	/* Save the command */
	*command = ke;

	/* Done */
	return (ke.key != ESCAPE);
}


/*
 * Pause for user response
 *
 * This function is stupid.  XXX XXX XXX
 */
void pause_line(int row)
{
	prt("", row, 0);
	put_str("[Press any key to continue]", row, 23);
	(void)anykey();
	prt("", row, 0);
}


/*
 * Hack -- special buffer to hold the action of the current keymap
 */
static char request_command_buffer[256];


/*
 * Request a command from the user.
 *
 * Sets p_ptr->command_cmd, p_ptr->command_dir, p_ptr->command_rep,
 * p_ptr->command_arg.  May modify p_ptr->command_new.
 *
 * Note that "caret" ("^") is treated specially, and is used to
 * allow manual input of control characters.  This can be used
 * on many machines to request repeated tunneling (Ctrl-H) and
 * on the Macintosh to request "Control-Caret".
 *
 * Note that "backslash" is treated specially, and is used to bypass any
 * keymap entry for the following character.  This is useful for macros.
 *
 * Note that this command is used both in the dungeon and in
 * stores, and must be careful to work in both situations.
 *
 * Note that "p_ptr->command_new" may not work any more.  XXX XXX XXX
 */
void request_command(void)
{
	int i;
	int mode;

	char tmp[2] = { '\0', '\0' };

	ui_event_data ke = EVENT_EMPTY;

	cptr act;

	if (rogue_like_commands)
		mode = KEYMAP_MODE_ROGUE;
	else
		mode = KEYMAP_MODE_ORIG;

	/* Reset command/argument/direction */
	p_ptr->command_cmd = 0;
	p_ptr->command_arg = 0;
	p_ptr->command_dir = 0;

	/* Get command */
	while (TRUE)
	{
		/* Hack -- auto-commands */
		if (p_ptr->command_new)
		{
			/* Flush messages */
			message_flush();

			/* Use auto-command */
			ke.key = (char)p_ptr->command_new;
			ke.type = EVT_KBRD;

			/* Forget it */
			p_ptr->command_new = 0;
		}

		/* Get a keypress in "command" mode */
		else
		{
			/* Hack -- no flush needed */
			msg_flag = FALSE;

			/* Activate "command mode" */
			inkey_flag = TRUE;

			/* Get a command */
			ke = inkey_ex();
		}

		/* Clear top line */
		prt("", 0, 0);

		/* Resize events XXX XXX */
		if (ke.type == EVT_RESIZE)
		{
			p_ptr->command_cmd_ex = ke;
			p_ptr->command_new = ' ';
		}

		/* Command Count */
		if (ke.key == '0')
		{
			int old_arg = p_ptr->command_arg;

			/* Reset */
			p_ptr->command_arg = 0;

			/* Begin the input */
			prt("Count: ", 0, 0);

			/* Get a command count */
			while (TRUE)
			{
				/* Get a new keypress */
				ke.key = inkey();

				/* Simple editing (delete or backspace) */
				if ((ke.key == 0x7F) || (ke.key == KTRL('H')))
				{
					/* Delete a digit */
					p_ptr->command_arg = p_ptr->command_arg / 10;

					/* Show current count */
					prt(format("Count: %d", p_ptr->command_arg), 0, 0);
				}

				/* Actual numeric data */
				else if (isdigit((unsigned char)ke.key))
				{
					/* Stop count at 9999 */
					if (p_ptr->command_arg >= 1000)
					{
						/* Warn */
						bell("Invalid repeat count!");

						/* Limit */
						p_ptr->command_arg = 9999;
					}

					/* Increase count */
					else
					{
						/* Incorporate that digit */
						p_ptr->command_arg = p_ptr->command_arg * 10 + D2I(ke.key);
					}

					/* Show current count */
					prt(format("Count: %d", p_ptr->command_arg), 0, 0);
				}

				/* Exit on "unusable" input */
				else
				{
					break;
				}
			}

			/* Hack -- Handle "zero" */
			if (p_ptr->command_arg == 0)
			{
				/* Default to 99 */
				p_ptr->command_arg = 99;

				/* Show current count */
				prt(format("Count: %d", p_ptr->command_arg), 0, 0);
			}

			/* Hack -- Handle "old_arg" */
			if (old_arg != 0)
			{
				/* Restore old_arg */
				p_ptr->command_arg = old_arg;

				/* Show current count */
				prt(format("Count: %d", p_ptr->command_arg), 0, 0);
			}

			/* Hack -- white-space means "enter command now" */
			if ((ke.key == ' ') || (ke.key == '\n') || (ke.key == '\r'))
			{
				/* Get a real command */
				if (!get_com("Command: ", &ke.key))
				{
					/* Clear count */
					p_ptr->command_arg = 0;

					/* Continue */
					continue;
				}
			}
		}

		/* Special case for the arrow keys */
		if (isarrow(ke.key))
		{
			switch (ke.key)
			{
				case ARROW_DOWN:	ke.key = '2'; break;
				case ARROW_LEFT:	ke.key = '4'; break;
				case ARROW_RIGHT:	ke.key = '6'; break;
				case ARROW_UP:		ke.key = '8'; break;
			}
		}

		/* Allow "keymaps" to be bypassed */
		if (ke.key == '\\')
		{
			/* Get a real command */
			(void)get_com("Command: ", &ke.key);

			/* Hack -- bypass keymaps */
			if (!inkey_next) inkey_next = "";
		}

		/* Allow "control chars" to be entered */
		if (ke.key == '^')
		{
			/* Get a new command and controlify it */
			if (get_com("Control: ", &ke.key)) ke.key = KTRL(ke.key);
		}

		/* Buttons are always specified in standard keyset */
		if (ke.type == EVT_BUTTON)
		{
			act = tmp;
			tmp[0] = ke.key;
		}

		/* Look up applicable keymap */
		else
			act = keymap_act[mode][(byte)(ke.key)];

		/* Apply keymap if not inside a keymap already */
		if (act && !inkey_next)
		{
			/* Install the keymap */
			my_strcpy(request_command_buffer, act,
			          sizeof(request_command_buffer));

			/* Start using the buffer */
			inkey_next = request_command_buffer;

			/* Continue */
			continue;
		}

		/* Paranoia */
		if (ke.key == '\0') continue;

		/* Use command */
		p_ptr->command_cmd = ke.key;
		p_ptr->command_cmd_ex = ke;

		/* Done */
		break;
	}

	/* Hack -- Scan equipment */
	for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
	{
		char verify_inscrip[] = "^*";
		unsigned n;

		object_type *o_ptr = &inventory[i];

		/* Skip non-objects */
		if (!o_ptr->k_idx) continue;

		/* Set up string to look for, e.g. "^d" */
		verify_inscrip[1] = p_ptr->command_cmd;

		/* Verify command */
		n = check_for_inscrip(o_ptr, "^*") + check_for_inscrip(o_ptr, verify_inscrip);
		while (n--)
		{
			if (!get_check("Are you sure? "))
				p_ptr->command_cmd = '\n';
		}
	}

	/* Hack -- erase the message line. */
	prt("", 0, 0);

	/* Hack again -- apply the modified key command */
	p_ptr->command_cmd_ex.key = p_ptr->command_cmd;

}


/*
 * Check a char for "vowel-hood"
 */
bool is_a_vowel(int ch)
{
	switch (tolower((unsigned char) ch))
	{
		case 'a':
		case 'e':
		case 'i':
		case 'o':
		case 'u':
		{
			return (TRUE);
		}
	}

	return (FALSE);
}


/*
 * Accept a color index character; if legal, return the color.  -LM-
 *
 * Unlike Sangband, we don't translate these colours here.
 */
int color_char_to_attr(char c)
{
	switch (c)
	{
		case 'd': return (TERM_DARK);
		case 'w': return (TERM_WHITE);
		case 's': return (TERM_SLATE);
		case 'o': return (TERM_ORANGE);
		case 'r': return (TERM_RED);
		case 'g': return (TERM_GREEN);
		case 'b': return (TERM_BLUE);
		case 'u': return (TERM_UMBER);

		case 'D': return (TERM_L_DARK);
		case 'W': return (TERM_L_WHITE);
		case 'v': return (TERM_VIOLET);
		case 'y': return (TERM_YELLOW);
		case 'R': return (TERM_L_RED);
		case 'G': return (TERM_L_GREEN);
		case 'B': return (TERM_L_BLUE);
		case 'U': return (TERM_L_UMBER);
	}

	return (TERM_WHITE);
}


/*
 * Converts a string to a terminal color byte.
 */
int color_text_to_attr(cptr name)
{
	int i, len, base, shade;

	/* Optimize name searching. See below */
	static byte len_names[MAX_BASE_COLORS];

	/* Separate the color name and the shade number */
	/* Only letters can be part of the name */
	for (i = 0; isalpha((int)name[i]); i++) ;

	/* Store the start of the shade number */
	len = i;

	/* Check for invalid characters in the shade part */
	while (name[i])
	{
		/* No digit, exit */
		if (!isdigit((int)name[i])) return (-1);
		++i;
	}

	/* Initialize the shade */
	shade = 0;

	/* Only analyze the shade if there is one */
	if (name[len])
	{
		/* Convert to number */
		shade = atoi(name + len);

		/* Check bounds */
		if ((shade < 0) || (shade > MAX_SHADES - 1)) return (-1);
	}

	/* Extra, allow the use of strings like "r1", "U5", etc. */
	if (len == 1)
	{
		/* Convert one character, check sanity */
		if ((base = color_char_to_attr(name[0])) == -1) return (-1);

		/* Build the extended color */
		return (MAKE_EXTENDED_COLOR(base, shade));
	}

	/* Hack - Initialize the length array once */
	if (!len_names[0])
	{
		for (base = 0; base < MAX_BASE_COLORS; base++)
		{
			/* Store the length of each color name */
			len_names[base] = (byte)strlen(color_names[base & 0x0F]);
		}
	}

	/* Find the name */
	for (base = 0; base < MAX_BASE_COLORS; base++)
	{
		/* Somewhat optimize the search */
		if (len != len_names[base]) continue;

		/* Compare only the found name */
		if (my_strnicmp(name, color_names[base & 0x0F], len) == 0)
		{
			/* Build the extended color */
			return (MAKE_EXTENDED_COLOR(base, shade));
		}
	}

	/* We can not find it */
	return (-1);
}


static const char *short_color_names[MAX_BASE_COLORS] =
{
	"Dark",
	"White",
	"Slate",
	"Orange",
	"Red",
	"Green",
	"Blue",
	"Umber",
	"L.Dark",
	"L.Slate",
	"Violet",
	"Yellow",
	"L.Red",
	"L.Green",
	"L.Blue",
	"L.Umber"
};


/*
 * Extract a textual representation of an attribute
 */
cptr attr_to_text(byte a)
{
  const char *base;

  base = short_color_names[GET_BASE_COLOR(a)];

  return (base);
}


#ifdef SUPPORT_GAMMA

/*
 * XXX XXX XXX Important note about "colors" XXX XXX XXX
 *
 * The "TERM_*" color definitions list the "composition" of each
 * "Angband color" in terms of "quarters" of each of the three color
 * components (Red, Green, Blue), for example, TERM_UMBER is defined
 * as 2/4 Red, 1/4 Green, 0/4 Blue.
 *
 * These values are NOT gamma-corrected.  On most machines (with the
 * Macintosh being an important exception), you must "gamma-correct"
 * the given values, that is, "correct for the intrinsic non-linearity
 * of the phosphor", by converting the given intensity levels based
 * on the "gamma" of the target screen, which is usually 1.7 (or 1.5).
 *
 * The actual formula for conversion is unknown to me at this time,
 * but you can use the table below for the most common gamma values.
 *
 * So, on most machines, simply convert the values based on the "gamma"
 * of the target screen, which is usually in the range 1.5 to 1.7, and
 * usually is closest to 1.7.  The converted value for each of the five
 * different "quarter" values is given below:
 *
 *  Given     Gamma 1.0       Gamma 1.5       Gamma 1.7     Hex 1.7
 *  -----       ----            ----            ----          ---
 *   0/4        0.00            0.00            0.00          #00
 *   1/4        0.25            0.27            0.28          #47
 *   2/4        0.50            0.55            0.56          #8f
 *   3/4        0.75            0.82            0.84          #d7
 *   4/4        1.00            1.00            1.00          #ff
 */

/* Table of gamma values */
byte gamma_table[256];

/* Table of ln(x / 256) * 256 for x going from 0 -> 255 */
static const s16b gamma_helper[256] =
{
	0, -1420, -1242, -1138, -1065, -1007, -961, -921, -887, -857, -830,
	-806, -783, -762, -744, -726, -710, -694, -679, -666, -652, -640,
	-628, -617, -606, -596, -586, -576, -567, -577, -549, -541, -532,
	-525, -517, -509, -502, -495, -488, -482, -475, -469, -463, -457,
	-451, -455, -439, -434, -429, -423, -418, -413, -408, -403, -398,
	-394, -389, -385, -380, -376, -371, -367, -363, -359, -355, -351,
	-347, -343, -339, -336, -332, -328, -325, -321, -318, -314, -311,
	-308, -304, -301, -298, -295, -291, -288, -285, -282, -279, -276,
	-273, -271, -268, -265, -262, -259, -257, -254, -251, -248, -246,
	-243, -241, -238, -236, -233, -231, -228, -226, -223, -221, -219,
	-216, -214, -212, -209, -207, -205, -203, -200, -198, -196, -194,
	-192, -190, -188, -186, -184, -182, -180, -178, -176, -174, -172,
	-170, -168, -166, -164, -162, -160, -158, -156, -155, -153, -151,
	-149, -147, -146, -144, -142, -140, -139, -137, -135, -134, -132,
	-130, -128, -127, -125, -124, -122, -120, -119, -117, -116, -114,
	-112, -111, -109, -108, -106, -105, -103, -102, -100, -99, -97, -96,
	-95, -93, -92, -90, -89, -87, -86, -85, -83, -82, -80, -79, -78,
	-76, -75, -74, -72, -71, -70, -68, -67, -66, -65, -63, -62, -61,
	-59, -58, -57, -56, -54, -53, -52, -51, -50, -48, -47, -46, -45,
	-44, -42, -41, -40, -39, -38, -37, -35, -34, -33, -32, -31, -30,
	-29, -27, -26, -25, -24, -23, -22, -21, -20, -19, -18, -17, -16,
	-14, -13, -12, -11, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1
};


/*
 * Build the gamma table so that floating point isn't needed.
 *
 * Note gamma goes from 0->256.  The old value of 100 is now 128.
 */
void build_gamma_table(int gamma)
{
	int i, n;

	/*
	 * value is the current sum.
	 * diff is the new term to add to the series.
	 */
	long value, diff;

	/* Hack - convergence is bad in these cases. */
	gamma_table[0] = 0;
	gamma_table[255] = 255;

	for (i = 1; i < 255; i++)
	{
		/*
		 * Initialise the Taylor series
		 *
		 * value and diff have been scaled by 256
		 */
		n = 1;
		value = 256L * 256L;
		diff = ((long)gamma_helper[i]) * (gamma - 256);

		while (diff)
		{
			value += diff;
			n++;

			/*
			 * Use the following identity to calculate the gamma table.
			 * exp(x) = 1 + x + x^2/2 + x^3/(2*3) + x^4/(2*3*4) +...
			 *
			 * n is the current term number.
			 *
			 * The gamma_helper array contains a table of
			 * ln(x/256) * 256
			 * This is used because a^b = exp(b*ln(a))
			 *
			 * In this case:
			 * a is i / 256
			 * b is gamma.
			 *
			 * Note that everything is scaled by 256 for accuracy,
			 * plus another factor of 256 for the final result to
			 * be from 0-255.  Thus gamma_helper[] * gamma must be
			 * divided by 256*256 each iteration, to get back to
			 * the original power series.
			 */
			diff = (((diff / 256) * gamma_helper[i]) * (gamma - 256)) / (256 * n);
		}

		/*
		 * Store the value in the table so that the
		 * floating point pow function isn't needed.
		 */
		gamma_table[i] = ((long)(value / 256) * i) / 256;
	}
}

#endif /* SUPPORT_GAMMA */


/*
 * Accept values for y and x (considered as the endpoints of lines) between
 * 0 and 40, and return an angle in degrees (divided by two).  -LM-
 *
 * This table's input and output need some processing:
 *
 * Because this table gives degrees for a whole circle, up to radius 20, its
 * origin is at (x,y) = (20, 20).  Therefore, the input code needs to find
 * the origin grid (where the lines being compared come from), and then map
 * it to table grid 20,20.  Do not, however, actually try to compare the
 * angle of a line that begins and ends at the origin with any other line -
 * it is impossible mathematically, and the table will return the value "255".
 *
 * The output of this table also needs to be massaged, in order to avoid the
 * discontinuity at 0/180 degrees.  This can be done by:
 *   rotate = 90 - first value
 *   this rotates the first input to the 90 degree line)
 *   tmp = ABS(second value + rotate) % 180
 *   diff = ABS(90 - tmp) = the angular difference (divided by two) between
 *   the first and second values.
 *
 * Note that grids diagonal to the origin have unique angles.
 */
byte get_angle_to_grid[41][41] =
{
  {  68,  67,  66,  65,  64,  63,  62,  62,  60,  59,  58,  57,  56,  55,  53,  52,  51,  49,  48,  46,  45,  44,  42,  41,  39,  38,  37,  35,  34,  33,  32,  31,  30,  28,  28,  27,  26,  25,  24,  24,  23 },
  {  69,  68,  67,  66,  65,  64,  63,  62,  61,  60,  59,  58,  56,  55,  54,  52,  51,  49,  48,  47,  45,  43,  42,  41,  39,  38,  36,  35,  34,  32,  31,  30,  29,  28,  27,  26,  25,  24,  24,  23,  22 },
  {  69,  69,  68,  67,  66,  65,  64,  63,  62,  61,  60,  58,  57,  56,  54,  53,  51,  50,  48,  47,  45,  43,  42,  40,  39,  37,  36,  34,  33,  32,  30,  29,  28,  27,  26,  25,  24,  24,  23,  22,  21 },
  {  70,  69,  69,  68,  67,  66,  65,  64,  63,  61,  60,  59,  58,  56,  55,  53,  52,  50,  48,  47,  45,  43,  42,  40,  38,  37,  35,  34,  32,  31,  30,  29,  27,  26,  25,  24,  24,  23,  22,  21,  20 },
  {  71,  70,  69,  69,  68,  67,  66,  65,  63,  62,  61,  60,  58,  57,  55,  54,  52,  50,  49,  47,  45,  43,  41,  40,  38,  36,  35,  33,  32,  30,  29,  28,  27,  25,  24,  24,  23,  22,  21,  20,  19 },
  {  72,  71,  70,  69,  69,  68,  67,  65,  64,  63,  62,  60,  59,  58,  56,  54,  52,  51,  49,  47,  45,  43,  41,  39,  38,  36,  34,  32,  31,  30,  28,  27,  26,  25,  24,  23,  22,  21,  20,  19,  18 },
  {  73,  72,  71,  70,  69,  69,  68,  66,  65,  64,  63,  61,  60,  58,  57,  55,  53,  51,  49,  47,  45,  43,  41,  39,  37,  35,  33,  32,  30,  29,  27,  26,  25,  24,  23,  22,  21,  20,  19,  18,  17 },
  {  73,  73,  72,  71,  70,  70,  69,  68,  66,  65,  64,  62,  61,  59,  57,  56,  54,  51,  49,  47,  45,  43,  41,  39,  36,  34,  33,  31,  29,  28,  26,  25,  24,  23,  21,  20,  20,  19,  18,  17,  17 },
  {  75,  74,  73,  72,  72,  71,  70,  69,  68,  66,  65,  63,  62,  60,  58,  56,  54,  52,  50,  47,  45,  43,  40,  38,  36,  34,  32,  30,  28,  27,  25,  24,  23,  21,  20,  19,  18,  18,  17,  16,  15 },
  {  76,  75,  74,  74,  73,  72,  71,  70,  69,  68,  66,  65,  63,  61,  59,  57,  55,  53,  50,  48,  45,  42,  40,  37,  35,  33,  31,  29,  27,  25,  24,  23,  21,  20,  19,  18,  17,  16,  16,  15,  14 },
  {  77,  76,  75,  75,  74,  73,  72,  71,  70,  69,  68,  66,  64,  62,  60,  58,  56,  53,  51,  48,  45,  42,  39,  37,  34,  32,  30,  28,  26,  24,  23,  21,  20,  19,  18,  17,  16,  15,  15,  14,  13 },
  {  78,  77,  77,  76,  75,  75,  74,  73,  72,  70,  69,  68,  66,  64,  62,  60,  57,  54,  51,  48,  45,  42,  39,  36,  33,  30,  28,  26,  24,  23,  21,  20,  18,  17,  16,  15,  15,  14,  13,  13,  12 },
  {  79,  79,  78,  77,  77,  76,  75,  74,  73,  72,  71,  69,  68,  66,  63,  61,  58,  55,  52,  49,  45,  41,  38,  35,  32,  29,  27,  24,  23,  21,  19,  18,  17,  16,  15,  14,  13,  13,  12,  11,  11 },
  {  80,  80,  79,  79,  78,  77,  77,  76,  75,  74,  73,  71,  69,  68,  65,  63,  60,  57,  53,  49,  45,  41,  37,  33,  30,  27,  25,  23,  21,  19,  17,  16,  15,  14,  13,  13,  12,  11,  11,  10,  10 },
  {  82,  81,  81,  80,  80,  79,  78,  78,  77,  76,  75,  73,  72,  70,  68,  65,  62,  58,  54,  50,  45,  40,  36,  32,  28,  25,  23,  20,  18,  17,  15,  14,  13,  12,  12,  11,  10,  10,   9,   9,   8 },
  {  83,  83,  82,  82,  81,  81,  80,  79,  79,  78,  77,  75,  74,  72,  70,  68,  64,  60,  56,  51,  45,  39,  34,  30,  26,  23,  20,  18,  16,  15,  13,  12,  11,  11,  10,   9,   9,   8,   8,   7,   7 },
  {  84,  84,  84,  83,  83,  83,  82,  81,  81,  80,  79,  78,  77,  75,  73,  71,  68,  63,  58,  52,  45,  38,  32,  27,  23,  19,  17,  15,  13,  12,  11,  10,   9,   9,   8,   7,   7,   7,   6,   6,   6 },
  {  86,  86,  85,  85,  85,  84,  84,  84,  83,  82,  82,  81,  80,  78,  77,  75,  72,  68,  62,  54,  45,  36,  28,  23,  18,  15,  13,  12,  10,   9,   8,   8,   7,   6,   6,   6,   5,   5,   5,   4,   4 },
  {  87,  87,  87,  87,  86,  86,  86,  86,  85,  85,  84,  84,  83,  82,  81,  79,  77,  73,  68,  58,  45,  32,  23,  17,  13,  11,   9,   8,   7,   6,   6,   5,   5,   4,   4,   4,   4,   3,   3,   3,   3 },
  {  89,  88,  88,  88,  88,  88,  88,  88,  88,  87,  87,  87,  86,  86,  85,  84,  83,  81,  77,  68,  45,  23,  13,   9,   7,   6,   5,   4,   4,   3,   3,   3,   2,   2,   2,   2,   2,   2,   2,   2,   1 },
  {  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90, 255,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 },
  {  91,  92,  92,  92,  92,  92,  92,  92,  92,  93,  93,  93,  94,  94,  95,  96,  97,  99, 103, 113, 135, 158, 167, 171, 173, 174, 175, 176, 176, 177, 177, 177, 178, 178, 178, 178, 178, 178, 178, 178, 179 },
  {  93,  93,  93,  93,  94,  94,  94,  94,  95,  95,  96,  96,  97,  98,  99, 101, 103, 107, 113, 122, 135, 148, 158, 163, 167, 169, 171, 172, 173, 174, 174, 175, 175, 176, 176, 176, 176, 177, 177, 177, 177 },
  {  94,  94,  95,  95,  95,  96,  96,  96,  97,  98,  98,  99, 100, 102, 103, 105, 108, 113, 118, 126, 135, 144, 152, 158, 162, 165, 167, 168, 170, 171, 172, 172, 173, 174, 174, 174, 175, 175, 175, 176, 176 },
  {  96,  96,  96,  97,  97,  97,  98,  99,  99, 100, 101, 102, 103, 105, 107, 109, 113, 117, 122, 128, 135, 142, 148, 153, 158, 161, 163, 165, 167, 168, 169, 170, 171, 171, 172, 173, 173, 173, 174, 174, 174 },
  {  97,  97,  98,  98,  99,  99, 100, 101, 101, 102, 103, 105, 106, 108, 110, 113, 116, 120, 124, 129, 135, 141, 146, 150, 154, 158, 160, 162, 164, 165, 167, 168, 169, 169, 170, 171, 171, 172, 172, 173, 173 },
  {  98,  99,  99, 100, 100, 101, 102, 102, 103, 104, 105, 107, 108, 110, 113, 115, 118, 122, 126, 130, 135, 140, 144, 148, 152, 155, 158, 160, 162, 163, 165, 166, 167, 168, 168, 169, 170, 170, 171, 171, 172 },
  { 100, 100, 101, 101, 102, 103, 103, 104, 105, 106, 107, 109, 111, 113, 115, 117, 120, 123, 127, 131, 135, 139, 143, 147, 150, 153, 155, 158, 159, 161, 163, 164, 165, 166, 167, 167, 168, 169, 169, 170, 170 },
  { 101, 101, 102, 103, 103, 104, 105, 106, 107, 108, 109, 111, 113, 114, 117, 119, 122, 125, 128, 131, 135, 139, 142, 145, 148, 151, 153, 156, 158, 159, 161, 162, 163, 164, 165, 166, 167, 167, 168, 169, 169 },
  { 102, 103, 103, 104, 105, 105, 106, 107, 108, 110, 111, 113, 114, 116, 118, 120, 123, 126, 129, 132, 135, 138, 141, 144, 147, 150, 152, 154, 156, 158, 159, 160, 162, 163, 164, 165, 165, 166, 167, 167, 168 },
  { 103, 104, 105, 105, 106, 107, 108, 109, 110, 111, 113, 114, 116, 118, 120, 122, 124, 127, 129, 132, 135, 138, 141, 143, 146, 148, 150, 152, 154, 156, 158, 159, 160, 161, 162, 163, 164, 165, 165, 166, 167 },
  { 104, 105, 106, 106, 107, 108, 109, 110, 111, 113, 114, 115, 117, 119, 121, 123, 125, 127, 130, 132, 135, 138, 140, 143, 145, 147, 149, 151, 153, 155, 156, 158, 159, 160, 161, 162, 163, 164, 164, 165, 166 },
  { 105, 106, 107, 108, 108, 109, 110, 111, 113, 114, 115, 117, 118, 120, 122, 124, 126, 128, 130, 133, 135, 137, 140, 142, 144, 146, 148, 150, 152, 153, 155, 156, 158, 159, 160, 161, 162, 162, 163, 164, 165 },
  { 107, 107, 108, 109, 110, 110, 111, 113, 114, 115, 116, 118, 119, 121, 123, 124, 126, 129, 131, 133, 135, 137, 139, 141, 144, 146, 147, 149, 151, 152, 154, 155, 156, 158, 159, 160, 160, 161, 162, 163, 163 },
  { 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 119, 120, 122, 123, 125, 127, 129, 131, 133, 135, 137, 139, 141, 143, 145, 147, 148, 150, 151, 153, 154, 155, 156, 158, 159, 159, 160, 161, 162, 163 },
  { 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 120, 121, 122, 124, 126, 128, 129, 131, 133, 135, 137, 139, 141, 142, 144, 146, 148, 149, 150, 152, 153, 154, 155, 157, 158, 159, 159, 160, 161, 162 },
  { 109, 110, 111, 112, 113, 114, 114, 115, 117, 118, 119, 120, 122, 123, 125, 126, 128, 130, 131, 133, 135, 137, 139, 140, 142, 144, 145, 147, 148, 150, 151, 152, 153, 155, 156, 157, 158, 159, 159, 160, 161 },
  { 110, 111, 112, 113, 114, 114, 115, 116, 117, 119, 120, 121, 122, 124, 125, 127, 128, 130, 132, 133, 135, 137, 138, 140, 142, 143, 145, 146, 148, 149, 150, 151, 153, 154, 155, 156, 157, 158, 159, 159, 160 },
  { 111, 112, 113, 114, 114, 115, 116, 117, 118, 119, 120, 122, 123, 124, 126, 127, 129, 130, 132, 133, 135, 137, 138, 140, 141, 143, 144, 146, 147, 148, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 159 },
  { 112, 113, 114, 114, 115, 116, 117, 118, 119, 120, 121, 122, 124, 125, 126, 128, 129, 131, 132, 133, 135, 137, 138, 139, 141, 142, 144, 145, 146, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159 },
  { 113, 114, 114, 115, 116, 117, 118, 118, 120, 121, 122, 123, 124, 125, 127, 128, 129, 131, 132, 134, 135, 136, 138, 139, 141, 142, 143, 145, 146, 147, 148, 149, 150, 152, 152, 153, 154, 155, 156, 157, 158 }
};


/*
 * Calculates and returns the angle to the target or in the given
 * direction.
 *
 * Note:  If a compass direction is supplied, we ignore any target.
 * Note:  We supply the angle divided by 2.
 */
int get_angle_to_target(int y0, int x0, int y1, int x1, int dir)
{
	int ny, nx;
	int dist_conv;

	/* No valid compass direction given */
	if ((dir == 0) || (dir == 5) || (dir > 9))
	{
		/* Check for a valid target */
		if ((y1) && (x1))
		{
			/* Get absolute distance between source and target */
			int dy = ABS(y1 - y0);
			int dx = ABS(x1 - x0);

			/* Calculate distance conversion factor */
			if ((dy > 20) || (dx > 20))
			{
				/* Must shrink the distance to avoid illegal table access */
				if (dy > dx) dist_conv = 1 + (10 * dy / 20);
				else         dist_conv = 1 + (10 * dx / 20);
			}
			else
			{
				dist_conv = 10;
			}
			/* Convert and reorient grid for table access */
			ny = 20 + 10 * (y1 - y0) / dist_conv;
			nx = 20 + 10 * (x1 - x0) / dist_conv;

			/* Illegal table access is bad */
			if ((ny < 0) || (ny > 40) || (nx < 0) || (nx > 40))
			{
				/* Note error */
				return (-1);
			}
		}

		/* No compass direction and no target --> note error */
		else
		{
			return (-1);
		}
	}

	/* We have a valid compass direction */
	else
	{
		/* Step in that direction a bunch of times, get target */
		y1 = y0 + (ddy_ddd[dir] * 10);
		x1 = x0 + (ddx_ddd[dir] * 10);

		/* Convert to table grids */
		ny = 20 + (y1 - y0);
		nx = 20 + (x1 - x0);
	}

	/* Get angle to target. */
	return (get_angle_to_grid[ny][nx]);
}


/*
 * Using the angle given, find a grid that is in that direction from the
 * origin.
 *
 * Note:  This function does not yield very good results when the
 * character is adjacent to the outer wall of the dungeon and the projection
 * heads towards it.
 */
void get_grid_using_angle(int angle, int y0, int x0, int *ty, int *tx)
{
	int y, x;
	int best_y = 0, best_x = 0;

	int diff;
	int this_angle;
	int fudge = 180;

	/* Angle must be legal */
	if ((angle < 0) || (angle >= 180)) return;

	/* Scan the table, get as good a match as possible */
	for (y = 0; y < 41; y++)
	{
		for (x = 0; x < 41; x++)
		{
			/* Corresponding grid in dungeon must be fully in bounds  XXX */
			if (!in_bounds_fully(y0 - 20 + y, x0 - 20 + x)) continue;

			/* Check this table grid */
			this_angle = get_angle_to_grid[y][x];

			/* Get inaccuracy of this angle */
			diff = ABS(angle - this_angle);

			/* Inaccuracy is lower than previous best */
			if (diff < fudge)
			{
				/* Note coordinates */
				best_y = y;
				best_x = x;

				/* Save inaccuracy as a new best */
				fudge = diff;

				/* Note perfection */
				if (fudge == 0) break;
			}
		}

		/* Note perfection */
		if (fudge == 0) break;
	}

	/* We have an unacceptably large fudge factor */
	if (fudge >= 30)
	{
		/* Set target to original grid */
		*ty = y0;
		*tx = x0;
	}

	/* Usual case */
	else
	{
		/* Set target */
		*ty = y0 - 20 + best_y;
		*tx = x0 - 20 + best_x;
	}
}


/*
 * Returns a string which contains the name of a extended color.
 * Examples: "Dark", "Red1", "Yellow5", etc.
 * IMPORTANT: the returned string is statically allocated so it must *not* be
 * freed and its value changes between calls to this function.
 */
cptr get_ext_color_name(byte ext_color)
{
	static char buf[25];

	if (GET_SHADE(ext_color) > 0)
	{
		strnfmt(buf, sizeof(buf), "%s%d", color_names[GET_BASE_COLOR(ext_color)],
		GET_SHADE(ext_color));
	}
	else
	{
		strnfmt(buf, sizeof(buf), "%s", color_names[GET_BASE_COLOR(ext_color)]);
	}

	return buf;
}


/*
 * Create a new grid queue. "q" must be a pointer to an uninitialized
 * grid_queue_type structure. That structure must be already allocated
 * (it can be in the system stack).
 * You must supply the maximum number of grids for the queue
 */
void grid_queue_create(grid_queue_type *q, size_t max_size)
{
	/* Remember the maximum size */
	q->max_size = max_size;

	/* Allocate the grid storage */
	q->data = C_ZNEW(max_size, coord);

	/* Initialize head and tail of the queue */
	q->head = q->tail = 0;
}


/*
 * Free the resources used by a queue
 */
void grid_queue_destroy(grid_queue_type *q)
{
	/* Free the allocated grid storage */
	if (q->data)
	{
		FREE(q->data);
	}

	/* Clear all */
	WIPE(q, grid_queue_type);
}


/*
 * Append a grid at the tail of the queue, given the coordinates of that grid.
 * Returns FALSE if the queue is full, or TRUE on success.
 */
bool grid_queue_push(grid_queue_type *q, byte y, byte x)
{
	/* Check space */
	if (GRID_QUEUE_FULL(q)) return (FALSE);

	/* Append the grid */
	q->data[q->tail].y = y;
	q->data[q->tail].x = x;

	/*
	 * Update the tail of the queue.
	 * The queue is circular, note tail adjustment
	 */
	q->tail = (q->tail + 1) % q->max_size;

	/* Success */
	return (TRUE);
}


/*
 * Remove the grid at the front of the queue
 */
void grid_queue_pop(grid_queue_type *q)
{
	/* Something to pop? */
	if (GRID_QUEUE_EMPTY(q)) return;

	/*
	 * Update the head of the queue.
	 * The queue is circular, note head adjustment
	 */
	q->head = (q->head + 1) % q->max_size;
}


/*
 * Return a random index of the given array based on the weights contained in it
 * Each index can have a different weight and bigger values are more probable
 * to be picked
 * Return -1 on error
 */
int pick_random_item(int chance_values[], int max)
{
	int total_chance = 0;
	int rand_chance;
	int i;

	/* Paranoia */
	if (max < 1) return (-1);

	/* Get the sum of the chances */
	for (i = 0; i < max; i++)
	{
		/* Paranoia */
		if (chance_values[i] < 0) chance_values[i] = 0;

		/* Update the total */
		total_chance += chance_values[i];
	}

	/* Paranoia */
	if (total_chance == 0) return (0);

	/* Get a random chance value */
	rand_chance = rand_int(total_chance);

	/* Reset the counter */
	total_chance = 0;

	/* Get the respective index of that chance */
	for (i = 0; i < max; i++)
	{
		/* Update the total chance again */
		total_chance += chance_values[i];

		/* The random chance is contained in this entry */
		if (rand_chance < total_chance) break;
	}

	/* Paranoia */
	if (i >= max) i = max - 1;

	/* Return the index */
	return (i);
}

int effective_depth(int depth)
{
	/* in Quickband this is different */
	return depth;
}

