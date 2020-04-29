#include <ef/ft.h>
#include <ef/memory.h>
#include <ef/str.h>
#include <ef/file.h>
#include <ef/utf8.h>
#include <ef/vector.h>
#include <ef/err.h>

//#include <freetype2/ft2build.h>
#include <fontconfig/fontconfig.h>

#define FONT_GLYPH_MIN 10
#define FONT_GLYPH_MAX 512
#define FONT_GLYPH_KEY 32

__private FT_Library ftlib;

#undef FTERRORS_H_
#define FT_ERRORDEF( e, v, s )  [e] = s,
#define FT_ERROR_START_LIST     {
#define FT_ERROR_END_LIST       };

__private const char* ftStrError[] = 
#include FT_ERRORS_H

__private ftUtfCustom_s* utfCustom;

__private unsigned nohash_utf(const char* name, __unused size_t len){
	return *(unsigned*)name;
}

err_t ft_begin(void){
	err_t err;
	if( (err=FT_Init_FreeType(&ftlib)) ){
		err_push("freetype2 error %d: %s", err, ftStrError[err]);
		return -1;
	}
	utfCustom = vector_new(ftUtfCustom_s, 24, 12);
	return 0;
}

void ft_end(void){
	vector_free(utfCustom);
	FT_Done_FreeType(ftlib);
}

__private void rbh_ft_glyph_free(__unused uint32_t hash, __unused const char* key, void* a){
	ft_glyph_free(a);
}

ftFonts_s* ft_fonts_new(void){
	ftFonts_s* fonts = mem_new(ftFonts_s);
	if( !fonts ){
		err_pushno("malloc");
		return NULL;
	}
	fonts->font = vector_new(ftFont_s, 3, 2);
	if( !fonts->font ){
		free(fonts);
		return NULL;
	}
	fonts->charmap = rbhash_new(FONT_GLYPH_MAX, FONT_GLYPH_MIN, FONT_GLYPH_KEY, nohash_utf, rbh_ft_glyph_free);
	if( !fonts->charmap ){
		vector_free(fonts->font);
		free(fonts);
		return NULL;
	}
	return fonts;
}

ftFont_s* ft_fonts_search_name(ftFonts_s* fonts, const char* name){
	const size_t len = strlen(name);
	vector_foreach( fonts->font, i){
		if( !str_equal(name, len, fonts->font[i].fname, strlen(fonts->font[i].name)) ){
			return &fonts->font[i];
		}
	}
	return NULL;
}

ssize_t ft_fonts_search_index_byname(ftFonts_s* fonts, const char* name){
	const size_t len = strlen(name);
	vector_foreach( fonts->font, i){
		if( !str_equal(name, len, fonts->font[i].fname, strlen(fonts->font[i].name)) ){
			return i;
		}
	}
	return -1;
}

ftFont_s* ft_fonts_search_path(ftFonts_s* fonts, const char* path){
	const size_t len = strlen(path);
	vector_foreach( fonts->font, i){
		if( !str_equal(path, len, fonts->font[i].fname, strlen(fonts->font[i].fname)) ){
			return &fonts->font[i];
		}
	}
	return NULL;
}

ftFont_s* ft_fonts_search_index(ftFonts_s* fonts, unsigned index){
	if( index >= vector_count(fonts->font) ) return NULL;
	return &fonts->font[index];
}

char* ft_file_search(char const* path){
	if( file_exists(path) ){
		dbg_info("%s => %s", path, path);
		return str_dup(path, 0);
	}
	FcConfig* config = FcInitLoadConfigAndFonts();
	FcPattern* pat = FcNameParse((const FcChar8*)path);
	FcConfigSubstitute(config, pat, FcMatchPattern);
	FcDefaultSubstitute(pat);
	FcResult result;
	FcPattern* font = FcFontMatch(config, pat, &result);
	if (font){
		FcChar8* file = NULL;
		if (FcPatternGetString(font, FC_FILE, 0, &file) == FcResultMatch){
			char* ret = str_dup((char*)file, 0);
			dbg_info("%s => %s", path, (char*)file);
			FcPatternDestroy(font);
			FcPatternDestroy(pat);
			return ret;
		}
		FcPatternDestroy(font);
	}
	FcPatternDestroy(pat);
	return NULL;
}

