/* File: z-term.h */

/*
 * Copyright (c) 1997 Ben Harrison
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.
 */

#ifndef INCLUDED_Z_TERM_H
#define INCLUDED_Z_TERM_H

#include "h-basic.h"
#include "ui-event.h"


typedef struct term_win term_win;

/*
 * A term_win is a "window" for a Term
 *
 * Note that the attr/char pair at (x,y) is a[y][x]/c[y][x]
 * and that the row of attr/chars at (0,y) is a[y]/c[y]
 */
struct term_win
{
	bool cu, cv;	/* Cursor Useless/Visible codes */
	byte cx, cy;	/* Cursor location - valid only when cu is false */

	byte **a;		/* Array[h] -- Access to the attribute array */
	char **c;		/* Array[h] -- Access to the character array */

	byte *va;		/* Array[h*w] -- Attribute array */
	char *vc;		/* Array[h*w] -- Character array */

	byte **ta;		/* Array[h] -- Access to the terrain attribute array */
	char **tc;		/* Array[h] -- Access to the terrain character array */

	byte *vta;		/* Array[h*w] -- Terrain attribute array */
	char *vtc;		/* Array[h*w] -- Terrain character array */

	term_win *next;	/* next screen saved */
};


typedef struct term term;

/*
 * An actual "term" structure
 */
struct term
{
	void *user;			/* Extra "user" info (used by application) */

	void *data;			/* Extra "data" info (used by implementation) */

	bool user_flag;		/* An extra "user" flag (used by application) */

	bool data_flag;		/* An extra "data" flag (used by implementation) */

	bool active_flag;	/* This "term" is "active" */
	bool mapped_flag;	/* This "term" is "mapped" */
	bool total_erase;	/* This "term" should be fully erased */

	/* Term capability and limitation flags */
	bool fixed_shape;	/* This "term" is not allowed to resize */
	bool icky_corner;	/* This "term" has an "icky" corner grid (writing to
						   the grid in the lower right corner causes the
						   screen to scroll) */
	bool soft_cursor;	/* This "term" uses a "software" cursor */
	bool always_pict;	/* Use the "Term_pict()" routine for all text */
	bool higher_pict;	/* Use the "Term_pict()" routine for special text */
	bool always_text;	/* Use the "Term_text()" routine for invisible text */
	bool unused_flag;	/* Reserved for future use */
	bool never_bored;	/* Never call the "TERM_XTRA_BORED" action */
	bool never_frosh;	/* Never call the "TERM_XTRA_FROSH" action */

	byte attr_blank;	/* Use this "attr" value for "blank" grids */
	char char_blank;	/* Use this "char" value for "blank" grids */

	ui_event_data *key_queue;	/* Ignore this pointer */

	/* Keypress Queue -- pending keys */
	u16b key_head;
	u16b key_tail;
	/* Keypress Queue -- various data */
	u16b key_xtra;
	u16b key_size;

	byte wid;			/* Window Width (max 255) */
	byte hgt;			/* Window Height (max 255) */

	byte y1;			/* Minimum modified row */
	byte y2;			/* Maximum modified row */

	byte *x1;			/* Minimum modified column (per row) */
	byte *x2;			/* Maximum modified column (per row) */

	/* Offsets used by the map subwindows */
	byte offset_x;
	byte offset_y;

	term_win *old;		/* Displayed screen image */
	term_win *scr;		/* Requested screen image */

	term_win *tmp;		/* Temporary screen image */
	term_win *mem;		/* Memorized screen image */

	/* Hook for init-ing the term */
	void (*init_hook)(term *t);
	/* Hook for nuke-ing the term */
	void (*nuke_hook)(term *t);

	/* Hook for user actions */
	errr (*user_hook)(int n);

	/* Hook for extra actions */
	errr (*xtra_hook)(int n, int v);

	/* Hook for placing the cursor */
	errr (*curs_hook)(int x, int y);

	/* Hook for placing the cursor */
	errr (*bigcurs_hook)(int x, int y);

	/* Hook for drawing some blank spaces */
	errr (*wipe_hook)(int x, int y, int n);

