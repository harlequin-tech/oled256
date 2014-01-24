#ifndef _FONTS_H
#define _FONTS_H

/* select desired fonts. (Simply comment out those not needed) */
#undef EN_FIVE_DOT
#undef EN_SIX_DOT
#undef EN_SEVEN_DOT
#define EN_NINE_DOT
#define EN_TEN_DOT
#undef EN_FIFTEEN_DOT
#undef EN_EIGHTEEN_DOT
#define EN_FONT_MT1

/* define number labels for the font selections */
typedef enum
{
#ifdef EN_FIVE_DOT
	FONT_FIVE_DOT=0,
#endif

#ifdef EN_SIX_DOT
	FONT_SIX_DOT,
#endif

#ifdef EN_SEVEN_DOT
	FONT_SEVEN_DOT,
#endif

#ifdef EN_NINE_DOT
	FONT_NINE_DOT,
#endif

#ifdef EN_TEN_DOT
	FONT_TEN_DOT,
#endif

#ifdef EN_FIFTEEN_DOT
	FONT_FIFTEEN_DOT,
#endif

#ifdef EN_EIGHTEEN_DOT
	FONT_EIGHTEEN_DOT,
#endif

#ifdef EN_FONT_MT1
	FONT_MT1,
#endif

	FONT_COUNT
} FONT_BASE;

struct FONT_DEF 
{
   uint8_t store_width;            /* glyph storage width in bytes */
   uint8_t glyph_height;  		 /* glyph height for storage */
   const uint8_t *glyph_table;      /* font table start address in memory */
   uint8_t fixed_width;            /* fixed width of glyphs. If zero */
                                         /* then use the width table. */
   const uint8_t *width_table; 	 /* variable width table start adress */
   uint8_t glyph_beg;			 	 /* start ascii offset in table */
   uint8_t glyph_end;				 /* end ascii offset in table */
   uint8_t glyph_def;				 /*  for undefined glyph  */
   const uint8_t *map;		/* ascii map */
};

/* font definition tables for the fonts */
extern const struct FONT_DEF fonts[FONT_COUNT];

/* glyph bitmap and width tables for the fonts */ 
#ifdef EN_FIVE_DOT
  extern const uint8_t five_dot_glyph_table[];
  extern const uint8_t five_dot_width_table[];
#endif

#ifdef EN_SIX_DOT
  extern const uint8_t six_dot_glyph_table[];
  extern const uint8_t six_dot_width_table[];
#endif

#ifdef EN_SEVEN_DOT
#define DEG_CHAR ('~'+1)
  extern const uint8_t seven_dot_glyph_table[];
  extern const uint8_t seven_dot_width_table[];
#endif

#ifdef EN_NINE_DOT
  extern const uint8_t  nine_dot_glyph_table[];
#endif

#ifdef EN_TEN_DOT
  extern const uint8_t  ten_dot_glyph_table[];
#endif

#ifdef EN_FIFTEEN_DOT
  extern const uint8_t  fifteen_dot_glyph_table[];
  extern const uint8_t  fifteen_dot_width_table[];
#endif

#ifdef EN_EIGHTEEN_DOT
  extern const uint8_t  eighteen_dot_glyph_table[];
  extern const uint8_t  eighteen_dot_width_table[];
#endif

#ifdef EN_FONT_MT1
  extern const uint8_t  font_mt1_glyph_table[];
  extern const uint8_t  font_mt1_width_table[];
#endif

#endif