ftFont_s* ft_fonts_load(ftFonts_s* fonts, const char* path, const char* internalName){
	iassert(internalName);
	if( strlen(internalName) > FT_FONT_NAME_SIZE - 1) {
		err_push("internal name to long");
		return NULL;
	}

	ftFont_s* font = NULL;
	if( !(path = ft_file_search(path)) ){
		err_push("invalid font file %s", path);
		return NULL;
	}

	if( (font=ft_fonts_search_path(fonts, path)) ){
		dbg_info("font %s already loaded", path);
		free((char*)path);
		return font;
	}
	
	if( (font=ft_fonts_search_name(fonts, internalName)) ){
		dbg_info("font %s already loaded", internalName);
		free((char*)path);
		return font;
	}

	font = vector_get_push_back(fonts->font);
	memset(font, 0, sizeof(ftFont_s));

	err_t err;
	if( (err=FT_New_Face(ftlib, path, 0, &font->face)) ){
		err_push("freetype2 newface error %d: %s", err, ftStrError[err]);
		(void)vector_pull_back(fonts->font);
		free((char*)path);
		return NULL;
	}
	if( (err=FT_Select_Charmap(font->face,ft_encoding_unicode)) ){
		err_push("freetype2 load charmap %d: %s", err, ftStrError[err]);
		FT_Done_Face(font->face);
		(void)vector_pull_back(fonts->font);
		free((char*)path);
		return NULL;
	}

	font->fname = path;
	strcpy(font->name, internalName);

	return font;
}

__private void ft_font_delete(ftFont_s* font){
	FT_Done_Face(font->face);
	free((void*)font->fname);
}

void ft_fonts_remove_byindex(ftFonts_s* fonts, size_t index){
	if( index > vector_count(fonts->font) - 1 ) return;
	ft_font_delete(&fonts->font[index]);
	vector_remove(fonts->font, index);
}

void ft_fonts_remove_byname(ftFonts_s* fonts, const char* name){
	ssize_t index = ft_fonts_search_index_byname(fonts, name);
	if( index < 0 ) return;
	ft_fonts_remove_byindex(fonts, index);
}

void ft_fonts_free(ftFonts_s* fonts){
	vector_foreach(fonts->font, i){
		ft_font_delete(&fonts->font[i]);
	}
	vector_free(fonts->font);
	free(fonts);
}

__private void ft_font_metric_set(ftFont_s* font){
	font->width     = font->face->size->metrics.max_advance / 64;
	font->height    = font->face->size->metrics.height / 64;
	font->descender = font->face->size->metrics.descender / 64;
	font->ascender  = font->face->size->metrics.ascender / 64;
	font->advanceX  = font->face->max_advance_width / 64;
	font->advanceY  = font->face->max_advance_height /64;
}

err_t ft_font_size(ftFont_s* font, long w, long h){
	err_t err;	
	if( (err=FT_Set_Pixel_Sizes(font->face, w, h)) ){
		err_push("freetype2 set size error %d: %s", err, ftStrError[err]);
		return -1;
	}
	ft_font_metric_set(font);
	return 0;
}

err_t ft_font_size_dpi(ftFont_s* font, long w, long h, long dpiw, long dpih){
	err_t err;
	if( (err=FT_Set_Char_Size(font->face, w*64, h*64, dpiw, dpih)) ){
		err_push("freetype2 set size error %d: %s", err, ftStrError[err]);
		return -1;
	}
	ft_font_metric_set(font);
	return 0;
}

ftRender_s* ft_glyph_get(ftFonts_s* fonts, utf_t utf){
	char key[32];
	int len = sprintf(key,"%u",(uint32_t)utf);
	return rbhash_find_hash(fonts->charmap, utf, key, len);
}

__private void ft_glyph_render_hori_mono_byte(ftRender_s* glyph, const unsigned w, const unsigned h, unsigned char* buf, g2dMode_e mode){
	glyph->img = g2d_new(w, h, mode);

	g2dColor_t whyte = g2d_color_make(glyph->img, 255, 255, 255, 255);
	g2dColor_t black = g2d_color_make(glyph->img, 255, 0, 0, 0);

	g2dCoord_s pos = {.x = 0, .y = 0, .w = w, .h = h};
	g2d_clear(glyph->img, whyte, &pos);
	
	unsigned const gw = glyph->width;
	unsigned const gh = (glyph->height > h) ? h : glyph->height;
	unsigned const p = glyph->pitch;
	unsigned y = ((h - (-glyph->descender)) - glyph->horiBearingY) < 0 ? 0 : ((h - (-glyph->descender)) - glyph->horiBearingY);

	for( unsigned gy = 0; gy < gh && gy < h; ++y, ++gy ){
		unsigned const row = g2d_row(glyph->img, y);
		unsigned const gr = gy * p;
		for( 
				unsigned gx = 0, ix = glyph->horiBearingX; 
				gx < gw && ix < w;
			   	++gx, ++ix
		){
			if( (buf[gr+gx/8] << gx%8) & 0x80 ){
				g2dColor_t* pixel = g2d_color(glyph->img, row, ix);
				*pixel = black;
			}
		}
	}
}

