#include <ef/guiText.h>
#include <ef/memory.h>
#include <ef/str.h>
#include <ef/err.h>

guiText_s* gui_text_new(ftFonts_s* font, g2dColor_t foreground, g2dColor_t select, g2dColor_t colCursor, unsigned tabspace, unsigned blinktime, unsigned flags){
	guiText_s* txt = mem_new(guiText_s);
	if( !txt ) return NULL;
	txt->render = NULL;
	txt->scroll.x = 0;
	txt->scroll.y = 0;
	txt->cursor.x = 0;
	txt->cursor.y = 0;
	txt->cursor.h = ft_line_height(font);
	txt->cursor.w = 1 + (flags >> GUI_TEXT_FLAGS_END);
	txt->blink = NULL;
	txt->blinktime = blinktime;
	txt->selStart = txt->selEnd = NULL;
	txt->clipmem = NULL;

	dbg_info("genric glyph size: %u*%u", txt->cursor.w, txt->cursor.h);

	txt->len = 0;
	txt->size = 0;
	txt->resize = 0;
	txt->text = NULL;

	txt->fonts = font;
	txt->foreground = foreground;
	txt->select = select;
	txt->colCursor = colCursor;
	txt->flags = flags;
	txt->tabspace = tabspace;

	ftRender_s* glyph = ft_fonts_glyph_load(font, ' ', FT_RENDER_VALID | FT_RENDER_ANTIALIASED);
	iassert(glyph);
	txt->spacesize = glyph->horiAdvance;

	return txt;
}

gui_s* gui_text_attach(gui_s* gui, guiText_s* txt){
	if( !gui ) goto ERR;
	if( !txt ) goto ERR;
	gui->control = txt;
	gui->type = GUI_TYPE_TEXT;
	gui->redraw = gui_text_event_redraw;
	gui->key = gui_text_event_key;
	gui->mouse = gui_text_event_mouse;
	gui->free = gui_text_event_free;
	gui->focus = gui_text_event_focus;
	gui->clipboard = gui_text_event_clipboard;
	gui->move = gui_text_event_move;
	gui->themes = gui_text_event_themes;
	gui->focusable = 1;

	txt->resize = txt->size = (gui->surface->img->w / ft_line_lenght(txt->fonts, U8(" "))) * (gui->surface->img->h / txt->cursor.h);
	txt->text = mem_many(utf8_t, txt->resize);
	if( !txt->text ) err_fail("eom");
	*txt->text = 0;
	txt->it = utf8_iterator(txt->text, 0);
	txt->render = gui_image_custom_new(
			g2d_new(gui->surface->img->w, gui->surface->img->h, -1), 
			GUI_IMAGE_FLAGS_ALPHA
	);
	gui_composite_add(gui->img, txt->render);

	guiImage_s* cursor = gui_image_fn_new(gui_text_render_cursor, txt, txt->cursor.w, txt->cursor.h, 0);
	gui_composite_add(gui->img, cursor);

	return gui;
ERR:
	if( txt ) gui_text_free(txt);
	if( gui ) gui_free(gui);
	return NULL;
}

void gui_text_free(guiText_s* txt){
	if( txt->text ) free(txt->text);
	if( txt->clipmem ) free(txt->clipmem);
	free(txt);
}

void gui_text_flags_set(gui_s* gui, unsigned flags){
	iassert(gui->type == GUI_TYPE_TEXT);
	guiText_s* txt = gui->control;
	txt->flags = flags;
}

void gui_text_ir_toggle(gui_s* gui){
	iassert(gui->type == GUI_TYPE_TEXT);
	guiText_s* txt = gui->control;
	if( txt->flags & GUI_TEXT_INSERT ){
		txt->flags &= ~GUI_TEXT_INSERT;
	}
	else{
		txt->flags |= GUI_TEXT_INSERT;
	}	
}

const utf8_t* gui_text_str_raw(gui_s* gui){
	iassert(gui->type == GUI_TYPE_TEXT);
	guiText_s* txt = gui->control;
	return txt->text;
}

utf8_t* gui_text_str(gui_s* gui){
	iassert(gui->type == GUI_TYPE_TEXT);
	guiText_s* txt = gui->control;
	utf8_t* str = mem_many(utf8_t, txt->len + 1);
	utf8Iterator_s itsrc = utf8_iterator(txt->text, 0);
	utf8Iterator_s itdst = utf8_iterator(str, 0);
	utf_t utf;
	while( (utf=utf8_iterator_next(&itsrc)) ){
		if( utf >= UTF_PRIVATE0_START ){
			continue;
		}
		utf8_iterator_replace(&itdst, utf);
	}
	utf8_iterator_replace(&itdst, 0);
	return str;
}

void gui_text_sel(gui_s* gui){
	iassert(gui->type == GUI_TYPE_TEXT);
	guiText_s* txt = gui->control;
	if( txt->selStart ){
		txt->selEnd = txt->it.str+1;
	}
	else{
		txt->selStart = txt->it.str + 1;
		txt->selEnd = txt->selStart;
		gui_clipboard_copy(gui, 1);
	}
	txt->flags |= GUI_TEXT_SEL | GUI_TEXT_REND_TEXT;
}

void gui_text_unsel(gui_s* gui){
	iassert(gui->type == GUI_TYPE_TEXT);
	guiText_s* txt = gui->control;
	txt->flags &= ~GUI_TEXT_SEL;
	txt->selStart = NULL;
	txt->selEnd = NULL;
	txt->flags |= GUI_TEXT_REND_TEXT;
}

