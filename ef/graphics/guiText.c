#include <ef/guiText.h>
#include <ef/memory.h>
#include <ef/str.h>
#include <ef/err.h>

guiText_s* gui_text_new(ftFonts_s* font, g2dColor_t foreground, g2dColor_t colCursor, unsigned tabspace, unsigned blinktime, unsigned flags){
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

	dbg_info("genric glyph size: %u*%u", txt->cursor.w, txt->cursor.h);

	txt->len = 0;
	txt->size = 0;
	txt->resize = 0;
	txt->text = NULL;

	txt->fonts = font;
	txt->foreground = foreground;
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
	gui->free = gui_text_event_free;
	gui->focus = gui_text_event_focus;
	gui->focusable = 1;

	txt->resize = txt->size = (gui->surface->img->w / ft_line_lenght(txt->fonts, U8(" "))) * (gui->surface->img->h / txt->cursor.h);
	txt->text = mem_many(utf8_t, txt->resize);
	if( !txt->text ) err_fail("eom");
	*txt->text = 0;
	txt->it = utf8_iterator(txt->text, 0);

	return gui;
ERR:
	if( txt ) gui_text_free(txt);
	if( gui ) gui_free(gui);
	return NULL;
}

void gui_text_free(guiText_s* txt){
	if( txt->text ) free(txt->text);
	if( txt->render ) free(txt->render);
	free(txt);
}

void gui_text_flags_set(guiText_s* txt, unsigned flags){
	txt->flags = flags;
}

void gui_text_ir_toggle(guiText_s* txt){
	if( txt->flags & GUI_TEXT_INSERT ){
		txt->flags &= ~GUI_TEXT_INSERT;
	}
	else{
		txt->flags |= GUI_TEXT_INSERT;
	}
	
}

const utf8_t* gui_text_str_raw(guiText_s* txt){
	return txt->text;
}