__private void ft_glyph_render_graphics(ftRender_s* glyph, const unsigned w, const unsigned h, unsigned char* buf, g2dMode_e mode){
	glyph->img = g2d_new(w, h, mode);

	g2dColor_t whyte = g2d_color_make(glyph->img, 0, 255, 255, 255);

	g2dCoord_s pos = {.x = 0, .y = 0, .w = w, .h = h};
	g2d_clear(glyph->img, whyte, &pos);
	
	unsigned const gw = glyph->width;
	unsigned const gh = (glyph->height > h) ? h : glyph->height;
	unsigned y = ((h - (-glyph->descender)) - glyph->horiBearingY) < 0 ? 0 : ((h - (-glyph->descender)) - glyph->horiBearingY);
	unsigned const hBX = glyph->horiBearingX < 0 ? 0 : glyph->horiBearingX;
/*	
	dbg_error("glyph info:: y %u width %d height %d gw %d gh %d w %u h %u desc %ld hBY %d hBX %d", y, glyph->width, glyph->height, gw, gh, w, h, glyph->descender, glyph->horiBearingY, glyph->horiBearingX);
	
	

	for( unsigned dy = 0; dy < gh; ++dy){
		for( unsigned dx = 0; dx < gw; ++dx ){
			fputc(buf[dy*gw+dx] ? '#' : ' ' ,stderr);
		}
		fputc('\n',stderr);
	}

	dbg_error("----------------------------------------------------------------------------");
*/
	for( unsigned gy = 0; gy < gh && y < h; ++y, ++gy ){
		unsigned const row = g2d_row(glyph->img, y);
		unsigned const gr = gy * gw;
		for( 
				unsigned gx = 0, ix = hBX; 
				gx < gw && ix < w;
			   	++gx, ++ix
		){
			unsigned char gc = buf[ gr + gx ];
			unsigned char bw = gc ? 0 : 255;
			g2dColor_t* pixel = g2d_color(glyph->img, row, ix);
			*pixel = g2d_color_make(glyph->img, gc, bw, bw, bw);
//			fputc(gc?'#':' ',stderr);
		}
//		fputc('\n',stderr);
	}
}

/*
//bit position
//     0123456 
//8  0 0000000 1 0
//16 1 000#00  2 00
//24 2 00000   3 000
//32 3 0000    4 0000
//40 4 000     5 00000
//48 5 00      6 000000
//56 6 0       7 00#0000
//64 7 0000000 8
//72 8 000000  9 0

// y * w + x
//y7 w7 x3
//7*7+3=49+3=52
//y1 w7 x4
//1*7+4=7+4=11
//byte = (y*w+x)/8
//bits = (y*w+x)%8
__private void ft_glyph_render_hori_mono(ftRender_s* glyph, unsigned char* buf){
	if( glyph->pixel ) return;
	
	size_t nbyte = ((glyph->pixelWidth*glyph->pixelHeight)/8)+1;
	glyph->pixel  = mem_zero_many(unsigned char, nbyte);
	iassert(glyph->pixel);

	unsigned w = glyph->pixelWidth;
	unsigned h = glyph->pixelHeight;
	unsigned gw = glyph->width;
	unsigned gh = (glyph->height > h) ? h : glyph->height;
	unsigned p = glyph->pitch;
	unsigned y = (h + glyph->descender) - glyph->horiBearingY < 0 ? 0 : (h + glyph->descender) - glyph->horiBearingY;
	unsigned gy;

	for(gy = 0; gy < gh && y < h; ++y, ++gy){
		unsigned x = glyph->horiBearingX;
		unsigned r = y * w;
		unsigned gr = gy * p;
		for( unsigned gx = 0; (gx < gw) && (x < w) ; ++gx, ++x){
			if( (buf[gr+gx/8] << gx%8) & 0x80 ){
				unsigned byte = (r+x)/8;
				unsigned bit  = (r+x)%8; 
				//dbg_info("render r(%u) gx(%u/%u) gy(%u/%u) x(%u/%u) y(%u/%u) B(%u/%lu) b(%u)", r, gx, gw, gy, gh, x, w, y, h, byte, nbyte, bit);
				iassert( byte < nbyte );
				glyph->pixel[byte] |= 1 << (7-bit);
			}
		}
	}
}
*/