	/* Hook for drawing a string of chars using an attr */
	errr (*text_hook)(int x, int y, int n, byte a, cptr s);

	/* Hook for drawing a sequence of special attr/char pairs */
	errr (*pict_hook)(int x, int y, int n, const byte *ap, const char *cp, const byte *tap, const char *tcp);

	/* Hook for translating Latin-1 (8-bit) characters */
	byte (*xchar_hook)(byte c);
};


/**** Available Constants ****/


/*
 * Definitions for the "actions" of "Term_xtra()"
 *
 * These values may be used as the first parameter of "Term_xtra()",
 * with the second parameter depending on the "action" itself.  Many
 * of the actions shown below are optional on at least one platform.
 *
 * The "TERM_XTRA_EVENT" action uses "v" to "wait" for an event
 * The "TERM_XTRA_SHAPE" action uses "v" to "show" the cursor
 * The "TERM_XTRA_FROSH" action uses "v" for the index of the row
 * The "TERM_XTRA_ALIVE" action uses "v" to "activate" (or "close")
 * The "TERM_XTRA_LEVEL" action uses "v" to "resume" (or "suspend")
 * The "TERM_XTRA_DELAY" action uses "v" as a "millisecond" value
 *
 * The other actions do not need a "v" code, so "zero" is used.
 */
#define TERM_XTRA_EVENT	1	/* Process some pending events */
#define TERM_XTRA_FLUSH	2	/* Flush all pending events */
#define TERM_XTRA_CLEAR	3	/* Clear the entire window */
#define TERM_XTRA_SHAPE	4	/* Set cursor shape (optional) */
#define TERM_XTRA_FROSH	5	/* Flush one row (optional) */
#define TERM_XTRA_FRESH	6	/* Flush all rows (optional) */
#define TERM_XTRA_NOISE	7	/* Make a noise (optional) */
#define TERM_XTRA_BORED	9	/* Handle stuff when bored (optional) */
#define TERM_XTRA_REACT	10	/* React to global changes (optional) */
#define TERM_XTRA_ALIVE	11	/* Change the "hard" level (optional) */
#define TERM_XTRA_LEVEL	12	/* Change the "soft" level (optional) */
#define TERM_XTRA_DELAY	13	/* Delay some milliseconds (optional) */


/*
 * Angband "attributes" (with symbols, and base (R,G,B) codes)
 *
 * The "(R,G,B)" codes are given in "fourths" of the "maximal" value,
 * and should "gamma corrected" on most (non-Macintosh) machines.
 */

#define TERM_DARK			0	/* 'd' */	/* 0,0,0 */
#define TERM_WHITE			1	/* 'w' */	/* 4,4,4 */
#define TERM_SLATE			2	/* 's' */	/* 2,2,2 */
#define TERM_ORANGE			3	/* 'o' */	/* 4,2,0 */
#define TERM_RED			4	/* 'r' */	/* 3,0,0 */
#define TERM_GREEN			5	/* 'g' */	/* 0,2,1 */
#define TERM_BLUE			6	/* 'b' */	/* 0,0,4 */
#define TERM_UMBER			7	/* 'u' */	/* 2,1,0 */
#define TERM_L_DARK			8	/* 'D' */	/* 1,1,1 */
#define TERM_L_WHITE		9	/* 'W' */	/* 3,3,3 */
#define TERM_VIOLET			10	/* 'v' */	/* 4,0,4 */
#define TERM_YELLOW			11	/* 'y' */	/* 4,4,0 */
#define TERM_L_RED			12	/* 'R' */	/* 4,0,0 */
#define TERM_L_GREEN		13	/* 'G' */	/* 0,4,0 */
#define TERM_L_BLUE			14	/* 'B' */	/* 0,4,4 */
#define TERM_L_UMBER		15	/* 'U' */	/* 3,2,1 */
/*  16 is unused  */
#define TERM_SNOW_WHITE		17		/* 'w1'*/
#define TERM_SLATE_GRAY		18		/* 's1'*/
#define TERM_ORANGE_PEEL 	19		/* 'o1' */
#define TERM_RED_LAVA		20		/* 'r1' */
#define TERM_JUNGLE_GREEN	21		/* 'g1' */
#define TERM_NAVY_BLUE		22		/* 'b1' */
#define TERM_AUBURN			23		/* 'u1' */
#define TERM_TAUPE			24		/* 'D1' */
#define TERM_L_WHITE_2		25		/* 'W1' */
#define TERM_D_PURPLE		26		/* 'v1' */
#define TERM_MAIZE			27		/* 'Y1' */
#define TERM_RASPBERRY		28		/* 'R1' */
#define TERM_LIME_GREEN		29		/* 'G1' */
#define TERM_SKY_BLUE		30		/* 'B1' */
#define TERM_L_BROWN		31		/* 'U1' */
/*  32 is unused  */
/*  33 is unused  */
#define TERM_SILVER			34		/* 's2' */
#define TERM_MAHAGONY		35		/* 'o2' */
#define TERM_RED_RUST		36		/* 'r2' */
/*  37 is unused  */
/*  38 is unused  */
#define TERM_COPPER			39		/* 'u2' */
/*  40 is unused  */
/*  41 is unused  */
/*  42 is unused  */
#define TERM_GOLD			43		/* 'Y2' */
#define TERM_PINK			44		/* 'R2' */
/*  45 is unused  */
/*  46 is unused  */
#define TERM_EARTH_YELLOW	47		/* 'U2' */