void gui_text_sel_del(gui_s* gui){
	iassert(gui->type == GUI_TYPE_TEXT);
	guiText_s* txt = gui->control;
	utf8Iterator_s it;
	utf8_t* end;
	size_t n;

	if( txt->selStart < txt->selEnd ){
		it = utf8_iterator(txt->selStart > txt->text ? txt->selStart - 1 : txt->text, 0);
		end = txt->selEnd;
	}
	else if( txt->selStart > txt->selEnd ){
		it = utf8_iterator(txt->selEnd > txt->text ? txt->selEnd - 1 : txt->text, 0);
		end = txt->selStart;
	}
	else{
		return;
	}
	n = end-it.str;
	if( n ) --n;
	txt->it.str = it.str;
	utf8_iterator_delete_to(&it, n);

	gui_text_unsel(gui);
	txt->flags |= GUI_TEXT_REND_TEXT | GUI_TEXT_REND_CURSOR | GUI_TEXT_REND_SCROLL;
}

utf8_t* gui_text_sel_get(gui_s* gui){
	iassert(gui->type == GUI_TYPE_TEXT);
	guiText_s* txt = gui->control;
	utf8Iterator_s it;
	utf8_t* end;

	dbg_info("its %p ite %p", txt->selStart, txt->selEnd);
	if( txt->selStart < txt->selEnd ){
		it = utf8_iterator(txt->selStart > txt->text ? txt->selStart - 1 : txt->text, 0);
		end = txt->selEnd;
	}
	else if( txt->selStart > txt->selEnd ){
		it = utf8_iterator(txt->selEnd > txt->text ? txt->selEnd - 1 : txt->text, 0);
		end = txt->selStart;
	}
	else{
		return NULL;
	}

	size_t size = end - it.str + 1;
	utf8_t* get = mem_many(utf8_t, size+1);
	*get = 0;
	utf8Iterator_s ds = utf8_iterator(get,0);
	utf_t u;
	while( (u=utf8_iterator_next(&it)) && it.str < end ) utf8_iterator_replace(&ds, u);
	return get;
}

size_t gui_text_line_right_len(gui_s* gui){
	iassert(gui->type == GUI_TYPE_TEXT);
	guiText_s* txt = gui->control;
	utf8Iterator_s it = txt->it;
	size_t width = 0;
	utf_t utf;
	while( (utf=utf8_iterator_next(&it)) && utf != '\n' ) ++width;
	return width;
}

size_t gui_text_line_left_len(gui_s* gui){
	iassert(gui->type == GUI_TYPE_TEXT);
	guiText_s* txt = gui->control;
	utf8Iterator_s it = txt->it;
	size_t width = 0;
	utf_t utf;
	while( (utf=utf8_iterator_prev(&it)) && utf != '\n' ) ++width;
	return width;
}

utf8_t* gui_text_back_line_ptr(gui_s* gui){
	iassert(gui->type == GUI_TYPE_TEXT);
	guiText_s* txt = gui->control;
	utf8Iterator_s it = txt->it;
	utf_t u;
	while( (u=utf8_iterator_prev(&it)) ){
		if( u == '\n' ){
			while( (u=utf8_iterator_prev(&it)) ){
				if( u == '\n' ){
					utf8_iterator_next(&it);	
					break;
				}
			}
			return it.str;
		}
	}
	return NULL;
}

utf8_t* gui_text_next_line_ptr(gui_s* gui){
	iassert(gui->type == GUI_TYPE_TEXT);
	guiText_s* txt = gui->control;
	utf8Iterator_s it = txt->it;
	utf_t u;
	while( (u=utf8_iterator_next(&it)) ){
		if( u == '\n' ){
			return it.str;
		}
	}
	return NULL;
}

void gui_text_put(gui_s* gui, utf_t utf){
	iassert(gui->type == GUI_TYPE_TEXT);
	guiText_s* txt = gui->control;
	if( !(txt->flags & GUI_TEXT_SCROLL_X) && !(txt->flags & GUI_TEXT_SCROLL_Y) ){
		if( utf == '\n' ) return;
		if( txt->cursor.x +  txt->cursor.w + txt->spacesize>= gui->surface->img->w ) return;
	}

	if( txt->it.str - txt->it.begin + 4 >= (long)txt->size - 1 ){
		txt->size += txt->resize;
		if( mem_resize(txt->text, utf8_t, txt->size) ){
			err_fail("eom");
		}
		size_t offset = txt->it.str - txt->it.begin;
		txt->it.begin = txt->text;
		txt->it.str =  txt->it.begin + offset;
	}
	if( txt->flags & GUI_TEXT_INSERT ){
		dbg_info("insert 0x%X", utf);
		utf8_iterator_insert(&txt->it, utf);
	}
	else{
		dbg_info("replace 0x%X", utf);
		utf8_iterator_replace(&txt->it, utf);
	}

	txt->flags |= GUI_TEXT_REND_TEXT | GUI_TEXT_REND_CURSOR | GUI_TEXT_REND_SCROLL;
}

void gui_text_puts(gui_s* gui,  utf8_t* str){
	iassert(gui->type == GUI_TYPE_TEXT);
	guiText_s* txt = gui->control;
	if( txt->selStart ) gui_text_sel_del(gui);
	utf8Iterator_s it = utf8_iterator(str, 0);
	utf_t utf;
	while( (utf=utf8_iterator_next(&it)) ){
		gui_text_put(gui, utf);
	}
}

void gui_text_del(gui_s* gui){
	iassert(gui->type == GUI_TYPE_TEXT);
	guiText_s* txt = gui->control;
	utf8_iterator_delete(&txt->it);
	txt->flags |= GUI_TEXT_REND_TEXT | GUI_TEXT_REND_CURSOR | GUI_TEXT_REND_SCROLL;
}

void gui_text_backspace(gui_s* gui){
	iassert(gui->type == GUI_TYPE_TEXT);
	guiText_s* txt = gui->control;
	if( utf8_iterator_prev(&txt->it) ){
		utf8_iterator_delete(&txt->it);
		txt->flags |= GUI_TEXT_REND_TEXT | GUI_TEXT_REND_CURSOR | GUI_TEXT_REND_SCROLL;
	}
}