__private ftRender_s* ft_font_glyph_load(ftFonts_s* fonts, ftFont_s* font, utf_t utf, unsigned mode){
	iassert(font);
	ftRender_s* glyph = NULL;
	
	unsigned index = FT_Get_Char_Index(font->face, utf);
	if( (mode & FT_RENDER_VALID) ){
		if (index == 0) {
			dbg_warning("no utf 0x%X in font %s", utf, font->name);
			return NULL;
		}
		mode &=~FT_RENDER_VALID;
	}

	err_t err;
	if( (err=FT_Load_Glyph(font->face, index, 0)) ){
		dbg_error("freetype2 load glyph error %d: %s", err, ftStrError[err]);
		return NULL;
	}

	if( font->face->glyph->format != FT_GLYPH_FORMAT_BITMAP ){
		if( (err=FT_Render_Glyph( font->face->glyph, mode & FT_RENDER_ANTIALIASED ? FT_RENDER_MODE_NORMAL : FT_RENDER_MODE_MONO )) ){
			dbg_error("freetype2 render glyph error %d: %s", err, ftStrError[err]);
			return NULL;
		}
	}

	glyph = mem_new(ftRender_s);
	if( !glyph ){
		err_pushno("malloc");
		return NULL;
	}
	glyph->pitch = font->face->glyph->bitmap.pitch;
	glyph->width = font->face->glyph->bitmap.width;
	glyph->height = font->face->glyph->bitmap.rows;
	glyph->horiBearingX = font->face->glyph->metrics.horiBearingX/64;
	glyph->horiBearingY = font->face->glyph->metrics.horiBearingY/64;
	glyph->horiAdvance =  font->face->glyph->metrics.horiAdvance/64;
	glyph->penAdvance = font->face->glyph->bitmap_left;
	glyph->vertBearingX = font->face->glyph->metrics.vertBearingX/64;
	glyph->vertBearingY = font->face->glyph->metrics.vertBearingY/64;
	glyph->vertAdvance = font->face->glyph->metrics.vertAdvance/64;
	glyph->linearHoriAdvance = font->face->glyph->linearHoriAdvance/65536;
	glyph->linearVertAdvance = font->face->glyph->linearVertAdvance/65536;
	glyph->img = NULL;
	//glyph->img.h = font->height;
	//glyph->img.w  = font->width;
	glyph->descender = font->descender;
	glyph->utf = utf;

	char key[32];
	int len = sprintf(key,"%u",(uint32_t)utf);
	if( rbhash_add_hash(fonts->charmap, utf, key, len, glyph) ){
		err_push("fail to add new glyph");
		free(glyph);
		return NULL;
	}

	if( mode & FT_RENDER_BYTE ){
		dbg_info("mono");
		ft_glyph_render_hori_mono_byte(glyph, font->width, font->height, font->face->glyph->bitmap.buffer, G2D_MODE_BGRA);
	}
	else if( mode & FT_RENDER_ANTIALIASED ){
		dbg_info("antialaised");
		ft_glyph_render_graphics(glyph, font->width, font->height, font->face->glyph->bitmap.buffer, G2D_MODE_ARGB);
	}
	else{
		dbg_fail("not implemented");
	}
	return glyph;
}

ftRender_s* ft_fonts_glyph_load(ftFonts_s* fonts, utf_t utf, unsigned mode){
	ftRender_s* glyph = ft_glyph_get(fonts, utf);
	if( glyph ) return glyph;

	vector_foreach(fonts->font, i){
			if( (glyph=ft_font_glyph_load(fonts, &fonts->font[i], utf, mode)) ){
			return glyph;
		}
	}
	return NULL;
}

void ft_glyph_free(ftRender_s* glyph){
	g2d_free(glyph->img);
	free(glyph);
}

int ft_glyph_min_width(ftFont_s* font, utf_t utf){
	unsigned index = FT_Get_Char_Index(font->face, utf);

	err_t err;
	if( (err=FT_Load_Glyph(font->face, index, 0)) ){
		dbg_error("freetype2 load glyph error %d: %s", err, ftStrError[err]);
		return -1;
	}

	if( font->face->glyph->format != FT_GLYPH_FORMAT_BITMAP ){
		if( (err=FT_Render_Glyph( font->face->glyph, FT_RENDER_MODE_MONO )) ){
			dbg_error("freetype2 render glyph error %d: %s", err, ftStrError[err]);
			return -1;
		}
	}

	return font->face->glyph->metrics.horiAdvance/64;
}

