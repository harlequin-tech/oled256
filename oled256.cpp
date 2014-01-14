/*-
 * Copyright (c) 2014 Darran Hunt (darran [at] hunt dot net dot nz)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
 * THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <SPI.h>
#include <oled256.h>

#include <avr/pgmspace.h>
/* Work around a bug with PROGMEM and PSTR where the compiler always
 * generates warnings.
 */
#undef PROGMEM 
#define PROGMEM __attribute__(( section(".progmem.data") )) 
#undef PSTR 
#define PSTR(s) (__extension__({static prog_char __c[] PROGMEM = (s); &__c[0];})) 

#define pinLow(port, pin)	*port &= ~pin
#define pinHigh(port, pin)	*port |= pin

#define MIN_SEG 28
#define MAX_SEG 91

#define OLED_WIDTH 256
#define OLED_HEIGHT 64


oled256::oled256(const uint8_t cs, const uint8_t dc, const uint8_t reset)
{
    _cs = cs;
    _dc = dc;
    _reset = reset;
    foreground = 15;
    background = 0;
    cur_x = 0;
    cur_y = 0;
    end_x = 0;
    end_y = 0;
    wrap = true;
    _offset = 0;
    _bufHeight = LCDHEIGHT;
}

void oled256::setColour(uint8_t colour)
{
    foreground = colour & 0x0F;
}

void oled256::setBackground(uint8_t colour)
{
    background = colour & 0x0F;
}

void oled256::setContrast(uint8_t contrast)
{
    writeCommand(CMD_SET_CONTRAST_CURRENT);
    writeData(contrast);
}

size_t oled256::write(uint8_t ch)
{
    if (ch == '\n') {
	cur_y += glyphHeight();
	cur_x = 0;
    } else if (ch == '\r') {
	// skip em
    } else {
	uint8_t width = glyphWidth(ch);
	if (wrap && ((cur_x + width) > OLED_WIDTH)) {
	    cur_y += glyphHeight();
	    cur_x = 0;
	}
	cur_x += glyphDraw(cur_x, cur_y, ch, foreground, background);
    }
    return 1;
}


size_t oled256::write(const char *buf)
{

    size_t size=0;

    while (*buf) {
	size += write(*buf++);
    }

    return size;
}

size_t oled256::write(const uint8_t *buf, size_t size)
{
    while (size--) {
	write(*buf++);
    }

    return size;
}

void oled256::setXY(uint8_t col, uint8_t row)
{
    cur_x = col;
    cur_y = row;
}

void oled256::printXY(uint8_t col, uint8_t row, const char *string)
{
    setXY(col, row);
    print(string);
}

void oled256::begin(uint8_t font)
{
    _font = font;
    port_cs = portOutputRegister(digitalPinToPort(_cs));
    pin_cs = digitalPinToBitMask(_cs);
    port_dc = portOutputRegister(digitalPinToPort(_dc));
    pin_dc = digitalPinToBitMask(_dc);

    pinMode(_cs, OUTPUT);
    pinMode(_dc, OUTPUT);
    pinMode(_reset, OUTPUT);
    digitalWrite(_cs,HIGH);

    reset();
    init();
}

void oled256::init()
{

    writeCommand(CMD_SET_COMMAND_LOCK);
    writeData(0x12); /* Unlock OLED driver IC*/

    writeCommand(CMD_SET_DISPLAY_OFF);

    writeCommand(CMD_SET_CLOCK_DIVIDER);
    writeData(0x91);

    writeCommand(CMD_SET_MULTIPLEX_RATIO);
    writeData(0x3F); /*duty = 1/64*,64 COMS are enabled*/

    writeCommand(CMD_SET_DISPLAY_OFFSET);
    writeData(0x00);

    writeCommand(CMD_SET_DISPLAY_START_LINE); /*set start line position*/
    writeData(0x00);

    writeCommand(CMD_SET_REMAP);
    writeData(0x14);	//Horizontal address increment,Disable Column Address Re-map,Enable Nibble Re-map,Scan from COM[N-1] to COM0,Disable COM Split Odd Even
    writeData(0x11);	//Enable Dual COM mode

    /*writeCommand(0xB5); //GPIO

    writeCommand(0x00); */

    writeCommand(CMD_SET_FUNCTION_SELECTION);
    writeData(0x01); /* selection external VDD */

    writeCommand(CMD_DISPLAY_ENHANCEMENT);
    writeData(0xA0);	/*enables the external VSL*/
    writeData(0xfd);	/*0xfd,Enhanced low GS display quality;default is 0xb5(normal),*/

    writeCommand(CMD_SET_CONTRAST_CURRENT);
    writeData(0xff); /* 0xff */	/*default is 0x7f*/

    writeCommand(CMD_MASTER_CURRENT_CONTROL); 
    writeData(0x0f);	/*default is 0x0f*/

    /* writeCommand(0xB9); GRAY TABLE,linear Gray Scale*/

    writeCommand(CMD_SET_PHASE_LENGTH);
    writeData(0xE2);	 /*default is 0x74*/

    writeCommand(CMD_DISPLAY_ENHANCEMENT_B);
    writeData(0x82);	 /*Reserved;default is 0xa2(normal)*/
    writeData(0x20);

    writeCommand(CMD_SET_PRECHARGE_VOLTAGE);
    writeData(0x1F);	 /*0.6xVcc*/

    writeCommand(CMD_SET_SECOND_PRECHARGE_PERIOD);
    writeData(0x08);	 /*default*/

    writeCommand(CMD_SET_VCOMH_VOLTAGE	);
    writeData(0x07);	 /*0.86xVcc;default is 0x04*/

    writeCommand(CMD_SET_DISPLAY_MODE_NORMAL);

    writeCommand(CMD_SET_DISPLAY_ON);
}