void gui_text_cursor_change(gui_s* gui, unsigned modeflags){
	iassert(gui->type == GUI_TYPE_TEXT);
	guiText_s* txt = gui->control;
	txt->flags = (txt->flags & GUI_TEXT_FLAGS_MASK) | modeflags;
}

void gui_text_cursor_next(gui_s* gui){
	iassert(gui->type == GUI_TYPE_TEXT);
	guiText_s* txt = gui->control;
	while( utf8_iterator_next(&txt->it) >= UTF_PRIVATE0_START );
	txt->flags |= GUI_TEXT_REND_CURSOR | GUI_TEXT_REND_SCROLL;
}

void gui_text_cursor_prev(gui_s* gui){
	iassert(gui->type == GUI_TYPE_TEXT);
	guiText_s* txt = gui->control;
	while( utf8_iterator_prev(&txt->it) >= UTF_PRIVATE0_START );
	txt->flags |= GUI_TEXT_REND_CURSOR | GUI_TEXT_REND_SCROLL;
}

void gui_text_cursor_end(gui_s* gui){
	iassert(gui->type == GUI_TYPE_TEXT);
	guiText_s* txt = gui->control;
	while( *txt->it.str && *txt->it.str != '\n' ){
		gui_text_cursor_next(gui);
	}
	txt->flags |= GUI_TEXT_REND_CURSOR | GUI_TEXT_REND_SCROLL;
}

void gui_text_cursor_home(gui_s* gui){
	iassert(gui->type == GUI_TYPE_TEXT);
	guiText_s* txt = gui->control;
	utf_t utf;
	while( (utf=utf8_iterator_prev(&txt->it)) && utf != '\n' );
	if( utf ) utf8_iterator_next(&txt->it);
	txt->scroll.x = 0;
	txt->flags |= GUI_TEXT_REND_CURSOR | GUI_TEXT_REND_SCROLL;
}

void gui_text_cursor_scroll_left(gui_s* gui){
	iassert(gui->type == GUI_TYPE_TEXT);
	guiText_s* txt = gui->control;
	if( txt->flags & GUI_TEXT_SCROLL_X && txt->scroll.x > 0 ){
	   	txt->scroll.x = txt->scroll.x > txt->spacesize ?  txt->scroll.x - txt->spacesize : 0;
	}
	txt->flags |= GUI_TEXT_REND_SCROLL;
}

void gui_text_cursor_scroll_right(gui_s* gui){
	iassert(gui->type == GUI_TYPE_TEXT);
	guiText_s* txt = gui->control;
	if( txt->flags & GUI_TEXT_SCROLL_X && txt->scroll.x + gui->surface->img->w + txt->spacesize < txt->render->img->w ){
		txt->scroll.x += txt->spacesize;
	}
	txt->flags |= GUI_TEXT_REND_SCROLL;
}

err_t gui_text_cursor_up(gui_s* gui){
	iassert(gui->type == GUI_TYPE_TEXT);
	guiText_s* txt = gui->control;
	const size_t col = gui_text_line_left_len(gui);
	utf8_t* bl = gui_text_back_line_ptr(gui);
	if( !bl ) return -1;
	txt->it.str = bl;
	size_t linelen = gui_text_line_right_len(gui);
	if( linelen > col ) linelen = col;
	while( *txt->it.str && *txt->it.str != '\n' && linelen-->0 ){
		gui_text_cursor_next(gui);
	}
	txt->flags |= GUI_TEXT_REND_CURSOR | GUI_TEXT_REND_SCROLL;
	return 0;
}

err_t gui_text_cursor_down(gui_s* gui){
	iassert(gui->type == GUI_TYPE_TEXT);
	guiText_s* txt = gui->control;
	const size_t col = gui_text_line_left_len(gui);
	utf8_t* nl = gui_text_next_line_ptr(gui);
	if( !nl ) return -1;
	txt->it.str = nl;
	size_t linelen = gui_text_line_right_len(gui);
	if( linelen > col ) linelen = col;
	while( *txt->it.str && *txt->it.str != '\n' && linelen-->0 ){
		gui_text_cursor_next(gui);
	}
	txt->flags |= GUI_TEXT_REND_CURSOR | GUI_TEXT_REND_SCROLL;
	return 0;
}

void gui_text_cursor_pagdn(gui_s* gui){
	iassert(gui->type == GUI_TYPE_TEXT);
	guiText_s* txt = gui->control;
	const size_t col = gui_text_line_left_len(gui);
	const unsigned fullLine = gui->surface->img->h / txt->cursor.h;
	const unsigned dwnLine = fullLine - (txt->cursor.x-txt->scroll.x) / txt->cursor.h;
	unsigned i = fullLine+dwnLine;
	utf8_t* nl = NULL;
   	while( i-->0 && (nl=gui_text_next_line_ptr(gui)) ){
		txt->it.str = nl;
	}
	if( nl ){
		txt->flags |= GUI_TEXT_REND_CURSOR | GUI_TEXT_REND_SCROLL;
		gui_text_redraw(gui, 0);
		i = dwnLine;
		while( i-->0 && (nl=gui_text_back_line_ptr(gui)) ){
			txt->it.str = nl;
		}
	}
	size_t linelen = gui_text_line_right_len(gui);
	if( linelen > col ) linelen = col;
	while( *txt->it.str && *txt->it.str != '\n' && linelen-->0 ){
		gui_text_cursor_next(gui);
	}
	txt->flags |= GUI_TEXT_REND_CURSOR | GUI_TEXT_REND_SCROLL;
}