void ft_font_render_size(ftFont_s* font, unsigned w, unsigned h){
	font->width = w;
	font->height = h;
}

void ft_font_print_info(ftFont_s* font){
	printf("[font]\n");
	printf("path: %s ", font->fname);
	printf("family: %s ", font->face->family_name);
	printf("style: %s\n", font->face->style_name);
	printf("nglyphs: %ld ", font->face->num_glyphs);
	printf("EM: %d ", font->face->units_per_EM);
	printf("max_advance: %ld ", font->face->size->metrics.max_advance/64);
	printf("height: %ld ", font->height);
	printf("descending: %ld ", font->descender);
	printf("count size: %d\n", font->face->num_fixed_sizes);
	printf("size: ");
	for(int i = 0; i < font->face->num_fixed_sizes; ++i){
		printf("%ld|%d*%d ", font->face->available_sizes[i].size, font->face->available_sizes[i].width, font->face->available_sizes[i].height);
	}
	printf("\n");
}

#ifdef DEBUG_ENABLE
/*
__private void draw_hmB(ftRender_s* render){
	for( unsigned i = 0; i < render->pixelWidth + 2; ++i)
		putchar('O');
	putchar('\n');
	for( unsigned y = 0; y < render->pixelHeight; ++y){
		putchar('O');
		unsigned p = y * render->pixelWidth;
		for( unsigned x = 0; x < render->pixelWidth; ++x){
			putchar( render->pixel[p+x] ? '#' : ' ' );
		}
		puts("O");
	}
	for( unsigned i = 0; i < render->pixelWidth + 2; ++i)
		putchar('O');
	putchar('\n');
}

__private void draw_hmb(ftRender_s* render){
	for( unsigned i = 0; i < render->pixelWidth + 2; ++i)
		putchar('O');
	putchar('\n');
	for( unsigned y = 0; y < render->pixelHeight; ++y){
		putchar('O');
		unsigned r = y * render->pixelWidth;
		for( unsigned x = 0; x < render->pixelWidth; ++x){
			unsigned byte = (r+x)/8;
			unsigned bit  = (r+x)%8;
			//dbg_info("render x(%u/%u) y(%u/%u) B(%u) b(%u)", x, render->pixelWidth, y, render->pixelHeight, byte, bit);
			putchar( render->pixel[byte] & (1<<(7-bit)) ? '#' : ' ' );
		}
		puts("O");
	}
	for( unsigned i = 0; i < render->pixelWidth + 2; ++i)
		putchar('O');
	putchar('\n');
}

void ft_font_test(int mode){
	ftlib_h lib;
	ftFonts_s fonts;
	ftFont_s* font;
	const char* fontName = "/usr/share/fonts/TTF/Unifont.ttf";
	const size_t size = 16;
	utf8_t* fontPatterns = (utf8_t*)"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789▀▁▂▃▄▅▆▇█▉▊▋▌▍▎▏▐░▒▓▔▕▖▗▘▙▚▛▜▝▞▟";

	if( ft_init(&lib) ) return;
	ft_fonts_init(lib, &fonts);
	if( !(font=ft_fonts_load(&fonts, fontName)) ){	
		ft_terminate(lib);
		return;
	}
	if( ft_font_size(font, 0, size) ){
		ft_fonts_free(&fonts);
		ft_terminate(lib);
		return;
	}
	
	//ft_font_render_size(font, 8, size);

	if( mode & FT_RENDER_BYTE ){
		puts("Render Byte");
	}
	else{
		puts("Render Bits");
	}

	utf8Iterator_s it = utf8_iterator(fontPatterns, 0);
	utf_t utf;
	while( (utf=utf8_iterator_next(&it)) ){
		ftRender_s* glyph;
		if( !(glyph=ft_glyph_load(font, utf, mode)) ){
			dbg_error("fail load glyph %u", utf);
			continue;
		}
		if( mode & FT_RENDER_BYTE ){
			draw_hmB(glyph);
		}
		else{
			draw_hmb(glyph);
		}
	}
	
	ft_font_print_info(font);
	ft_fonts_free(&fonts);
	ft_terminate(lib);
}
*/
#endif

int ft_fonts_is_monospace(ftFonts_s* fonts){
	ftRender_s* t1 = ft_fonts_glyph_load(fonts, ' ', FT_RENDER_ANTIALIASED | FT_RENDER_VALID);
	if( !t1 ){
		err_push("invalid space character");
		return 0;
	}
	ftRender_s* t2 = ft_fonts_glyph_load(fonts, '9', FT_RENDER_ANTIALIASED | FT_RENDER_VALID);
	if( !t2 ){
		err_push("invalid 9 character");
		return 0;
	}
	return t1->horiAdvance == t2->horiAdvance ? 1 : 0;
}

