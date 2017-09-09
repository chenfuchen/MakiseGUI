#include "makise_text.h"
#include <stdio.h>

static void _makise_draw_char(MakiseBuffer *b, uint16_t ind, int16_t x, int16_t y, const MakiseFont *font, uint32_t c, uint16_t width)
{
    uint32_t bitCounter, rawIndex, colIndex;
    const uint8_t * ptrByte;

    if(b->border.ex + b->border.w < x ||
       b->border.ey + b->border.h < y ||
       x <= b->border.x - width ||
       y <= b->border.y - font->height)
	return;
    
    ptrByte = &font->table[font->char_index[ind]];
    bitCounter = 0;
    for(rawIndex = 0; rawIndex < font->height; rawIndex++)
    {
	for(colIndex = 0; colIndex < width; colIndex++)
	{
	    if (bitCounter > 7)
	    {
		bitCounter = 0;
		ptrByte++;
	    }
	    if(*ptrByte & (1<<bitCounter))
	    {
		makise_pset(b, x+colIndex, y+rawIndex, c);
	    }
	    bitCounter++;
	}
    }
}

void makise_d_char(MakiseBuffer *b, uint16_t ch, int16_t x, int16_t y, const MakiseFont *font, uint32_t c)
{
    uint32_t width;

    ch = (uint8_t)ch - font->offset;
    
    // Symbol width
    if (ch > font->num_char) ch = 0;
    width = font->width ? font->width : font->char_width[ch];

    // Draw Char
    _makise_draw_char(b, ch, x, y, font, c, width);
}

void makise_d_string(MakiseBuffer *b,
		     char *s, uint32_t len,
		     int16_t x, int16_t y, MDTextPlacement place,
		     const MakiseFont *font, uint32_t c)
{
    uint32_t width, i = 0;

    if(s == 0)
	return;
    
    if(place == MDTextPlacement_Center )
    {
	width = makise_d_string_width(s, len, font);
	x -= width / 2;
	y -= font->height / 2;
    } else if(place == MDTextPlacement_CenterUp )
    {
	width = makise_d_string_width(s, len, font);
	x -= width / 2;
    } else if(place == MDTextPlacement_CenterDown )
    {
	width = makise_d_string_width(s, len, font);
	x -= width / 2;
	y -= font->height;
    }
    
    uint32_t ch, xt = x, yt = y;

    if(y + font->height < b->border.y ||
       y > b->border.ey) //borders
	return;
    
    while (i < len && s[i]) {
#if MAKISE_UNICODE
	uint8_t bts = 0;
	ch = makise_d_utf_char_id(&s[i], len - i, &bts);
	ch = makise_d_utf_char_font(ch, font);
	if (ch > font->num_uni) ch = 0;
	i += bts;
#else
	ch = s[i];
	ch = (uint8_t)ch - font->offset;
	if (ch > font->num_char) ch = 0;
	i++;
#endif
	// Symbol width
	width = font->width ? font->width : font->char_width[ch];

	// Draw Char
	_makise_draw_char(b, ch, xt, yt, font, c, width);
	xt += width + font->space_char;

	if(xt >= b->border.ex) //border
	    return;
    }
}

uint32_t makise_d_string_width(char *s, uint32_t len, const MakiseFont *font)
{
    uint32_t width , i = 0;
    uint32_t ch, res = 0;

    if(s == 0)
	return 0;
    
    while (i < len && s[i]) {
#if MAKISE_UNICODE
	uint8_t bts = 0;
	ch = makise_d_utf_char_id(&s[i], len - i, &bts);
	ch = makise_d_utf_char_font(ch, font);
	if (ch > font->num_uni) ch = 0;
	i += bts;
#else
	ch = s[i];
	ch = (uint8_t)ch - font->offset;
	if (ch > font->num_char) ch = 0;
	i++;
#endif

	// Symbol width
	//if (ch > font->num_char) ch = 0;
	width = font->width ? font->width : font->char_width[ch];
	res += width + font->space_char;

    }
    return res;
}

uint32_t makise_d_string_height_get ( char*             s,
                                      uint32_t          len,
                                      uint16_t          width_window,
                                      const MakiseFont* font,
                                      uint32_t          font_line_spacing ) {
    uint32_t i = 0;
    uint32_t height = font->height;
    uint32_t width = 0;
    while ( i < len && s[i] ) {

#if MAKISE_UNICODE
        uint8_t bts = 0;
        uint32_t ch;
        ch = makise_d_utf_char_id( &s[i], len - i, &bts );
        ch = makise_d_utf_char_font(ch, font);
        if ( ch > font->num_uni ) ch = 0;
        i += bts;
#else
        ch = s[i];
        ch = (uint8_t)ch - font->offset;
        if (ch > font->num_char) ch = 0;
        i++;
#endif
        uint32_t width_char = font->width ? font->width : font->char_width[ch];

        if ( width + width_char + font->space_char > width_window ) {
            width = width_char;
            height += font->height + font_line_spacing;
        } else {
            width += width_char + font->space_char;
        }
    }
    return height;
}