void gui_text_cursor_pagup(gui_s* gui){
	iassert(gui->type == GUI_TYPE_TEXT);
	guiText_s* txt = gui->control;
	const size_t col = gui_text_line_left_len(gui);
	const unsigned fullLine = gui->surface->img->h / txt->cursor.h;
	const unsigned upLine = (txt->cursor.x-txt->scroll.x) / txt->cursor.h;
	unsigned i = fullLine+upLine;
	utf8_t* nl = NULL;
   	while( i-->0 && (nl=gui_text_back_line_ptr(gui)) ){
		txt->it.str = nl;
	}
	if( nl ){
		txt->flags |= GUI_TEXT_REND_CURSOR | GUI_TEXT_REND_SCROLL;
		gui_text_redraw(gui, 0);
		i = upLine;
		while( i-->0 && (nl=gui_text_next_line_ptr(gui)) ){
			txt->it.str = nl;
		}
	}
	size_t linelen = gui_text_line_right_len(gui);
	if( linelen > col ) linelen = col;
	while( *txt->it.str && *txt->it.str != '\n' && linelen-->0 ){
		gui_text_cursor_next(gui);
	}
	txt->flags |= GUI_TEXT_REND_CURSOR | GUI_TEXT_REND_SCROLL;
}

void gui_text_cursor_on_position(gui_s* gui, unsigned x, unsigned y){
	iassert(gui->type == GUI_TYPE_TEXT);
	guiText_s* txt = gui->control;
	const int scrollXEnable = txt->flags & GUI_TEXT_SCROLL_X;
	const int scrollYEnable = txt->flags & GUI_TEXT_SCROLL_Y;
	const unsigned w = gui->surface->img->w;
	const unsigned h = gui->surface->img->h;
	
	g2dCoord_s cursor = { 0, 0, txt->cursor.w, txt->cursor.h};
	
	utf8Iterator_s it = utf8_iterator(txt->text, 0);

	utf_t u;
	while( (u=utf8_iterator_next(&it)) ){
		if( u >= UTF_PRIVATE0_START ){
			continue;
		}
		if( u == '\n' ){
			if( y > (cursor.y-txt->scroll.y) && y < (cursor.y-txt->scroll.y) + cursor.h ){
				txt->it.str = it.str - 1;
				break;
			}
			if( scrollYEnable ){
				cursor.x = 0;
				cursor.y += txt->cursor.h;	
				continue;
			}
		}
		if( u == '\t' ){
			if( !scrollXEnable && cursor.x + txt->spacesize * txt->tabspace >= w ){
				if( scrollYEnable ){	
					cursor.x = 0;
					cursor.y += txt->cursor.h;
					if( y > (cursor.y-txt->scroll.y) && y < (cursor.y-txt->scroll.y) + cursor.h &&
						x > (cursor.x-txt->scroll.x) && x < (cursor.x-txt->scroll.x) + txt->spacesize * txt->tabspace ){
						txt->it.str = it.str - 1;
						break;
					}
					continue;
				}
				if( y > (cursor.y-txt->scroll.y) && y < (cursor.y-txt->scroll.y) + cursor.h &&
					x > (cursor.x-txt->scroll.x) && x < (cursor.x-txt->scroll.x) + txt->spacesize * txt->tabspace ){
					txt->it.str = it.str;
					break;
				}
				break;
			}
			if( y > (cursor.y-txt->scroll.y) && y < (cursor.y-txt->scroll.y) + cursor.h &&
				x > (cursor.x-txt->scroll.x) && x < (cursor.x-txt->scroll.x) + txt->spacesize * txt->tabspace ){
				txt->it.str = it.str;
				break;
			}
			cursor.x += txt->spacesize * txt->tabspace;
			continue;
		}
		
		ftRender_s* rch = ft_fonts_glyph_load(txt->fonts, u, FT_RENDER_ANTIALIASED | FT_RENDER_VALID);
		if( !rch ){
			dbg_warning("invalid render glyph");
			continue;
		}
		if( rch->horiBearingX < 0 ){
			unsigned const phBX = -rch->horiBearingX;
			if( cursor.x < phBX ) 
				cursor.x = 0;
			else
				cursor.x -= phBX;
		}

		if( !scrollXEnable && (cursor.x + rch->horiAdvance >= w) ){
			if( scrollYEnable ){
				cursor.y += txt->cursor.h;
				cursor.x = 0;
			}
			else if( cursor.y + txt->cursor.h > h ){
				break;
			}
		}

		dbg_info("x:%u y:%u cx:%u cy:%u cw:%u ch:%u", x, y, 
				cursor.x-txt->scroll.x, cursor.y-txt->scroll.y, (cursor.x-txt->scroll.x) + rch->horiAdvance,(cursor.y-txt->scroll.y) + cursor.h);
		if( y > (cursor.y-txt->scroll.y) && y < (cursor.y-txt->scroll.y) + cursor.h &&
			x > (cursor.x-txt->scroll.x) && x < (cursor.x-txt->scroll.x) + rch->horiAdvance ){
			txt->it.str = it.str - 1;
			break;
		}

		cursor.x += rch->horiAdvance;
	}
	
	txt->flags |= GUI_TEXT_REND_CURSOR | GUI_TEXT_REND_CURON;
}

void gui_text_render_cursor(gui_s* gui, __unused guiImage_s* img, __unused void* ud){
	iassert(gui->type == GUI_TYPE_TEXT);
	guiText_s* txt = gui->control;
	if( !(txt->flags & GUI_TEXT_REND_CURSOR) ) return;
	if( !(txt->flags & GUI_TEXT_CUR_VISIBLE) ) return;
	if( !(txt->flags & GUI_TEXT_REND_CURON)  ) return;

	g2dCoord_s cursor = txt->cursor;
	
	cursor.x = cursor.x > cursor.w+1 ? cursor.x - cursor.w + 1 : 0;
	
	iassert(cursor.y >= txt->scroll.y);
	cursor.y -= txt->scroll.y;
	
	iassert(cursor.x >= txt->scroll.x);
	cursor.x -= txt->scroll.x;

	dbg_info("relative x %u y %u w %u", cursor.x, cursor.y, cursor.w);

	g2dPoint_s lineST = { .x = cursor.x, .y = cursor.y };
	g2dPoint_s lineEN = { .x = cursor.x, .y = cursor.y + cursor.h - 1};
	iassert( lineST.x < gui->surface->img->w);
	iassert( lineST.y < gui->surface->img->h);
	iassert( lineEN.x < gui->surface->img->w);
	iassert( lineEN.y < gui->surface->img->h);

	for( unsigned i = 0; i < cursor.w; ++i){
		iassert(lineST.x + i < gui->surface->img->w);
		g2d_line(gui->surface->img, &lineST, &lineEN, txt->colCursor, 1);
		++lineST.x;
		++lineEN.x;
	}

	txt->flags &= ~GUI_TEXT_REND_CURSOR;
}