unsigned ft_line_height(ftFonts_s* fonts){
	ftRender_s* rch = ft_fonts_glyph_load(fonts, ' ', FT_RENDER_ANTIALIASED | FT_RENDER_VALID);
	if( !rch ){
		err_push("char not find");
		return 0;
	}
	return rch->img->h;
}

unsigned ft_line_lenght(ftFonts_s* fonts, const utf8_t* str){
	if( !str || !*str ){
		dbg_warning("no str");
		return 0;
	}
	unsigned lenght = 0;
	utf8Iterator_s it = utf8_iterator((utf8_t*)str, 0);
	utf_t u;	
	while( (u = utf8_iterator_next(&it)) ){
		if( u >= UTF_PRIVATE0_START ){
			continue;
		}
		if( u == '\n' ) break;
		ftRender_s* rch = ft_fonts_glyph_load(fonts, u, FT_RENDER_ANTIALIASED | FT_RENDER_VALID);
		if( rch ) lenght += rch->horiAdvance;
	}
	dbg_info("line lenght:%u", lenght);
	return lenght;
}	

unsigned ft_line_lenght_rev(ftFonts_s* fonts, const utf8_t* begin, const utf8_t* str){
	if( !str || !*str || !begin || !*begin){
		dbg_warning("no str");
		return 0;
	}
	unsigned lenght = 0;
	utf8Iterator_s it = utf8_iterator((utf8_t*)str, 0);
	utf_t u;
	while( (u = utf8_iterator_prev(&it)) ){
		if( u >= UTF_PRIVATE0_START ){
			continue;
		}
		if( u == '\n' ) break;
		ftRender_s* rch = ft_fonts_glyph_load(fonts, u, FT_RENDER_ANTIALIASED | FT_RENDER_VALID);
		if( rch ) lenght += rch->horiAdvance;
	}
	dbg_info("line lenght:%u", lenght);
	return lenght;
}

unsigned ft_multiline_lenght(ftFonts_s* fonts, const utf8_t* str){
	if( !str || !*str ){
		dbg_warning("no str");
		return 0;
	}
	unsigned max = 0;
	unsigned lenght = 0;
	utf8Iterator_s it = utf8_iterator((utf8_t*)str, 0);
	utf_t u;	
	while( (u = utf8_iterator_next(&it)) ){
		if( u >= UTF_PRIVATE0_START ){
			continue;
		}
		if( u == '\n' ){
			if( lenght > max ) max = lenght;
			continue;
		}
		ftRender_s* rch = ft_fonts_glyph_load(fonts, u, FT_RENDER_ANTIALIASED | FT_RENDER_VALID);
		if( rch ) lenght += rch->horiAdvance;
	}
	if( lenght > max ) max = lenght;
	dbg_info("multiline lenght:%u", max);
	return max;
}

unsigned ft_multiline_height(ftFonts_s* fonts, const utf8_t* str){
	if( !str ){
		dbg_warning("no str");
		return 0;
	}

	unsigned monoh = ft_line_height(fonts);
	unsigned height = monoh;
	unsigned lenght = 0;

	utf8Iterator_s it = utf8_iterator((utf8_t*)str, 0);
	utf_t utf;
	while( (utf = utf8_iterator_next(&it)) ){
		if( utf >= UTF_PRIVATE0_START ){
			continue;
		}
		if( utf == '\n' ){
			height += monoh;
			lenght = 0;
			continue;
		}
		ftRender_s* rch = ft_fonts_glyph_load(fonts, *str, FT_RENDER_ANTIALIASED | FT_RENDER_VALID);
		if( rch ) lenght += rch->horiAdvance;
	}
	return height;
}

unsigned ft_multiline_height_tostr(ftFonts_s* fonts, const utf8_t* str, const utf8_t* end){
	if( !str ){
		dbg_warning("no str");
		return 0;
	}

	unsigned monoh = ft_line_height(fonts);
	unsigned height = monoh;
	unsigned lenght = 0;

	utf8Iterator_s it = utf8_iterator((utf8_t*)str, 0);
	utf_t utf;
	while( (utf = utf8_iterator_next(&it)) && it.str < end){
		if( utf >= UTF_PRIVATE0_START ){
			continue;
		}
		if( utf == '\n' ){
			height += monoh;
			lenght = 0;
			continue;
		}
		ftRender_s* rch = ft_fonts_glyph_load(fonts, *str, FT_RENDER_ANTIALIASED | FT_RENDER_VALID);
		if( rch ) lenght += rch->horiAdvance;
	}
	return height;
}

