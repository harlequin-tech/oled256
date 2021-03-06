#ifndef FONTHQ_H_
#define FONTHQ_H_

// Font selection
#undef FONT_CHECKBOOK_12
#define FONT_CHECKBOOK_14

typedef struct {
    uint8_t width;		// Width of glyph data in pixels
    uint8_t xrect;		// x width of rectangle
    uint8_t yrect;		// y height of rectangle
    int8_t xoffset;		// x offset of glyph in rectangle
    int8_t yoffset;		// y offset of glyph in rectangle
    const uint8_t *glyph;	// glyph pixel data
} glyph_t;

typedef struct {
    const glyph_t *glyphs;
    const uint8_t *map;
    uint8_t height;
} font_t;

#ifdef FONT_CHECKBOOK_12
#include "font_checkbook_12.h"
#endif
#ifdef FONT_CHECKBOOK_14
#include "font_checkbook_14.h"
#endif

typedef enum {
#ifdef FONT_CHECKBOOK_12
    font_CHECKBOOK_12,
#endif
#ifdef FONT_CHECKBOOK_14
    font_CHECKBOOK_14,
#endif
} font_e;

extern font_t fontsHQ[];

#endif