/**
 * Get total line count of the text in the frame with selected width.
 * Methods calculates line wraps and new line returns.
 *
 * @param s text
 * @param len length of text
 * @param w width of frame
 * @param font 
 * @return count of text lines
 */
uint32_t    makise_d_string_get_line_count (
                                     char *s,
                                     uint32_t len,
                                     uint16_t w,
                                     const MakiseFont *font )
{
    uint32_t width, i = 0;

    uint32_t ch, xt = 0, lines = 1;

    if(s == 0)
	return 0;

    while ( i < len && s[i] ) {
	if(s[i] == '\n' || s[i] == '\r')
	{
	    //calculate return
	    xt = 0;
	    lines ++;
	    i++;
	}
	else
	{
#if MAKISE_UNICODE
	    uint8_t bts = 0;
	    ch = makise_d_utf_char_id( &s[i], len - i, &bts );
	    ch = makise_d_utf_char_font(ch, font);
	    if ( ch > font->num_uni ) ch = 0;
	    i += bts;
#else
	    ch = s[i];
	    ch = (uint8_t)ch - font->offset;
	    if (ch > font->num_char) ch = 0;
	    i++;
#endif
	    width = font->width ? font->width : font->char_width[ch];
	    
	    if(xt + width > w)
	    {
		xt = 0;
		lines ++;
	    }
	    xt += width + font->space_char;
	}
    }
    return lines;
}


/**
 * Returns pointer to the n's line. It calculates returns, wraps and etc.
 *
 * @param s string
 * @param len string's len
 * @param line required line
 * @param w width of frame
 * @param font font
 * @return pointer tobeginning og n's line
 */
char *     makise_d_string_get_line (
                                     char *s,
                                     uint32_t len,
				     uint32_t n,
                                     uint16_t w,
                                     const MakiseFont *font )
{
    uint32_t width, i = 0;

    uint32_t ch, xt = 0, lines = 0;

    if(s == 0)
	return s;
    if(n == 0)
	return s;

    while ( i < len && s[i] ) {
	if(s[i] == '\n' || s[i] == '\r')
	{
	    //calculate return
	    xt = 0;
	    lines ++;
	    i++;
	    if(lines == n)
		return &s[i];
	}
	else
	{
#if MAKISE_UNICODE
	    uint8_t bts = 0;
	    ch = makise_d_utf_char_id( &s[i], len - i, &bts );
	    ch = makise_d_utf_char_font(ch, font);
	    if ( ch > font->num_uni ) ch = 0;
#else
	    ch = s[i];
	    ch = (uint8_t)ch - font->offset;
	    if (ch > font->num_char) ch = 0;
#endif
	    width = font->width ? font->width : font->char_width[ch];
	    
	    if(xt + width > w)
	    {
		xt = 0;
		lines ++;
		if(lines == n)
		    return &s[i];
	    }
#if MAKISE_UNICODE
	    i += bts;
#else
	    i++;
#endif

	    xt += width + font->space_char;
	}
    }
    return 0;
}

//draw multiline text in the defined frame
void makise_d_string_frame(MakiseBuffer *b, char *s, uint32_t len, int16_t x, int16_t y, uint16_t w, uint16_t h, const MakiseFont *font, uint16_t line_spacing, uint32_t c)
{
    uint32_t width, i = 0;

    uint32_t ch, xt = x, yt = y;

    if(s == 0)
	return;

    
    while (i < len && s[i])
    {
	if(s[i] == '\n' || s[i] == '\r')
	{
	    xt = x;
	    yt += font->height + line_spacing;
	    i++;
	    if(yt + font->height > y + h)
		return;
	}
	else
	{
#if MAKISE_UNICODE
	    uint8_t bts = 0;
	    ch = makise_d_utf_char_id(&s[i], len - i, &bts);
	    ch = makise_d_utf_char_font(ch, font);
	    if (ch > font->num_uni) ch = 0;
	    i += bts;
#else
	    ch = s[i];
	    ch = (uint8_t)ch - font->offset;
	    if (ch > font->num_char) ch = 0;
	    i++;
#endif

	    // Symbol width
	    width = font->width ? font->width : font->char_width[ch];

	    if(xt + width > x + w)
	    {
		xt = x;

		yt += font->height + line_spacing;
		if(yt + font->height > y + h)
		    return;
	    }
	    // Draw Char
	    _makise_draw_char(b, ch, xt, yt, font, c, width);
	    xt += width + font->space_char;
	}
    }

}