unsigned ft_autowrap_height(ftFonts_s* fonts, const utf8_t* str, unsigned width){
	if( !str ){
		dbg_warning("no str");
		return 0;
	}

	unsigned monoh = ft_line_height(fonts);
	unsigned height = monoh;
	unsigned lenght = 0;

	utf8Iterator_s it = utf8_iterator((utf8_t*)str, 0);
	utf_t utf;
	while( (utf = utf8_iterator_next(&it)) ){
		if( utf >= UTF_PRIVATE0_START ){
			continue;
		}
		if( utf == '\n' ){
			height += monoh;
			lenght = 0;
			continue;
		}
		if( lenght >= width ){
			height += monoh;
			lenght = 0;
		}	
		ftRender_s* rch = ft_fonts_glyph_load(fonts, *str, FT_RENDER_ANTIALIASED | FT_RENDER_VALID);
		if( rch ) lenght += rch->horiAdvance;
	}

	return height;
}	

unsigned ft_autowrap_height_to(ftFonts_s* fonts, const utf8_t* str, const utf8_t* end, unsigned width){
	if( !str ){
		dbg_warning("no str");
		return 0;
	}

	unsigned monoh = ft_line_height(fonts);
	unsigned height = monoh;
	unsigned lenght = 0;

	utf8Iterator_s it = utf8_iterator((utf8_t*)str, 0);
	utf_t utf;
	while( (utf = utf8_iterator_next(&it)) && it.str < end ){
		if( utf >= UTF_PRIVATE0_START ){
			continue;
		}
		if( utf == '\n' ){
			height += monoh;
			lenght = 0;
			continue;
		}
		if( lenght >= width ){
			height += monoh;
			lenght = 0;
		}	
		ftRender_s* rch = ft_fonts_glyph_load(fonts, *str, FT_RENDER_ANTIALIASED | FT_RENDER_VALID);
		if( rch ) lenght += rch->horiAdvance;
	}

	return height;
}

__private void g2d_ch_clear(g2dImage_s* dst, g2dCoord_s* pos, ftRender_s* glyph, g2dColor_t color){
	g2dCoord_s co = {.x = pos->x, .y = pos->y, .w = glyph->img->w, .h = glyph->img->h};
	g2d_clear(dst, color, &co);
}

int g2d_putch(g2dImage_s* dst, g2dCoord_s* pos, ftFonts_s* fonts, utf_t ch, g2dColor_t fore, g2dColor_t back, unsigned originX, int cls, int indirect){
	if( ch == '\n' ){
		pos->x = originX;
		pos->y += ft_line_height(fonts);
		return 1;
	}
	
	ftRender_s* rch = ft_fonts_glyph_load(fonts, ch, FT_RENDER_ANTIALIASED | FT_RENDER_VALID);
	if( !rch ){
		dbg_warning("invalid render glyph");
		return -1;
	}
	if( rch->horiBearingX < 0 ){
		unsigned const phBX = -rch->horiBearingX;
		if( pos->x < phBX ) 
			pos->x = 0;
		else
			pos->x -= phBX;
	}
	if( pos->x + rch->horiAdvance > pos->w ){
		dbg_error("END OF LINE");
		pos->x = originX;
		pos->y += ft_line_height(fonts);
		return 2;
	}

	if( pos->y + rch->img->h > pos->h ){
		dbg_error("END OF HEIGHT");
		return -1;
	}

	dbg_info("putch %u %u wh:%u*%u ch:%u*%u hory:%u", pos->x, pos->y, pos->w, pos->h, rch->img->w, rch->img->h, rch->horiAdvance);

	if( cls ) g2d_ch_clear(dst, pos, rch, back);
	if( indirect ){
		g2d_char_indirect(dst, pos, rch->img, fore);
	}
	else{
		g2d_char(dst, pos, rch->img, fore);
	}
	pos->x += rch->horiAdvance;
	return 0;
}

const utf8_t* g2d_string(g2dImage_s* dst, g2dCoord_s* pos, ftFonts_s* fonts, utf8_t const* str, g2dColor_t col, unsigned originX, int indirect){
	utf8Iterator_s it = utf8_iterator((utf8_t*)str, 0);
	utf_t utf;
	while( (utf = utf8_iterator_next(&it)) ){
		if( utf >= UTF_PRIVATE0_START ){
			ftUtfCustom_s* fuc = ft_utf_custom_get(utf);
			fuc->fn(utf, &fonts, &pos->x, &pos->y, &col, fuc->userdata);
			continue;
		}
		int stret = g2d_putch(dst, pos, fonts, utf, col, 0,  originX, 0, indirect);
		if( stret ) return stret == 1 ? it.str+1 : NULL;
	}
	return NULL;
}

