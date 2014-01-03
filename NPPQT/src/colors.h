#ifndef COLORS_H
#define COLORS_H

/*
 * Number of base colors. These are the TERM_* constants
 */

#define MAX_BASE_COLORS	16
#define BASIC_COLORS	MAX_BASE_COLORS

#define MAX_COLORS			128
#define MAX_COLOR_USED		(TERM_EARTH_YELLOW + 1)

/*
 * Number of shades, including the shade 0 (base colors)
 */
#define MAX_SHADES		8


/*
 * Get the index of a shaded color in angband_color_table.
 * base_color must be between 0 and 15. These are the TERM_* constants.
 * shade must be between 0 and 7 (8 shades supported, 0 is the base color).
 * 16 shades could be supported but the values returned are in the range 0-127
 * to avoid clashes with graphic or big_tile modes (bit 0x80 is reserved)
 * All shades will look like the base color in 16 color ports.
 */
#define MAKE_EXTENDED_COLOR(base_color, shade) \
((((shade) << 4) | ((base_color) & 0x0F)) & 0x7F)

/*
 * Get the base color of a given extended color (shade).
 * Values returned are in the range 0-15 (TERM_*). See above.
 */
#define GET_BASE_COLOR(ext_color) ((ext_color) & 0x0F)

/*
 * Get the shade number of a given extended color.
 * Values returned are in the range 0-7. See above.
 */
#define GET_SHADE(ext_color) (((ext_color) >> 4) & 0x07)

/*
 * Angband "attributes" (with symbols, and base (R,G,B) codes)
 *
 * The "(R,G,B)" codes are given in "fourths" of the "maximal" value,
 * and should "gamma corrected" on most (non-Macintosh) machines.
 */

#define TERM_DARK		0	/* 'd' */	/* 0,0,0 */
#define TERM_WHITE		1	/* 'w' */	/* 4,4,4 */
#define TERM_SLATE		2	/* 's' */	/* 2,2,2 */
#define TERM_ORANGE		3	/* 'o' */	/* 4,2,0 */
#define TERM_RED		4	/* 'r' */	/* 3,0,0 */
#define TERM_GREEN		5	/* 'g' */	/* 0,2,1 */
#define TERM_BLUE		6	/* 'b' */	/* 0,0,4 */
#define TERM_UMBER		7	/* 'u' */	/* 2,1,0 */
#define TERM_L_DARK		8	/* 'D' */	/* 1,1,1 */
#define TERM_L_WHITE	9	/* 'W' */	/* 3,3,3 */
#define TERM_VIOLET		10	/* 'v' */	/* 4,0,4 */
#define TERM_YELLOW		11	/* 'y' */	/* 4,4,0 */
#define TERM_L_RED		12	/* 'R' */	/* 4,0,0 */
#define TERM_L_GREEN	13	/* 'G' */	/* 0,4,0 */
#define TERM_L_BLUE		14	/* 'B' */	/* 0,4,4 */
#define TERM_L_UMBER	15	/* 'U' */	/* 3,2,1 */
/*  16 is unused  */
#define TERM_SNOW_WHITE  	17   	/* 'w1'*/
#define TERM_SLATE_GRAY  	18   	/* 's1'*/
#define TERM_ORANGE_PEEL 	19	 	/* 'o1' */
#define TERM_RED_LAVA    	20	 	/* 'r1' */
#define TERM_JUNGLE_GREEN	21	 	/* 'g1' */
#define TERM_NAVY_BLUE		22		/* 'b1' */
#define TERM_AUBURN			23		/* 'u1' */
#define TERM_TAUPE			24		/* 'D1' */
#define TERM_L_WHITE_2	    25		/* 'W1' */
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
#define TERM_RED_RUST    	36	 	/* 'r2' */
/*  37 is unused  */
/*  38 is unused  */
#define TERM_COPPER	    	39	 	/* 'u2' */
/*  40 is unused  */
/*  41 is unused  */
/*  42 is unused  */
#define TERM_GOLD	    	43	 	/* 'Y2' */
#define TERM_PINK	    	44	 	/* 'R2' */
/*  45 is unused  */
/*  46 is unused  */
#define TERM_EARTH_YELLOW  	47	 	/* 'U2' */


#endif // COLORS_H