/*
 * 47 and beyond are unused
 * Note that if a color is added beyond TERM_EARTH_YELLOW, then
 * MAX_COLOR_USED in defines.h needs to be modified
 */


/* The following allow color 'translations' to support environments with a limited color depth
 * as well as translate colors to alternates for e.g. menu highlighting. */

#define ATTR_FULL		0	/* full color translation */
#define ATTR_MONO		1	/* mono color translation */
#define ATTR_VGA		2	/* 16 color translation */
#define ATTR_BLIND		3	/* "Blind" color translation */
#define ATTR_LIGHT		4	/* "Torchlit" color translation */
#define ATTR_DARK		5	/* "Dark" color translation */
#define ATTR_HIGH		6	/* "Highlight" color translation */
#define ATTR_METAL		7	/* "Metallic" color translation */
#define ATTR_MISC		8	/* "Miscellaneous" color translation - see misc_to_attr */

#define MAX_ATTR		9


/**** Available Variables ****/

extern term *Term;


/**** Available Functions ****/

extern errr Term_user(int n);
extern errr Term_xtra(int n, int v);

extern void Term_queue_char(term *t, int x, int y, byte a, char c, byte ta, char tc);
extern void Term_queue_chars(int x, int y, int n, byte a, cptr s);

extern errr Term_fresh(void);
extern errr Term_set_cursor(bool v);
extern errr Term_gotoxy(int x, int y);
extern errr Term_draw(int x, int y, byte a, char c);
extern errr Term_addch(byte a, char c);
extern errr Term_addstr(int n, byte a, cptr buf);
extern errr Term_putch(int x, int y, byte a, char c);
extern errr Term_putstr(int x, int y, int n, byte a, cptr s);
extern errr Term_erase(int x, int y, int n);
extern errr Term_clear(void);
extern errr Term_redraw(void);
extern errr Term_redraw_section(int x1, int y1, int x2, int y2);

extern errr Term_get_cursor(bool *v);
extern errr Term_get_size(int *w, int *h);
extern errr Term_locate(int *x, int *y);
extern errr Term_what(int x, int y, byte *a, char *c);

extern errr Term_flush(void);
extern errr Term_mousepress(int x, int y, char button);
extern errr Term_keypress(int k);
extern errr Term_key_push(int k);
extern errr Term_event_push(const ui_event_data *ke);
extern errr Term_inkey(ui_event_data *ch, bool wait, bool take);

extern errr Term_save(void);
extern errr Term_load(void);

extern errr Term_resize(int w, int h);

extern errr Term_activate(term *t);

extern errr term_nuke(term *t);
extern errr term_init(term *t, int w, int h, int k);


#endif