void gui_text_render_text(gui_s* gui, int partial){
	iassert(gui->type == GUI_TYPE_TEXT);
	guiText_s* txt = gui->control;
	const int scrollXEnable = txt->flags & GUI_TEXT_SCROLL_X;
	const int scrollYEnable = txt->flags & GUI_TEXT_SCROLL_Y;
	const unsigned w = gui->surface->img->w;
	const unsigned h = gui->surface->img->h;
	const g2dColor_t colClear = gui_color(0,0,0,0);
	int select = 0;
	g2dCoord_s newCursor = { 0, 0, txt->cursor.w, txt->cursor.h};
	g2dCoord_s cursor = newCursor;

	//g2dCoord_s part;
	g2dCoord_s co = { 
		.x = 0, 
		.y = 0, 
		.w = scrollXEnable ? ft_multiline_lenght(txt->fonts, txt->text) : w,
		.h = 
			scrollYEnable ? 
				scrollXEnable ? ft_multiline_height(txt->fonts, txt->text) : ft_autowrap_height(txt->fonts, txt->text, w)
			:
				h
	};
	if( co.h < h ) co.h = h;
	if( co.w < w ) co.w = w;
	co.h += cursor.h;

	if( (txt->render->img->w < co.w || txt->render->img->h < co.h) ){
		co.w += txt->cursor.h;
		co.h += txt->cursor.h;
		dbg_info("resize render %u*%u", co.w, co.h);
		g2d_free(txt->render->img);
		txt->render->img = g2d_new(co.w, co.h, -1);
		g2d_clear(txt->render->img, colClear, &co);
		partial = 0;
		txt->flags |= GUI_TEXT_REND_TEXT;
	}
	else{
		co.w = txt->render->img->w;
		co.h = txt->render->img->h;
		if( !partial && txt->flags & GUI_TEXT_REND_TEXT ){
			dbg_info("clear render %u*%u", co.w, co.h);
			g2d_clear(txt->render->img, colClear, &co);
		}
	}

	utf8Iterator_s it = utf8_iterator(txt->text, 0);

	utf_t u;
	while( (u=utf8_iterator_next(&it)) ){
		if( txt->selStart != txt->selEnd ){
		    if( txt->selStart < txt->selEnd ){
				if( txt->selStart == it.str )
					select = 1;
				else if( txt->selEnd == it.str )
					select = 0;
			}
			if( txt->selStart > txt->selEnd ){
				if( txt->selEnd == it.str )
					select = 1;
				else if( txt->selStart == it.str )
					select = 0;
			}
		}

		if( u >= UTF_PRIVATE0_START ){
			continue;
		}
		if( u == '\n' ){
			if( scrollYEnable ){
				cursor.x = 0;
				cursor.y += txt->cursor.h;
				if( it.str == txt->it.str ){
					newCursor.x = cursor.x;
					newCursor.y = cursor.y;
				}
				continue;
			}
			if( it.str == txt->it.str ){
					newCursor.x = cursor.x;
					newCursor.y = cursor.y;
			}
			break;
		}
		if( u == '\t' ){
			if( !scrollXEnable && cursor.x + txt->spacesize * txt->tabspace >= w ){
				if( scrollYEnable ){
					cursor.y += txt->cursor.h;
					cursor.x = 0;
					if( it.str == txt->it.str ){
						newCursor.x = cursor.x;
						newCursor.y = cursor.y;
					}
					continue;
				}
				if( it.str == txt->it.str ){
					newCursor.x = cursor.x;
					newCursor.y = cursor.y;
				}
				break;
			}
			cursor.x += txt->spacesize * txt->tabspace;
			if( it.str == txt->it.str ){
				newCursor.x = cursor.x;
				newCursor.y = cursor.y;
			}
			continue;
		}
		
		ftRender_s* rch = ft_fonts_glyph_load(txt->fonts, u, FT_RENDER_ANTIALIASED | FT_RENDER_VALID);
		if( !rch ){
			dbg_warning("invalid render glyph");
			continue;
		}
		if( rch->horiBearingX < 0 ){
			unsigned const phBX = -rch->horiBearingX;
			if( cursor.x < phBX ) 
				cursor.x = 0;
			else
				cursor.x -= phBX;
		}

		if( !scrollXEnable && (cursor.x + rch->horiAdvance >= w) ){
			if( scrollYEnable ){
				cursor.y += txt->cursor.h;
				cursor.x = 0;
			}
			else if( cursor.y + txt->cursor.h > h ){
				break;
			}
		}

		if( !partial && txt->flags & GUI_TEXT_REND_TEXT ){
			dbg_info("putch render %u %u %u*%u", cursor.x, cursor.y, rch->horiAdvance, cursor.h);
			if( select ){
				g2dCoord_s chs = { .x = cursor.x, .y = cursor.y, .w = rch->horiAdvance, .h = cursor.h};
				g2d_clear(txt->render->img, txt->select,  &chs);
				g2d_char(txt->render->img, &cursor, rch->img, txt->foreground);
			}
			else{
				g2d_char_indirect(txt->render->img, &cursor, rch->img, txt->foreground);
			}
		}

		cursor.x += rch->horiAdvance;
		if( it.str == txt->it.str ){
			newCursor.x = cursor.x;
			newCursor.y = cursor.y;
		}
	}
	
	txt->cursor = newCursor;

	txt->flags &= ~GUI_TEXT_REND_TEXT;
}