void g2d_string_autowrap(g2dImage_s* dst, g2dCoord_s* pos, ftFonts_s* fonts, utf8_t const* str, g2dColor_t col, unsigned originX, int indirect){
	utf8Iterator_s it = utf8_iterator((utf8_t*)str, 0);
	utf_t utf;
	while( (utf = utf8_iterator_next(&it)) ){
		if( utf >= UTF_PRIVATE0_START ){
			ftUtfCustom_s* fuc = ft_utf_custom_get(utf);
			fuc->fn(utf, &fonts, &pos->x, &pos->y, &col, fuc->userdata);
		//g2d_putch_autowrap(dst, pos, fonts, utf, col, 0,  originX, 0);
			continue;
		}
		int ret = g2d_putch(dst, pos, fonts, utf, col, 0,  originX, 0, indirect);
		if( ret < 0 ){
			dbg_info("exit");	
			break;
		}
		if( ret > 1 ){
			if( g2d_putch(dst, pos, fonts, utf, col, 0,  originX, 0, indirect) ){
				dbg_info("exit 2");
			}
		}
	}
}
//TODO
void g2d_string_replace(g2dImage_s* dst, g2dCoord_s* pos, ftFonts_s* fonts, utf8_t const* str, utf8_t const* old, g2dColor_t f, g2dColor_t b, unsigned originX){
	if( !str || !old ) return;
	utf8Iterator_s it = utf8_iterator((utf8_t*)str, 0);
	utf8Iterator_s ot = utf8_iterator((utf8_t*)old, 0);
	utf_t utf, of = 0;
	while( (utf = utf8_iterator_next(&it)) ){
		if( utf >= UTF_PRIVATE0_START ){
			ftUtfCustom_s* fuc = ft_utf_custom_get(utf);
			fuc->fn(utf, &fonts, &pos->x, &pos->y, &f, fuc->userdata);
			continue;
		}
		of = utf8_iterator_next(&ot);
		if( of == utf ){
			ftRender_s* rch = ft_fonts_glyph_load(fonts, utf, FT_RENDER_ANTIALIASED);
			pos->x += rch->horiAdvance;
		}
		else{
			if( g2d_putch(dst, pos, fonts, utf, f, b, originX, 1,1) ){
				dbg_info("exit");
				return;
			}
		}
		if( !of ) break;
	}

	if( utf ){
		dbg_info("write unknow old chars");
		while( (utf = utf8_iterator_next(&it))  ){
			if( utf >= UTF_PRIVATE0_START ){
				ftUtfCustom_s* fuc = ft_utf_custom_get(utf);
				fuc->fn(utf, &fonts, &pos->x, &pos->y, &f, fuc->userdata);
				continue;
			}
			if( g2d_putch(dst, pos, fonts, utf, f, b, originX, 1,1) ){
				dbg_info("exit l");
				return;
			}
		}
	}
	else if( of ){
		dbg_info("clear to end");
		size_t lenght = ft_line_lenght(fonts, ot.str);
		if( lenght + pos->x > pos->w ) lenght = pos->w - pos->x;
		g2dCoord_s co = { .x = pos->x, .y = pos->y, .w = lenght, .h = ft_line_height(fonts) };
		g2d_clear(dst, b, &co);
		pos->x += lenght;
	}
}

utf_t ft_utf_custom(utf_t u, ftUtfCustom_f fn, void* userdata){
	if( u == 0 ){
		ftUtfCustom_s* fuc = vector_get_push_back(utfCustom);
		u = fuc->utf = UTF_PRIVATE0_START + vector_count(utfCustom);
		fuc->fn = fn;
		fuc->userdata = userdata;
	}
	else{
		unsigned index = (u - UTF_PRIVATE0_START)-1;
		if( index >= vector_count(utfCustom) ) return 0;
		utfCustom[index].fn = fn;
		utfCustom[index].userdata = userdata;
	}	
	return u;
}

ftUtfCustom_s* ft_utf_custom_get(utf_t utf){
	unsigned index = (utf - UTF_PRIVATE0_START)-1;
	if( index >= vector_count(utfCustom) ) return 0;
	return &utfCustom[index];
}