utf8_t* gui_text_str(guiText_s* txt){
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

size_t gui_text_line_right_len(guiText_s* txt){
	utf8Iterator_s it = txt->it;
	size_t width = 0;
	utf_t utf;
	while( (utf=utf8_iterator_next(&it)) && utf != '\n' ) ++width;
	return width;
}

size_t gui_text_line_left_len(guiText_s* txt){
	utf8Iterator_s it = txt->it;
	size_t width = 0;
	utf_t utf;
	while( (utf=utf8_iterator_prev(&it)) && utf != '\n' ) ++width;
	return width;
}

utf8_t* gui_text_back_line_ptr(guiText_s* txt){
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

utf8_t* gui_text_next_line_ptr(guiText_s* txt){
	utf8Iterator_s it = txt->it;
	utf_t u;
	while( (u=utf8_iterator_next(&it)) ){
		if( u == '\n' ){
			return it.str;
		}
	}
	return NULL;
}

void gui_text_put(__unused gui_s* gui, guiText_s* txt, utf_t utf){

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

void gui_text_puts(gui_s* gui, guiText_s* txt, utf8_t* str){
	utf8Iterator_s it = utf8_iterator(str, 0);
	utf_t utf;
	while( (utf=utf8_iterator_next(&it)) ){
		gui_text_put(gui, txt, utf);
	}
}

void gui_text_del(guiText_s* txt){
	utf8_iterator_delete(&txt->it);
	txt->flags |= GUI_TEXT_REND_TEXT | GUI_TEXT_REND_CURSOR | GUI_TEXT_REND_SCROLL;
}

void gui_text_backspace(guiText_s* txt){
	if( utf8_iterator_prev(&txt->it) ){
		utf8_iterator_delete(&txt->it);
		txt->flags |= GUI_TEXT_REND_TEXT | GUI_TEXT_REND_CURSOR | GUI_TEXT_REND_SCROLL;
	}
}

void gui_text_cursor_next(guiText_s* txt){
	utf_t u;
	while( (u=utf8_iterator_next(&txt->it)) >= UTF_PRIVATE0_START );
	txt->flags |= GUI_TEXT_REND_CURSOR | GUI_TEXT_REND_SCROLL;
}

void gui_text_cursor_prev(guiText_s* txt){
	utf_t utf;
	while( (utf=utf8_iterator_prev(&txt->it)) >= UTF_PRIVATE0_START );
	txt->flags |= GUI_TEXT_REND_CURSOR | GUI_TEXT_REND_SCROLL;
}

void gui_text_cursor_end(guiText_s* txt){
	while( *txt->it.str && *txt->it.str != '\n' ){
		gui_text_cursor_next(txt);
	}
	txt->flags |= GUI_TEXT_REND_CURSOR | GUI_TEXT_REND_SCROLL;
}

void gui_text_cursor_home(guiText_s* txt){
	utf_t utf;
	while( (utf=utf8_iterator_prev(&txt->it)) && utf != '\n' );
	if( utf ) utf8_iterator_next(&txt->it);
	txt->scroll.x = 0;
	txt->flags |= GUI_TEXT_REND_CURSOR | GUI_TEXT_REND_SCROLL;
}

void gui_text_cursor_scroll_left(guiText_s* txt){
	if( txt->flags & GUI_TEXT_SCROLL_X && txt->scroll.x > 0 ){
	   	txt->scroll.x = txt->scroll.x > txt->spacesize ?  txt->scroll.x - txt->spacesize : 0;
	}
	txt->flags |= GUI_TEXT_REND_SCROLL;
}

void gui_text_cursor_scroll_right(gui_s* gui, guiText_s* txt){
	if( txt->flags & GUI_TEXT_SCROLL_X && txt->scroll.x + gui->surface->img->w + txt->spacesize < txt->render->w ){
		txt->scroll.x += txt->spacesize;
	}
	txt->flags |= GUI_TEXT_REND_SCROLL;
}

err_t gui_text_cursor_up(guiText_s* txt){
	const size_t col = gui_text_line_left_len(txt);
	utf8_t* bl = gui_text_back_line_ptr(txt);
	if( !bl ) return -1;
	txt->it.str = bl;
	size_t linelen = gui_text_line_right_len(txt);
	if( linelen > col ) linelen = col;
	while( *txt->it.str && *txt->it.str != '\n' && linelen-->0 ){
		gui_text_cursor_next(txt);
	}
	txt->flags |= GUI_TEXT_REND_CURSOR | GUI_TEXT_REND_SCROLL;
	return 0;
}

err_t gui_text_cursor_down(guiText_s* txt){
	const size_t col = gui_text_line_left_len(txt);
	utf8_t* nl = gui_text_next_line_ptr(txt);
	if( !nl ) return -1;
	txt->it.str = nl;
	size_t linelen = gui_text_line_right_len(txt);
	if( linelen > col ) linelen = col;
	while( *txt->it.str && *txt->it.str != '\n' && linelen-->0 ){
		gui_text_cursor_next(txt);
	}
	txt->flags |= GUI_TEXT_REND_CURSOR | GUI_TEXT_REND_SCROLL;
	return 0;
}

void gui_text_cursor_pagdn(gui_s* gui, guiText_s* txt){
	const size_t col = gui_text_line_left_len(txt);
	const unsigned fullLine = gui->surface->img->h / txt->cursor.h;
	const unsigned dwnLine = fullLine - (txt->cursor.x-txt->scroll.x) / txt->cursor.h;
	unsigned i = fullLine+dwnLine;
	utf8_t* nl;
   	while( i-->0 && (nl=gui_text_next_line_ptr(txt)) ){
		txt->it.str = nl;
	}
	if( nl ){
		txt->flags |= GUI_TEXT_REND_CURSOR | GUI_TEXT_REND_SCROLL;
		gui_text_redraw(gui, gui->background[0], txt, 0);
		i = dwnLine;
		while( i-->0 && (nl=gui_text_back_line_ptr(txt)) ){
			txt->it.str = nl;
		}
	}
	size_t linelen = gui_text_line_right_len(txt);
	if( linelen > col ) linelen = col;
	while( *txt->it.str && *txt->it.str != '\n' && linelen-->0 ){
		gui_text_cursor_next(txt);
	}
	txt->flags |= GUI_TEXT_REND_CURSOR | GUI_TEXT_REND_SCROLL;
}

void gui_text_cursor_pagup(gui_s* gui, guiText_s* txt){
	const size_t col = gui_text_line_left_len(txt);
	const unsigned fullLine = gui->surface->img->h / txt->cursor.h;
	const unsigned upLine = (txt->cursor.x-txt->scroll.x) / txt->cursor.h;
	unsigned i = fullLine+upLine;
	utf8_t* nl;
   	while( i-->0 && (nl=gui_text_back_line_ptr(txt)) ){
		txt->it.str = nl;
	}
	if( nl ){
		txt->flags |= GUI_TEXT_REND_CURSOR | GUI_TEXT_REND_SCROLL;
		gui_text_redraw(gui, gui->background[0], txt, 0);
		i = upLine;
		while( i-->0 && (nl=gui_text_next_line_ptr(txt)) ){
			txt->it.str = nl;
		}
	}
	size_t linelen = gui_text_line_right_len(txt);
	if( linelen > col ) linelen = col;
	while( *txt->it.str && *txt->it.str != '\n' && linelen-->0 ){
		gui_text_cursor_next(txt);
	}
	txt->flags |= GUI_TEXT_REND_CURSOR | GUI_TEXT_REND_SCROLL;
}

void gui_text_render_cursor(gui_s* gui, guiText_s* txt){
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

void gui_text_render_text(gui_s* gui, guiText_s* txt, int partial){
	if( !(txt->flags & GUI_TEXT_REND_TEXT) ) return;

	const int scrollXEnable = txt->flags & GUI_TEXT_SCROLL_X;
	const int scrollYEnable = txt->flags & GUI_TEXT_SCROLL_Y;
	const unsigned w = gui->surface->img->w;
	const unsigned h = gui->surface->img->h;
	const g2dColor_t colClear = gui_color(0,0,0,0);
	g2dCoord_s newCursor = { 0, 0, txt->cursor.w, txt->cursor.h};
	g2dCoord_s cursor = newCursor;

	g2dCoord_s part;
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

	if( !txt->render || txt->render->w < co.w || txt->render->h < co.h ){
		co.w += txt->cursor.h;
		co.h += txt->cursor.h;
		dbg_info("resize render %u*%u", co.w, co.h);
		if( txt->render ) g2d_free(txt->render);
		txt->render = g2d_new(co.w, co.h, -1);
		partial = 0;
	}
	co.w = txt->render->w;
	co.h = txt->render->h;
	if( !partial ){
		dbg_info("clear render %u*%u", co.w, co.h);
		g2d_clear(txt->render, colClear, &co);
	}

	utf8Iterator_s it = utf8_iterator(txt->text, 0);

	utf_t u;
	while( (u=utf8_iterator_next(&it)) ){
		if( partial && (cursor.x >= txt->cursor.x || cursor.y >= txt->cursor.y) ){
			part.x = cursor.x;
			part.y = cursor.y;
			part.h = txt->cursor.h;
			part.w = co.w - part.x;
			dbg_info("partial clear eol render %u %u %u*%u", part.x, part.y, part.w, part.h);
			g2d_clear(txt->render, colClear, &part);
			part.y += txt->cursor.h;
			if( part.y < co.h ){
				part.x = 0;
				part.w = co.w;
				part.h = co.h - part.y;
				dbg_info("partial clear render %u %u %u*%u", part.x, part.y, part.w, part.h);
				g2d_clear(txt->render, colClear, &part);		
			}
			partial = 0;
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

		if( !partial ){
			dbg_info("putch render %u %u %u*%u", cursor.x, cursor.y, rch->horiAdvance, cursor.h);
			g2d_char_indirect(txt->render, &cursor, rch->img, txt->foreground);
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

void gui_text_render_scroll(gui_s* gui, guiText_s* txt){
	if( (txt->flags & GUI_TEXT_REND_SCROLL) ) return;

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

void gui_text_redraw(gui_s* gui, guiBackground_s* bkg, guiText_s* txt, int partial){
	gui_background_redraw(gui, bkg);
	gui_text_render_text(gui, txt, partial);

	if( txt->render ){
		g2dCoord_s pdest = { 
			.x = 0, 
			.y = 0,
			.w = gui->surface->img->w,
			.h = gui->surface->img->h
		};
		g2dCoord_s psrc = { 
			.x = txt->scroll.x, 
			.y = txt->scroll.y,
			.w = gui->surface->img->w,
			.h = gui->surface->img->h
		};
		iassert( psrc.x+psrc.w <= txt->render->w );
		iassert( psrc.y+psrc.h <= txt->render->h );
		g2d_bitblt_alpha(gui->surface->img, &pdest, txt->render, &psrc);
	}

	gui_text_render_scroll(gui, txt);
	gui_text_render_cursor(gui, txt);
}

int gui_text_event_key(gui_s* gui, xorgEvent_s* event){
	iassert(gui->type == GUI_TYPE_TEXT);

	if( event->keyboard.event != XORG_KEY_PRESS ) return 0;
	int partial = 0;

	switch( event->keyboard.keysym ){
		case XKB_KEY_BackSpace:
			gui_text_backspace(gui->control);
		break;

		case XKB_KEY_Delete:
			gui_text_del(gui->control);
		break;

		case XKB_KEY_Left:
			gui_text_cursor_prev(gui->control);
		break;

		case XKB_KEY_Right:
			gui_text_cursor_next(gui->control);
		break;

		case XKB_KEY_Up:
			gui_text_cursor_up(gui->control);
		break;

		case XKB_KEY_Down:
			gui_text_cursor_down(gui->control);
		break;

		case XKB_KEY_Page_Up:
			gui_text_cursor_pagup(gui, gui->control);
		break;

		case XKB_KEY_Page_Down:
			gui_text_cursor_pagdn(gui, gui->control);
		break;

		case XKB_KEY_Home:
		case XKB_KEY_Begin:
			gui_text_cursor_home(gui->control);
		break;

		case XKB_KEY_End:
			gui_text_cursor_end(gui->control);
		break;

		case XKB_KEY_Insert:
			gui_text_ir_toggle(gui->control);
		break;

		case XKB_KEY_Return:
			gui_text_put(gui, gui->control, '\n');
		break;

		case XKB_KEY_Tab:
			gui_text_put(gui, gui->control, '\t');
		break;

		default:
			if( event->keyboard.utf){
				gui_text_put(gui, gui->control, event->keyboard.utf);
			}
			else{
				partial = -1;
			}
		break;
	}

	if( !partial ){
		gui_text_redraw(gui, gui->background[0], gui->control, partial);
		gui_draw(gui);
	}
	return 0;
}

int gui_text_event_redraw(gui_s* gui, __unused xorgEvent_s* ev){
	iassert(gui->type == GUI_TYPE_TEXT);
	gui_text_redraw(gui, gui->background[0], gui->control, 0);
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
	gui_text_redraw(gui, gui->background[0], gui->control, 0);
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
		iassert(txt->blink == NULL);
		txt->flags |= GUI_TEXT_REND_CURON;
		if( txt->blinktime && !txt->blink ) txt->blink = gui_timer_new(gui, txt->blinktime, gui_text_timer_blink, gui);
	}
	else{
		if( txt->blink ) gui_timer_free(txt->blink);
		txt->blink = NULL;
		txt->flags &= ~GUI_TEXT_REND_CURON;
	}
	txt->flags |= GUI_TEXT_REND_CURSOR;
	gui_text_redraw(gui, gui->background[0], gui->control, 0);
	gui_draw(gui);

	if( ev->focus.outin ){
		txt->flags &= ~GUI_TEXT_REND_CURON;
	}
	else{
		txt->flags |= GUI_TEXT_REND_CURON;
	}

	return 0;
}