void gui_text_render_scroll(gui_s* gui){
	iassert(gui->type == GUI_TYPE_TEXT);
	guiText_s* txt = gui->control;
	if( !(txt->flags & GUI_TEXT_REND_SCROLL) ) return;

	const int scrollXEnable = txt->flags & GUI_TEXT_SCROLL_X;
	const int scrollYEnable = txt->flags & GUI_TEXT_SCROLL_Y;
	const unsigned w = gui->surface->img->w;
	const unsigned h = gui->surface->img->h;

	if( txt->cursor.x <= txt->cursor.w ){
		txt->cursor.x = 0;
		txt->scroll.x = 0;
	}
	else if( txt->scroll.x && txt->cursor.x - (txt->cursor.w+1) <= txt->scroll.x ){
		dbg_info("scroll.x && cursor.x(%u) < scroll.x(%u):: scroll.x(%u)", txt->cursor.x, txt->scroll.x, (txt->cursor.x - txt->cursor.w)+1);
		txt->scroll.x = (txt->cursor.x - txt->cursor.w+1);
	}
	else if( (txt->cursor.x + txt->cursor.w) - txt->scroll.x >= w ){
		if( scrollXEnable ){
			dbg_info("scrollEnable && cursor.x(%u) - scroll.x(%u) > w(%u):: scroll.x(%u)", txt->cursor.x, txt->scroll.x, w, w-txt->cursor.x);
			txt->scroll.x = (txt->cursor.x+txt->cursor.w) - w;
		}
		else{
			dbg_info("!scrollEnable && cursor.x(%u) - scroll.x(%u) > w(%u):: scroll.x(%u) newCursor.x(%u)", txt->cursor.x, txt->scroll.x, w, 0, txt->cursor.x);
			txt->scroll.x = 0;
			txt->cursor.x = w;
		}
	}

	if( txt->scroll.y && txt->cursor.y < txt->scroll.y ){
		dbg_warning("to test");
		txt->scroll.y = txt->cursor.y;
	}
	else if( (txt->cursor.y + txt->cursor.h) - txt->scroll.y >= h ){
		if( scrollYEnable ){
			dbg_info("scroll && cursor.y(%u) scroll.y(%u) h(%u) cursor.h(%u)", txt->cursor.y, txt->scroll.y, h, txt->cursor.h);
			dbg_info("%u",((txt->cursor.y + txt->cursor.h) - txt->scroll.y) - h );	
			txt->scroll.y = (txt->cursor.y + txt->cursor.h) - h;
		}
		else{
			dbg_warning("not need?");
			txt->scroll.y = 0;
			txt->cursor.y = h - txt->cursor.h;
		}
	}
	dbg_info("calcolate scrolling %u+%u", txt->scroll.x, txt->scroll.y);

	txt->flags &= ~GUI_TEXT_REND_SCROLL;
}

void gui_text_redraw(gui_s* gui, int partial){
	iassert(gui->type == GUI_TYPE_TEXT);
	guiText_s* txt = gui->control;
	gui_text_render_text(gui, partial);
	gui_text_render_scroll(gui);
	
	txt->render->pos.x = 0, 
	txt->render->pos.y = 0,
	txt->render->pos.w = gui->surface->img->w;
	txt->render->pos.h = gui->surface->img->h;
	txt->render->src.x = txt->scroll.x;
	txt->render->src.y = txt->scroll.y,
	txt->render->src.w = gui->surface->img->w;
	txt->render->src.h = gui->surface->img->h;

	gui_composite_redraw(gui, gui->img);
}