#if MAKISE_UNICODE
static const struct validUTF8Sequence
{
    uint32_t  lowChar;
    uint32_t highChar;
    uint16_t numBytes;
    uint8_t validBytes[8];
} validUTF8[7] =
{
/*   low       high   #bytes  byte 1      byte 2      byte 3      byte 4 */
    {0x0000,   0x007F,   1, {0x00, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},
    {0x0080,   0x07FF,   2, {0xC2, 0xDF, 0x80, 0xBF, 0x00, 0x00, 0x00, 0x00}},
    {0x0800,   0x0FFF,   3, {0xE0, 0xE0, 0xA0, 0xBF, 0x80, 0xBF, 0x00, 0x00}},
    {0x1000,   0xFFFF,   3, {0xE1, 0xEF, 0x80, 0xBF, 0x80, 0xBF, 0x00, 0x00}},
    {0x10000,  0x3FFFF,  4, {0xF0, 0xF0, 0x90, 0xBF, 0x80, 0xBF, 0x80, 0xBF}},
    {0x40000,  0xFFFFF,  4, {0xF1, 0xF3, 0x80, 0xBF, 0x80, 0xBF, 0x80, 0xBF}},
    {0x100000, 0x10FFFF, 4, {0xF4, 0xF4, 0x80, 0x8F, 0x80, 0xBF, 0x80, 0xBF}} 
};

uint32_t    makise_d_utf_char_id  ( char *s, uint32_t len, uint8_t *bts )
{
    if(len == 0)
    {
	*bts = 0;
	return 0;
    }
    if(((uint8_t*)s)[0] < 128)
    {
	*bts = 1;
	return s[0];
    }

    uint8_t ch = s[0];
    uint32_t n = 0;
    uint16_t bytes = 0, i;
    uint8_t hasError = 0;
    
    
    if (ch <= 0x7F) /* 0XXX XXXX one byte */
    {
        n = ch;
        bytes = 1;
    }
    else if ((ch & 0xE0) == 0xC0)  /* 110X XXXX  two bytes */
    {
        n = ch & 31;
        bytes = 2;
    }
    else if ((ch & 0xF0) == 0xE0)  /* 1110 XXXX  three bytes */
    {
        n = ch & 15;
        bytes = 3;
    }
    else if ((ch & 0xF8) == 0xF0)  /* 1111 0XXX  four bytes */
    {
        n = ch & 7;
        bytes = 4;
    }
    else if ((ch & 0xFC) == 0xF8)  /* 1111 10XX  five bytes */
    {
        n = ch & 3;
        bytes = 5;
        hasError = 1;
    }
    else if ((ch & 0xFE) == 0xFC)  /* 1111 110X  six bytes */
    {
        n = ch & 1;
        bytes = 6;
        hasError = 1;
    }
    else
    {
        /* not a valid first byte of a UTF-8 sequence */
        n = ch;
        bytes = 1;
        hasError = 1;
    }

    if(len < bytes)
    {
	*bts = 1;
	return s[0];
    }
    
    for ( i=1; i < bytes; i++ )
    {
	if ( !s[i] || (s[i] & 0xC0) != 0x80 )
	{
	    hasError = 1;
	    bytes = i;
	    break;
	}
	n = (n << 6) | (s[i] & 0x3F);
    }

    *bts = bytes;
    
    if(hasError)
	return 0;

    return n;
    
}

uint32_t    makise_d_utf_char_font  ( uint32_t c, const MakiseFont *font)
{
    //if character is in the first block
    if(c < font->offset + font->num_char)
	return c - font->offset;

    for (uint16_t i = 0; i < font->num_uni - font->num_char; i++) {
	if((uint16_t)c == font->unicode_index[i])
	{
	    //printf("f %d %d\n", c, i);
	    return i + font->num_char;
	}
    }
    return UINT32_MAX;
}

#endif //unicode

static void uint_to_2_char ( uint32_t* number, char* const p ) {
    *p          = *number / 10 + '0';
    *(p + 1)    = *number % 10 + '0';
}

// Convert the time represented in seconds to hours:minutes:seconds (HH:MM:SS).
void convert_time_sec_to_char ( uint32_t time_sec, char* array ) {
    const uint32_t one_min  = 60;
    const uint32_t one_hour = 60 * one_min;

    // Convert sec on hours, minutes and seconds.
    uint32_t hour       = 0;
    uint32_t min        = 0;
    uint32_t sec        = time_sec;

    hour      = sec / one_hour;
    sec      -= hour * one_hour;

    min       = sec / one_min;
    sec      -= min * one_min;

    uint_to_2_char( &hour, &array[0] );
    uint_to_2_char( &min, &array[3] );
    uint_to_2_char( &sec, &array[6] );
}