void oled256::writeCommand(uint8_t reg)
{
    pinLow(port_cs, pin_cs);
    pinLow(port_dc, pin_dc);
    SPI.transfer(reg);
    pinHigh(port_cs, pin_cs);
}

void oled256::writeData(uint8_t data)
{
    pinLow(port_cs, pin_cs);
    pinHigh(port_dc, pin_dc);
    SPI.transfer(data);
    pinHigh(port_cs, pin_cs);
}

void oled256::setColumnAddr(uint8_t start, uint8_t end)
{
    writeCommand(CMD_SET_COLUMN_ADDR);
    writeData(start);
    writeData(end);
}

void oled256::setRowAddr(uint8_t start, uint8_t end)
{
    writeCommand(CMD_SET_ROW_ADDR);
    writeData(start);
    writeData(end);
}

void oled256::setWindow(uint8_t x, uint8_t y, uint8_t xend, uint8_t yend)
{
    setColumnAddr(MIN_SEG + x / 4, MIN_SEG + xend / 4);
    setRowAddr(y, yend);
    cur_x = x;
    end_x = xend;
    cur_y = y;
    end_y = yend;
}

void oled256::setOffset(uint8_t offset)
{
    writeCommand(CMD_SET_DISPLAY_OFFSET);
    writeData(offset);
    _offset = offset;
}

uint8_t oled256::getOffset(void)
{
    return _offset;
}

void oled256::setBufHeight(uint8_t rows)
{
    if (rows < LCDHEIGHT) {
	return;
    }
    writeCommand(CMD_SET_MULTIPLEX_RATIO);
    writeData(rows & 0x7F);
    _bufHeight = rows;
}

uint8_t oled256::getBufHeight(void)
{
    return _bufHeight;
}

#define CMD_SET_DISPLAY_OFFSET		0xA2

void oled256::fill(uint8_t colour)
{
    uint8_t x,y;
    setColumnAddr(MIN_SEG, MAX_SEG);	// SEG0 - SEG479
    setRowAddr(0, 63);	

    colour = (colour & 0x0F) | (colour << 4);;

    writeCommand(CMD_WRITE_RAM);
    for(y=0; y<64; y++)
    {
	for(x=0; x<64; x++) {
	    writeData(colour);
	    writeData(colour);
	}
    }
    delay(1);
}

void oled256::clear()
{
    fill(background);
}

void oled256::reset()
{
    digitalWrite(_reset,LOW);
    delay(10);
    digitalWrite(_reset,HIGH);
    delay(10);
}

uint8_t oled256::glyphWidth(char ch)
{
    uint8_t glyph_width=0;

    if ((ch < pgm_read_byte(&fonts[_font].glyph_beg)) || (ch > pgm_read_byte(&fonts[_font].glyph_end))) {
	uint8_t *map = (uint8_t *)pgm_read_word(&fonts[_font].map);
	if (map != 0) {
	    ch = pgm_read_byte(&map[(uint8_t)ch]);
	} else {
	    ch = pgm_read_byte(&fonts[_font].glyph_def);
	}
    }

    /* make zero based index into the font data arrays */
    ch -= pgm_read_byte(&fonts[_font].glyph_beg);
    glyph_width = pgm_read_byte(&fonts[_font].fixed_width);	/* check if it is a fixed width */
    if (glyph_width == 0) {
	uint8_t *width_table = (uint8_t *)pgm_read_word(&fonts[_font].width_table);	/* get the variable width instead */
	glyph_width = pgm_read_byte(&width_table[(uint8_t)ch]);	/* get the variable width instead */
    }

    return glyph_width;
}

uint8_t oled256::glyphHeight()
{
    return pgm_read_byte(&fonts[_font].glyph_height);
}

/*
 * @returns width of glyph
 */