int gui_text_event_key(gui_s* gui, xorgEvent_s* event){
	iassert(gui->type == GUI_TYPE_TEXT);
	guiText_s* txt = gui->control;
	int partial = 0;

	if( event->keyboard.event == XORG_KEY_RELEASE ){
		if( event->keyboard.keysym == XKB_KEY_Escape )
			gui_focus_next(gui);
		return 0;
	}

	switch( event->keyboard.keysym ){
		case XKB_KEY_Shift_L:
		case XKB_KEY_Shift_R:
			gui_text_sel(gui);
		break;

		case XKB_KEY_BackSpace:
			if( txt->selStart ){
				gui_text_sel_del(gui);
			}
			else{
				gui_text_backspace(gui);
			}
		break;

		case XKB_KEY_Delete:
			if( txt->selStart ){
				gui_text_sel_del(gui);
			}
			else{
				gui_text_del(gui);
			}
		break;

		case XKB_KEY_Left:
			if( event->keyboard.modifier & XORG_KEY_MOD_CONTROL ){
				utf_t u;
				while( (u=utf8_iterator_prev(&txt->it)) && !strchr(GUI_TEXT_WORD_SEP,u) );
				if( u ) utf8_iterator_next(&txt->it);
				txt->flags |= GUI_TEXT_REND_SCROLL | GUI_TEXT_REND_CURSOR;
			}
			else{
				gui_text_cursor_prev(gui->control);
			}
			if( event->keyboard.modifier & XORG_KEY_MOD_SHIFT ) gui_text_sel(gui);
			else gui_text_unsel(gui->control);
		break;

		case XKB_KEY_Right:
			if( event->keyboard.modifier & XORG_KEY_MOD_CONTROL ){
				utf_t u;
				while( (u=utf8_iterator_next(&txt->it)) && !strchr(GUI_TEXT_WORD_SEP,u) );
				if( u ) utf8_iterator_prev(&txt->it);
				txt->flags |= GUI_TEXT_REND_SCROLL | GUI_TEXT_REND_CURSOR;
			}
			else{
				gui_text_cursor_next(gui->control);
			}
			if( event->keyboard.modifier & XORG_KEY_MOD_SHIFT ) gui_text_sel(gui);
			else gui_text_unsel(gui->control);
		break;

		case XKB_KEY_Up:
			gui_text_cursor_up(gui->control);
			if( event->keyboard.modifier & XORG_KEY_MOD_SHIFT ) gui_text_sel(gui);
			else gui_text_unsel(gui->control);
		break;

		case XKB_KEY_Down:
			gui_text_cursor_down(gui->control);
			if( event->keyboard.modifier & XORG_KEY_MOD_SHIFT ) gui_text_sel(gui);
			else gui_text_unsel(gui->control);
		break;

		case XKB_KEY_Page_Up:
			gui_text_cursor_pagup(gui);
			if( event->keyboard.modifier & XORG_KEY_MOD_SHIFT ) gui_text_sel(gui);
			else gui_text_unsel(gui->control);
		break;

		case XKB_KEY_Page_Down:
			gui_text_cursor_pagdn(gui);
			if( event->keyboard.modifier & XORG_KEY_MOD_SHIFT ) gui_text_sel(gui);	
			else gui_text_unsel(gui->control);
		break;

		case XKB_KEY_Home:
		case XKB_KEY_Begin:
			gui_text_cursor_home(gui);
			if( event->keyboard.modifier & XORG_KEY_MOD_SHIFT ) gui_text_sel(gui);	
			else gui_text_unsel(gui->control);
		break;

		case XKB_KEY_End:
			gui_text_cursor_end(gui->control);
			if( event->keyboard.modifier & XORG_KEY_MOD_SHIFT ) gui_text_sel(gui);	
			else gui_text_unsel(gui->control);
		break;

		case XKB_KEY_Insert:
			gui_text_ir_toggle(gui->control);
		break;

		case XKB_KEY_Return:
			if( txt->selStart ){
				gui_text_sel_del(gui->control);
			}
			gui_text_put(gui, '\n');
		break;

		case XKB_KEY_Tab:
			if( txt->selStart ){
				gui_text_sel_del(gui->control);
			}
			gui_text_put(gui, '\t');
		break;

		default:
			dbg_info("modifier 0x%X 0x%X 0x%X", event->keyboard.modifier, XORG_KEY_MOD_CONTROL, XORG_KEY_MOD_CONTROL_L);
			if( (event->keyboard.modifier & XORG_KEY_MOD_CONTROL) ){
				switch( event->keyboard.keysym ){
					case XKB_KEY_v:
						gui_clipboard_paste(gui, 0);
					break;

					case XKB_KEY_c:
						gui_clipboard_copy(gui, 0);
						if( txt->clipmem ) free(txt->clipmem);
						txt->clipmem = gui_text_sel_get(gui);
					break;
				
					case XKB_KEY_x:
						gui_clipboard_copy(gui, 0);
						if( txt->clipmem ) free(txt->clipmem);
						txt->clipmem = gui_text_sel_get(gui);
						if( txt->selStart ) gui_text_sel_del(gui);
					break;

					default:
						//txt->flags |= GUI_TEXT_SEL;	
					break;
				}
			}
			else if( event->keyboard.utf){
				if( txt->selStart ){
					gui_text_sel_del(gui->control);
				}
				gui_text_put(gui, event->keyboard.utf);
			}
			else{
				partial = -1;
			}
		break;
	}

	//if( !(txt->flags & GUI_TEXT_SEL ) ){
	//	gui_text_unsel(gui->control);
	//}

	if( !partial ){
		txt->flags |= GUI_TEXT_REND_CURON;
		gui_text_redraw(gui, partial);
		gui_draw(gui);
	}

	return 0;
}

int gui_text_event_redraw(gui_s* gui, __unused xorgEvent_s* ev){
	iassert(gui->type == GUI_TYPE_TEXT);
	gui_text_redraw(gui, 0);
	return 0;
}

int gui_text_event_clipboard(gui_s* gui, xorgEvent_s* ev){
	iassert(gui->type == GUI_TYPE_TEXT);
	if( ev->clipboard.eventPaste ){
		if( ev->clipboard.data ){
			gui_text_puts(gui, ev->clipboard.data);
			free(ev->clipboard.data);
		}
	}
	else{
		guiText_s* txt = gui->control;
		if( ev->clipboard.primary || !txt->clipmem ){
			utf8_t* sel = gui_text_sel_get(gui->control);
			if( !sel ){
				return 0;
			}
			gui_clipboard_send(&ev->clipboard, sel, strlen((char*)sel));
			free(sel);
		}
		else{
			gui_clipboard_send(&ev->clipboard, txt->clipmem, strlen((char*)txt->clipmem));
		}
	}
	return 0;
}

int gui_text_event_free(gui_s* gui, __unused xorgEvent_s* ev){
	iassert(gui->type == GUI_TYPE_TEXT);
	gui_text_free(gui->control);
	return 0;
}

int gui_text_timer_blink(guiTimer_s* timer){
	gui_s* gui = timer->userdata;
	guiText_s* txt = gui->control;
	txt->flags |= GUI_TEXT_REND_CURSOR;
	gui_text_redraw(gui, 0);
	gui_draw(gui);

	if( txt->flags & GUI_TEXT_REND_CURON ){
		txt->flags &= ~GUI_TEXT_REND_CURON;
	}
	else{
		txt->flags |= GUI_TEXT_REND_CURON;
	}
	return GUI_TIMER_NEXT;
}

