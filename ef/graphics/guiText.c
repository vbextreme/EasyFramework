#include <ef/guiText.h>
#include <ef/memory.h>
#include <ef/str.h>
#include <ef/err.h>

guiText_s* gui_text_new(ftFonts_s* font, g2dColor_t foreground, g2dColor_t colCursor, unsigned tabspace, unsigned flags){
	guiText_s* txt = mem_new(guiText_s);
	if( !txt ) return NULL;

	txt->render = NULL;
	txt->scroll.x = 0;
	txt->scroll.y = 0;
	txt->cursor.x = 0;
	txt->cursor.y = 0;
	txt->cursor.h = ft_line_height(font);
	ftRender_s* glyph = ft_fonts_glyph_load(font, ' ', FT_RENDER_VALID | FT_RENDER_ANTIALIASED);
	iassert(glyph);
	txt->cursor.w = glyph->img->w;
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

size_t gui_text_line_left_width(guiText_s* txt){
	utf8Iterator_s it = txt->it;
	size_t width = 0;
	utf_t utf;
	while( (utf=utf8_iterator_prev(&it)) && utf != '\n' ) ++width;
	return width;
}

void gui_text_put(__unused gui_s* gui, guiText_s* txt, utf_t utf){
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

	txt->flags |= GUI_TEXT_REND_TEXT | GUI_TEXT_REND_CURSPOS;

	/*
	if( txt->flags & GUI_TEXT_SCROLL_X ){
		if( utf == '\n' ){
			txt->scroll.x = 0;
			txt->cursorOffsetX = gui_text_line_left_width(txt);
			++txt->cursorOffsetY;
		}
		else{
			++txt->scroll.x
			//if( (txt->cursor.x - txt->scroll.x) + txt->glyphWidth >= gui->surface->img->w ){
			//	txt->scroll.x += txt->glyphWidth;
			//}
			//
			//
		}
	}
	
	if( txt->flags & GUI_TEXT_SCROLL_Y && (txt->cursor.y - txt->scroll.y) + txt->glyphWidth > gui->surface->img->h ){
		txt->scroll.y += txt->glyphHeight;
	}
	*/
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
	txt->flags |= GUI_TEXT_REND_TEXT | GUI_TEXT_REND_CURSPOS;
}

void gui_text_backspace(guiText_s* txt){
	if( utf8_iterator_prev(&txt->it) ){
		txt->flags |= GUI_TEXT_REND_TEXT | GUI_TEXT_REND_CURSPOS;
		utf8_iterator_delete(&txt->it);
	}
}

/*
__private void dbg_cursor(termReadLine_s* rl){
	fprintf(stderr, "%s\n", rl->it.begin);
	utf8Iterator_s c = utf8_iterator(rl->it.begin, 0);
	
	while( c.str != rl->it.str ){
		utf8_iterator_next(&c);
		fprintf(stderr, " ");
	}
	fprintf(stderr,"^\n");
}
*/

void gui_text_cursor_next(guiText_s* txt){
	utf_t u;
	while( (u=utf8_iterator_next(&txt->it)) >= UTF_PRIVATE0_START );
	//dbg_cursor(rl);
}

void gui_text_cursor_prev(guiText_s* txt){
	utf_t utf;
	while( (utf=utf8_iterator_prev(&txt->it)) >= UTF_PRIVATE0_START );
	//dbg_cursor(rl);
}
/*
void gui_text_cursor_end(gui_s* gui, guiText_s* txt){
	while( *txt->it.str && *txt->it.str != '\n' ){
		gui_text_cursor_next(gui, txt);
	}
}

void gui_text_cursor_home(guiText_s* txt){
	utf_t utf;
	while( (utf=utf8_iterator_prev(&txt->it)) && utf != '\n' );
	if( utf ) utf8_iterator_next(&txt->it);
	txt->scroll.x = 0;
}

void gui_text_cursor_scroll_left(guiText_s* txt){
	if( txt->flags & GUI_TEXT_SCROLL_X && txt->scroll.x > txt->glyphWidth ) txt->scroll.x -= txt->glyphWidth;
}

void gui_text_cursor_scroll_right(guiText_s* txt){
	//TODO how check max scroll rght?
	if( txt->flags & GUI_TEXT_SCROLL_X ) txt->scroll.x += txt->glyphWidth
}



void gui_text_cursor_up(gui_s* gui, guiText_s* txt){
	const size_t col = gui_text_line_left_width(txt);
   	const size_t wid = gui->surface->img->w;
	utf_t utf;
	while( wid > 0 && (utf=utf8_iterator_prev(&txt->it)) ){
		if( utf >= UTF_PRIVATE0_START ) continue;
		if( utf == '\n' ){
			size_t curcol = gui_text_line_left_width(txt);
			if( curcol > col ){
				curcol -= col;
				while( curcol-->0 && (utf=utf8_iterator_prev(&txt->it) && utf != '\n') );
				break;
		}
	}
	const size_t rweight = ft_line_lenght_rev(txt->fonts, txt->text, txt->it.str);

	if( rl->cursor.scrollrow > 0 && (int)rl->cursor.row - rl->position.row <= 0 ){
		--rl->cursor.scrollrow;
	}	
}

void term_readline_cursor_down(termReadLine_s* rl){
	int col = (int)term_readline_line_left_width(rl) - (int)rl->cursor.scrollcol;
	int wid = rl->position.width;
	utf_t utf;
	while( wid > 0 && (utf=utf8_iterator_next(&rl->it)) ){
		if( utf >= TERM_READLINE_PRIVATE_UTF ) continue;
		if( utf == '\n' ){
			int cl = (int)term_readline_line_left_width(rl) - (int)rl->cursor.scrollcol;
			wid = col - cl;
		}
		else{
			--wid;
		}
	}
	if( (int)rl->cursor.row - rl->position.row + 1 >= (int)rl->position.height ){
		++rl->cursor.scrollrow;
	}
}

void term_readline_cursor_pagdn(termReadLine_s* rl){
	unsigned hei = rl->position.height;
	while( hei-->0 ){
		term_readline_cursor_down(rl);
	}
}

void term_readline_cursor_pagup(termReadLine_s* rl){
	unsigned hei = rl->position.height;
	while( hei-->0 ){
		term_readline_cursor_up(rl);
	}
}
*/
void gui_text_render_cursor(gui_s* gui, guiText_s* txt){
	if( !(txt->flags & GUI_TEXT_CUR_VISIBLE) ) return;
	if( !(txt->flags & GUI_TEXT_REND_CURON) ) return;

	g2dCoord_s cursor = txt->cursor;
	cursor.w = GUI_TEXT_CURSOR_LIGHT_VALUE;
	cursor.x = cursor.x > cursor.w+1 ? cursor.x - cursor.w + 1 : 0;
	iassert((int)cursor.y - (int)txt->scroll.y >= 0);
	cursor.y -= txt->scroll.y;
	iassert((int)cursor.x - (int)txt->scroll.x >= 0);
	cursor.x -= txt->scroll.x;

	dbg_info("relative x %u y %u w %u", cursor.x, cursor.y, cursor.w);

	g2dPoint_s lineST = { .x = cursor.x, .y = cursor.y };
	g2dPoint_s lineEN = { .x = cursor.x, .y = cursor.y + cursor.h };
	iassert( lineST.x < gui->surface->img->w);
	iassert( lineST.y < gui->surface->img->h);
	iassert( lineEN.x < gui->surface->img->w);
	iassert( lineEN.y < gui->surface->img->h);

	for( unsigned i = 0; i < cursor.w; ++i){
		g2d_line(gui->surface->img, &lineST, &lineEN, txt->colCursor, 1);
		++lineST.x;
		++lineEN.x;
	}
}

void gui_text_render_text(gui_s* gui, guiText_s* txt, int partial){
	const int scrollXEnable = txt->flags & GUI_TEXT_SCROLL_X;
	const int scrollYEnable = txt->flags & GUI_TEXT_SCROLL_Y;
	const unsigned w = gui->surface->img->w;
	const unsigned h = gui->surface->img->h;
	const g2dColor_t colClear = gui_color(0,0,0,0);
	g2dCoord_s newCursor = txt->cursor;
	g2dCoord_s cursor = txt->cursor;
	cursor.x = 0;
	cursor.y = 0;

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

	if( !txt->render || txt->render->w < co.w || txt->render->h < co.h ){
		co.w += txt->cursor.w;
		co.h += txt->cursor.h;
		dbg_info("resize render %u*%u", co.w, co.h);
		if( txt->render ) g2d_free(txt->render);
		txt->render = g2d_new(co.w, co.h, -1);
		partial = 0;
	}
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
			cursor.x = 0;
			if( scrollYEnable ){
				cursor.y += txt->cursor.h;
				continue;
			}
			if( cursor.y + txt->cursor.h > h ){
				break;
			}
		}
		if( u == '\t' ){
			if( !scrollXEnable && cursor.x + txt->cursor.w * txt->tabspace > w ){
				if( scrollYEnable ){
					cursor.y += txt->cursor.h;
					continue;
				}
				if( cursor.y + txt->cursor.h > h ){
					break;
				}
				cursor.x = 0;
				continue;
			}
			cursor.x += txt->cursor.w * txt->tabspace;
			continue;
		}
		
		ftRender_s* rch = ft_fonts_glyph_load(txt->fonts, u, FT_RENDER_ANTIALIASED | FT_RENDER_VALID);
		if( !rch ){
			dbg_warning("invalid render glyph");
			continue;
		}

		if( !scrollXEnable && (cursor.x + rch->horiAdvance > w) ){
			if( scrollYEnable ){
				cursor.y += txt->cursor.h;
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
	
	dbg_error("FIND SCROLLING %u+%u", txt->scroll.x, txt->scroll.y);
	//find scrolling
	if( txt->scroll.x && newCursor.x < txt->scroll.x ){
		dbg_info("scroll.x && cursor.x(%u) < scroll.x(%u):: scroll.x(%u)", newCursor.x, txt->scroll.x, newCursor.x);
		txt->scroll.x = newCursor.x;
	}
	else if( newCursor.x - txt->scroll.x > w ){
		if( scrollXEnable ){
			dbg_info("scrollEnable && cursor.x(%u) - scroll.x(%u) > w(%u):: scroll.x(%u)", newCursor.x, txt->scroll.x, w, w-newCursor.x);
			txt->scroll.x = w - newCursor.x;
		}
		else{
			dbg_info("!scrollEnable && cursor.x(%u) - scroll.x(%u) > w(%u):: scroll.x(%u) newCursor.x(%u)", newCursor.x, txt->scroll.x, w, 0, newCursor.x);
			txt->scroll.x = 0;
			newCursor.x = w;
		}
	}
	if( txt->scroll.y && newCursor.y < txt->scroll.y ){
		txt->scroll.y = newCursor.y;
	}
	else if( newCursor.y - txt->scroll.y > h ){
		if( scrollYEnable ){
			txt->scroll.y = h - newCursor.y;
			txt->scroll.y += cursor.h;
		}
		else{
			txt->scroll.y = 0;
			newCursor.y = h - cursor.h;
		}
	}
	txt->cursor = newCursor;
	dbg_info("calcolate scrolling %u+%u", txt->scroll.x, txt->scroll.y);

}

void gui_text_redraw(gui_s* gui, guiBackground_s* bkg, guiText_s* txt, int partial){
	dbg_info("clear background");
	gui_background_redraw(gui, bkg);
	
	dbg_info("render text");
	gui_text_render_text(gui, txt, partial);

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
	dbg_info("apply text");
	g2d_bitblt_alpha(gui->surface->img, &pdest, txt->render, &psrc);

	dbg_info("render cursor");
	gui_text_render_cursor(gui, txt);
	dbg_info("render ok");
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
		break;

		case XKB_KEY_Down:
		break;

		case XKB_KEY_Page_Up:
		break;

		case XKB_KEY_Page_Down:
		break;

		case XKB_KEY_Home:
		case XKB_KEY_Begin:
		break;

		case XKB_KEY_End:
		break;

		case XKB_KEY_Insert:
		break;

		case XKB_KEY_Return:
			gui_text_put(gui, gui->control, '\n');
		break;

		case XKB_KEY_Tab:
			gui_text_put(gui, gui->control, '\t');
		break;

		default:
			gui_text_put(gui, gui->control, event->keyboard.utf);
		break;
	}

	gui_text_redraw(gui, gui->background[0], gui->control, partial);
	gui_draw(gui);
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

/*
int gui_label_event_redraw(gui_s* gui, __unused xorgEvent_s* unset){
	iassert(gui->type == GUI_TYPE_LABEL);
	gui_label_redraw(gui, gui->background[0], gui->control);
	return 0;
}
*/


