#ifndef __EF_FREETYPE_H__
#define __EF_FREETYPE_H__

#include <ef/type.h>
#include <ef/utf8.h>
#include <ef/rbhash.h>
#include <ef/image.h>
#include <freetype2/ft2build.h>
#include FT_FREETYPE_H

#define FT_FONT_NAME_SIZE 128

typedef FT_Library ftlib_h;

typedef struct ftRender{
	utf_t utf;
	g2dImage_s* img;
	unsigned width;
	unsigned height;
	unsigned pitch;
	unsigned horiAdvance;
	int horiBearingX;
	int horiBearingY;
	unsigned linearHoriAdvance;
	unsigned vertAdvance;
	unsigned vertBearingX;
	unsigned vertBearingY;
	unsigned linearVertAdvance;
	long descender;
	int penAdvance;
}ftRender_s;

typedef struct ftFont{
	const char* fname;
	char name[FT_FONT_NAME_SIZE];
	FT_Face face;
	unsigned long width;
	unsigned long height;
	long descender;
	long ascender;
	long advanceX;
	long advanceY;
}ftFont_s;

typedef struct ftFonts{
	ftFont_s* font;    /**< first is font, sucessive is fallback*/
	rbhash_s* charmap;
}ftFonts_s;

typedef void (*ftUtfCustom_f)(utf_t u, ftFonts_s** fonts, unsigned* x, unsigned* y, g2dColor_t* fore, void* userdata);

typedef struct ftUtfCustom{
	utf_t utf;
	ftUtfCustom_f fn;
	void* userdata;
}ftUtfCustom_s;
	
#define FT_RENDER_ANTIALIASED 0x1
#define FT_RENDER_VERT        0x2
#define FT_RENDER_VALID       0x4
#define FT_RENDER_BYTE		  0x8

//pitch number byte for line
#define ft_buf_line(Y,P) ((Y)*(P))
#define ft_buf_mono_get(buf,X,Y,P) ((buf[ft_buf_line(Y,P)] << (X)) & 0x80)
#define ft_pixel_byte_get(PX) ((PX)/8)
#define ft_pixel_bit_get(PX)  ((PX)%8)
#define ft_pixel_bitval_get(BUF, PX) (BUF[ft_pixel_byte_get(PX)] & (1<<(7-ft_pixel_bit_get(PX))));

/** before use ft*/
err_t ft_begin(void);

/** end use ft*/
void ft_end(void);

/** create new fonts obj*/
ftFonts_s* ft_fonts_new(void);

/** search loaded font by name*/
ftFont_s* ft_fonts_search_name(ftFonts_s* fonts, const char* name);

/** get index of fonts loaded by name*/
ssize_t ft_fonts_search_index_byname(ftFonts_s* fonts, const char* name);

/** search fonts loaded by path*/
ftFont_s* ft_fonts_search_path(ftFonts_s* fonts, const char* path);

/** get fonts by index*/
ftFont_s* ft_fonts_search_index(ftFonts_s* fonts, unsigned index);

/** return font file path, have name or path similar to path
 * @param path absolute path or font name
 * @return string with font file or null for error, remember to free this
 */
char* ft_file_search(char const* path);

/** loading fonts
 * @param fonts obj
 * @param path path or name of font, call ft_file_search insde
 * @param internalName set a custom name for this font
 * @return a font object
 */
ftFont_s* ft_fonts_load(ftFonts_s* fonts, const char* path, const char* internalName);

/** remove a font by index*/
void ft_fonts_remove_byindex(ftFonts_s* fonts, size_t index);

/** remove a font by name*/
void ft_fonts_remove_byname(ftFonts_s* fonts, const char* name);

/** free fonts obj*/
void ft_fonts_free(ftFonts_s* fonts);

/** set fonts size*/
err_t ft_font_size(ftFont_s* font, long w, long h);

/** set font size with dpi*/
err_t ft_font_size_dpi(ftFont_s* font, long w, long h, long dpiw, long dpih);

/** get a glyph or NULL if glyph is not loaded*/
ftRender_s* ft_glyph_get(ftFonts_s* fonts, utf_t utf);

/** load a glyph if not exists, return a glyph or null*/
ftRender_s* ft_fonts_glyph_load(ftFonts_s* fonts, utf_t utf, unsigned mode);

/** free glyph*/
void ft_glyph_free(ftRender_s* glyph);

/** get glyph min width*/
int ft_glyph_min_width(ftFont_s* font, utf_t utf);

/** set font render size*/
void ft_font_render_size(ftFont_s* font, unsigned w, unsigned h);

/** print font info*/
void ft_font_print_info(ftFont_s* font);

/** check if fonts is monospace*/
int ft_fonts_is_monospace(ftFonts_s* fonts);

/** return line height*/
unsigned ft_line_height(ftFonts_s* fonts);

/** return line lenght in pixels*/
unsigned ft_line_lenght(ftFonts_s* fonts, const utf8_t* str);

/** return multiline lenght in pixel */
unsigned ft_multiline_lenght(ftFonts_s* fonts, const utf8_t* str);

/** return multiline height in pixel */
unsigned ft_multiline_height(ftFonts_s* fonts, const utf8_t* str);

/** return height in autowrap text*/
unsigned ft_autowrap_height(ftFonts_s* fonts, const utf8_t* str, unsigned width);

/** draw utf in to dst on position pos with color fore and background back at origin X, cls clear background, pos is incremented*/
int g2d_putch(g2dImage_s* dst, g2dCoord_s* pos, ftFonts_s* fonts, utf_t ch, g2dColor_t fore, g2dColor_t back, unsigned originX, int cls, int indirect);

/** write a string, with putch, return NULL if end or pointer after new line*/
const utf8_t* g2d_string(g2dImage_s* dst, g2dCoord_s* pos, ftFonts_s* fonts, utf8_t const* str, g2dColor_t col, unsigned originX, int indirect);

/** write a string, with putch autwrap*/
void g2d_string_autowrap(g2dImage_s* dst, g2dCoord_s* pos, ftFonts_s* fonts, utf8_t const* str, g2dColor_t col, unsigned originX, int indirect);

/** write a string but not rewrite previus char*/
void g2d_string_replace(g2dImage_s* dst, g2dCoord_s* pos, ftFonts_s* fonts, utf8_t const* str, utf8_t const* old, g2dColor_t f, g2dColor_t b, unsigned originX);

/** create new custom utf */
utf_t ft_utf_custom(utf_t u, ftUtfCustom_f fn, void* userdata);

/** get custom utf*/
ftUtfCustom_s* ft_utf_custom_get(utf_t utf);

#endif