int gui_text_event_focus(gui_s* gui, xorgEvent_s* ev){
	iassert(gui->type == GUI_TYPE_TEXT);
	guiText_s* txt = gui->control;
	if( ev->focus.outin ){
		txt->flags |= GUI_TEXT_REND_CURON;
		if( txt->blinktime && !txt->blink ) txt->blink = gui_timer_new(gui, txt->blinktime, gui_text_timer_blink, gui);
	}
	else{
		if( txt->blink ) gui_timer_free(txt->blink);
		txt->blink = NULL;
		txt->flags &= ~GUI_TEXT_REND_CURON;
	}
	txt->flags |= GUI_TEXT_REND_CURSOR;
	gui_text_redraw(gui, 0);
	gui_draw(gui);

	if( ev->focus.outin ){
		txt->flags &= ~GUI_TEXT_REND_CURON;
	}
	else{
		txt->flags |= GUI_TEXT_REND_CURON;
	}

	return 0;
}

int gui_text_event_mouse(gui_s* gui, xorgEvent_s* event){
	iassert( gui->type == GUI_TYPE_TEXT );
	guiText_s* txt = gui->control;

	if( event->mouse.event == XORG_MOUSE_CLICK && event->mouse.button == 1 ){
		gui_text_unsel(gui);
		gui_text_cursor_on_position(gui, event->mouse.relative.x, event->mouse.relative.y);
		gui_text_redraw(gui, 0);
		gui_draw(gui);
	}
	else if( event->mouse.event == XORG_MOUSE_RELEASE && event->mouse.button == 1 ){
		txt->flags &= ~GUI_TEXT_SEL;
		gui_text_redraw(gui, 0);
		gui_draw(gui);
	}
	else if( event->mouse.event == XORG_MOUSE_RELEASE && event->mouse.button == 2 ){
		if( txt->selStart ) gui_text_sel_del(gui);
		gui_clipboard_paste(gui, 1);
	}
	else if( event->mouse.event == XORG_MOUSE_PRESS && event->mouse.button == 1 ){
		gui_text_unsel(gui);
		gui_text_cursor_on_position(gui, event->mouse.relative.x, event->mouse.relative.y);
		gui_text_sel(gui);
		gui_text_redraw(gui, 0);
		gui_draw(gui);
	}
	else if( (txt->flags & GUI_TEXT_SEL) && event->mouse.event == XORG_MOUSE_MOVE ){
		gui_text_cursor_on_position(gui, event->mouse.relative.x, event->mouse.relative.y);
		gui_text_sel(gui);
		gui_text_redraw(gui, 0);
		gui_draw(gui);
	}else if( event->mouse.event == XORG_MOUSE_DBLCLICK && event->mouse.button == 1 ){
		gui_text_unsel(gui);
		utf_t u;
		utf8Iterator_s it = txt->it;
		while( (u=utf8_iterator_prev(&it)) && !strchr(GUI_TEXT_WORD_SEP,u) );
		if( u ) utf8_iterator_next(&it);
		txt->selStart = it.str+1;
		while( (u=utf8_iterator_next(&it)) && !strchr(GUI_TEXT_WORD_SEP,u) );
		txt->selEnd = *it.str ? it.str : it.str+1;
		txt->flags |= GUI_TEXT_REND_TEXT | GUI_TEXT_REND_SCROLL | GUI_TEXT_REND_CURSOR;
		gui_text_redraw(gui, 0);
		gui_draw(gui);
	}

	gui_event_mouse(gui, event);
	return 0;
}

int gui_text_event_move(gui_s* gui, xorgEvent_s* event){
	iassert(gui->type == GUI_TYPE_TEXT);
	//guiText_s* txt = gui->control;
	gui_event_move(gui, event);
	gui_text_redraw(gui, 0);
	gui_draw(gui);
	return 0;
}

int gui_text_event_themes(gui_s* gui, xorgEvent_s* ev){
	iassert(gui->type == GUI_TYPE_TEXT);
	guiText_s* txt = gui->control;
	char* name = ev->data.data;

	gui_themes_fonts_set(name, &txt->fonts);
	if( !gui_themes_uint_set(name, GUI_THEME_TEXT_FOREGROUND, &txt->foreground) ) txt->flags |= GUI_TEXT_REND_TEXT;
	gui_themes_uint_set(name, GUI_THEME_TEXT_BLINK, &txt->blinktime);
	gui_themes_uint_set(name, GUI_THEME_TEXT_CURSOR_COLOR, &txt->colCursor);
	gui_themes_uint_set(name, GUI_THEME_TEXT_SEL_COLOR, &txt->select);
	gui_themes_uint_set(name, GUI_THEME_TEXT_TAB, &txt->tabspace);
	
	int vbool;
	if( gui_themes_bool_set(name, GUI_THEME_TEXT_SCROLL_X , &vbool) ){
		if( vbool ) txt->flags |= GUI_TEXT_SCROLL_X;
		else        txt->flags &= ~GUI_TEXT_SCROLL_X;
		txt->flags |= GUI_TEXT_REND_TEXT | GUI_TEXT_REND_CURSOR | GUI_TEXT_REND_SCROLL;
	}

	if( gui_themes_bool_set(name, GUI_THEME_TEXT_SCROLL_Y , &vbool) ){
		if( vbool ) txt->flags |= GUI_TEXT_SCROLL_Y;
		else        txt->flags &= ~GUI_TEXT_SCROLL_Y;
		txt->flags |= GUI_TEXT_REND_TEXT | GUI_TEXT_REND_CURSOR | GUI_TEXT_REND_SCROLL;
	}

	__mem_free char* cursor = gui_themes_string(name, GUI_THEME_TEXT_CURSOR);
	if( cursor ){
		if( !strcmp(cursor, "thin") ) gui_text_cursor_change(gui, GUI_TEXT_CURSOR_THIN);
		else if( !strcmp(cursor, "light") )  gui_text_cursor_change(gui, GUI_TEXT_CURSOR_THIN);
		else if( !strcmp(cursor, "plentiful") ) gui_text_cursor_change(gui, GUI_TEXT_CURSOR_PLENTIFUL);
		else if( !strcmp(cursor, "fat") ) gui_text_cursor_change(gui, GUI_TEXT_CURSOR_FAT);
	}

	return 0;
}