uint8_t oled256::glyphDraw(uint16_t x, uint16_t y, char ch, uint16_t colour, uint16_t bg)
{
    uint8_t pix;
    uint32_t bits=0xAA55AA55;
    int ind;
    uint8_t *glyph;
    uint16_t glyph_width;
    uint16_t glyph_height;
    uint16_t glyph_byte_width;
    if (colour == bg) {
	bg = 0;
    }

    /* check to make sure the symbol is a legal one */
    /* if not then just replace it with the default character */
    if ((ch < pgm_read_byte(&fonts[_font].glyph_beg)) || (ch > pgm_read_byte(&fonts[_font].glyph_end)))
    {
	uint8_t *map = (uint8_t *)pgm_read_word(&fonts[_font].map);
	if (map != 0) {
	    ch = pgm_read_byte(&map[(uint8_t)ch]);
	} else {
	    ch = pgm_read_byte(&fonts[_font].glyph_def);
	}
    }

    /* make zero based index into the font data arrays */
    ch -= pgm_read_byte(&fonts[_font].glyph_beg);
    glyph_width = pgm_read_byte(&fonts[_font].fixed_width);	/* check if it is a fixed width */
    if (glyph_width == 0) {
	uint8_t *width_table = (uint8_t *)pgm_read_word(&fonts[_font].width_table);	/* get the variable width instead */
	glyph_width = pgm_read_byte(&width_table[(uint8_t)ch]);	/* get the variable width instead */
    }

    glyph_height = pgm_read_byte(&fonts[_font].glyph_height);
    glyph_byte_width = pgm_read_byte(&fonts[_font].store_width);

    uint8_t scratch[glyph_height][glyph_byte_width];

    /* fill scratch */
    if (((cur_x & 0x3) == 0) || (cur_col == 0)) {
	/* nothing to the left */
	for (ind=0; ind<glyph_height; ind++) {
	    scratch[ind][0] = 0;
	}
    } else {
	/* Get existing glyph column */
	char lchar = display[cur_col - 1][cur_row];
	glyph = (uint8_t *)pgm_read_word(&fonts[_font].glyph_table) + lchar * glyph_byte_width * glyph_height;
	for (uint16_t yind=0; yind<glyph_height; yind++) {
	    scratch[ind][0] = pgm_read_byte(&glyph[yind*glyph_byte_width + glyph_byte_width-1]);
	}
    }

    glyph = (uint8_t *)pgm_read_word(&fonts[_font].glyph_table) + ch * glyph_byte_width * glyph_height;

    setWindow(x, y, x+glyph_width-1, y+glyph_height-1);
    writeCommand(CMD_WRITE_RAM);

    for (uint16_t yind=0; yind<glyph_height; yind++) {
	ind = (uint8_t)(yind*glyph_byte_width);
	switch (glyph_byte_width) {
	    case 1:
		bits = (uint32_t)pgm_read_byte(&glyph[ind]) << 24;
		break;
	    case 2:
		bits = (uint32_t)pgm_read_byte(&glyph[ind]) << 24 | (uint32_t)pgm_read_byte(&glyph[ind+1]) << 16;
		break;
	    case 3:
		bits = (uint32_t)pgm_read_byte(&glyph[ind]) << 24 | (uint32_t)pgm_read_byte(&glyph[ind+1]) << 16 | (uint32_t)pgm_read_byte(&glyph[ind+2]) << 8;
		break;
	    case 4:
		bits = (uint32_t)pgm_read_byte(&glyph[ind]) << 24 | (uint32_t)pgm_read_byte(&glyph[ind+1]) << 16 | (uint32_t)pgm_read_byte(&glyph[ind+2]) << 8 | (uint32_t)pgm_read_byte(&glyph[ind+3]);
		break;
	}

	// Only works for fixed width, multiple of 4 in size...
	for (pix=0; pix<glyph_width; pix+=2) {
	    uint8_t seg;
	    seg = (bits & 0x80000000) ? colour << 4 : bg << 4;
	    seg |= (bits & 0x40000000) ? colour : bg;
	    bits <<= 2;
	    writeData(seg);
	}
    }

    return (uint8_t)glyph_width;
}

LcdDisplay::LcdDisplay(const uint8_t cs, const uint8_t dc, const uint8_t reset) : oled256(cs, dc, reset) {
}

void LcdDisplay::begin(uint8_t cols, uint8_t rows, uint8_t font)
{
    oled256::begin(font);
    clear();
    _cols = cols;
    _rows = rows;
}

void LcdDisplay::setCursor(int16_t x, int16_t y)
{
    setXY(x * glyphWidth('0'), y * glyphHeight());
}

void LcdDisplay::createChar(uint8_t ch_id, uint8_t *data)
{
    if (ch_id > 7) 
	return;

    for (uint8_t ind=0; ind<7; ind++) {
	userChar[ch_id][ind] = data[ind];
    }
}


size_t LcdDisplay::write(uint8_t ch)
{
    if (ch > 7) {
	return oled256::write(ch);
    } else {
	/* Custom character */

	/* check for wrap */
	if (wrap && ((cur_x + 8) > OLED_WIDTH)) {
	    cur_y += glyphHeight();
	    cur_x = 0;
	}
	/* Draw the custom character using the current colours */
	setWindow(cur_x, cur_y, cur_x+7, cur_y+7);
	writeCommand(CMD_WRITE_RAM);
	for (uint8_t ind=0; ind<8; ind++) {
	    uint8_t line = userChar[ch][ind];
	    for (uint8_t bit=0x80; bit; bit>>=2) {
		uint8_t seg;
		seg = (line & 0x80) ? foreground << 4 : background << 4;
		seg |= (line & 0x40) ? foreground : background;
		line <<= 2;
		writeData(seg);
	    }
	}
	cur_x += 8;
	return 1;
    }
}