#include <ef/guiLabel.h>
#include <ef/memory.h>
#include <ef/str.h>
#include <ef/ft.h>
#include <ef/err.h>

guiLabel_s* gui_label_new(ftFonts_s* font, int autowrap, g2dColor_t foreground, unsigned flags){
	guiLabel_s* lbl = mem_new(guiLabel_s);
	if( !lbl ) return NULL;
	lbl->text = NULL;
	lbl->autowrap = autowrap;
	lbl->fonts = font;
	lbl->foreground = foreground;
	lbl->render = NULL;
	lbl->flags = flags;
	lbl->scroll.x = 0;
	lbl->scroll.y = 0;

	return lbl;
}

gui_s* gui_label_attach(gui_s* gui, guiLabel_s* lbl){
	if( !gui ) goto ERR;
	if( !lbl ) goto ERR;
	gui->control = lbl;
	gui->type = GUI_TYPE_LABEL;
	gui->redraw = gui_label_event_redraw;
	gui->free = gui_label_event_free;
	gui->focusable = 0;
	return gui;
ERR:
	if( lbl ) gui_label_free(lbl);
	if( gui ) gui_free(gui);
	return NULL;
}

void gui_label_free(guiLabel_s* lbl){
	if( lbl->text ) free(lbl->text);
	if( lbl->render ) free(lbl->render);
	free(lbl);
}

void gui_label_text_set(__unused gui_s* gui, guiLabel_s* lbl, const utf8_t* text){
	iassert(gui && lbl);
	if( lbl->text ) free(lbl->text);
	lbl->text = (utf8_t*)str_dup((const char*)text, 0);	
	lbl->flags |= GUI_LABEL_RENDERING;
}

__private void label_render(guiLabel_s* lbl, unsigned w, unsigned h){
	lbl->flags &= ~GUI_LABEL_RENDERING;
//TODO g2d_char_indirect no fusion alpha
	if( lbl->autowrap ){
		h = ft_autowrap_height(lbl->fonts, lbl->text, w);
		if( lbl->render ){
			if( lbl->render->w != w || lbl->render->h != h ){
				dbg_info("autowrap resize render %u*%u", w,h);
				g2d_free(lbl->render);
				lbl->render = g2d_new(w, h, -1);
			}
			else{
				dbg_info("autowrap reuse render");
			}
		}
		else{
			dbg_info("autowrap new render %u*%u", w,h);
			lbl->render = g2d_new(w, h, -1);
		}
		g2dCoord_s cr = { .x = 0, .y = 0, .w = w, .h = h };
		g2d_clear(lbl->render, gui_color( 0, 0, 0, 0), &cr);
		g2d_string_autowrap(lbl->render, &cr, lbl->fonts, lbl->text, lbl->foreground, cr.x, 1);
	}
	else{
		h = ft_multiline_height(lbl->fonts, lbl->text);
		w = ft_multiline_lenght(lbl->fonts, lbl->text);
		if( lbl->render ){
			if( lbl->render->w != w || lbl->render->h != h ){
				dbg_info("resize render %u*%u", w,h);
				g2d_free(lbl->render);
				lbl->render = g2d_new(w, h, -1);
			}
			else{
				dbg_info("reuse render");
			}
		}
		else{
			dbg_info("new render %u*%u", w,h);
			lbl->render = g2d_new(w, h, -1);
		}
		g2dCoord_s cr = { .x = 0, .y = 0, .w = w, .h = h }; 
		g2d_clear(lbl->render, gui_color( 0, 255, 255, 255), &cr);
		const utf8_t* txt = lbl->text;
		while( (txt=g2d_string(lbl->render, &cr, lbl->fonts, lbl->text, lbl->foreground, cr.x,1)) );
	}	
}

void gui_label_redraw(gui_s* gui, guiBackground_s* bkg, guiLabel_s* lbl){
	gui_background_redraw(gui, bkg);
	if( !lbl->text ) return;
	if( lbl->flags & GUI_LABEL_RENDERING ) label_render(lbl, gui->surface->img->w, gui->surface->img->h);
	unsigned x = 0;
	unsigned y = 0;
	if( lbl->flags & GUI_LABEL_CENTER_X ){
		if( lbl->render->w - lbl->scroll.x < gui->surface->img->w ){
			x = lbl->render->w - lbl->scroll.x;
			x = (gui->surface->img->w - x) / 2;
		}
	}
	if( lbl->flags & GUI_LABEL_CENTER_Y ){
		if( lbl->render->h - lbl->scroll.y < gui->surface->img->h ){
			y = lbl->render->h - lbl->scroll.y;
			y = (gui->surface->img->h - y) / 2;
		}
	}

	g2dCoord_s di;
	di.x = x,
	di.y = y;
	di.w = gui->surface->img->w - x;
	if( di.w > lbl->render->w ) di.w = lbl->render->w;
	di.h = gui->surface->img->h - y;
	if( di.h > lbl->render->h ) di.h = lbl->render->h;
	g2dCoord_s ri = { .x = lbl->scroll.x, .y = lbl->scroll.y, .w = di.w, .h = di.h }; 

	dbg_info("bitblt %u %u %u*%u -> %u %u %u*%u", ri.x, ri.y, ri.w, ri.h, di.x, di.y, di.w, di.h);

	g2d_bitblt_alpha(gui->surface->img, &di, lbl->render, &ri);
}

void gui_label_scroll(gui_s* gui, guiLabel_s* lbl, unsigned x, unsigned y){
	if( x + gui->surface->img->w > lbl->render->w ){
		if( lbl->render->w > gui->surface->img->w ){
			x = gui->surface->img->w - lbl->render->w;
		}
		else{
			x = 0;
		}
	}
	if( y + gui->surface->img->h > lbl->render->h ){
		if( lbl->render->h > gui->surface->img->h ){
			y = gui->surface->img->h - lbl->render->h;
		}
		else{
			y = 0;
		}

	}

	lbl->scroll.x = x;
	lbl->scroll.y = y;
}

int gui_label_event_free(gui_s* gui, __unused xorgEvent_s* ev){
	iassert(gui->type == GUI_TYPE_LABEL);
	gui_label_free(gui->control);
	return 0;
}

int gui_label_event_redraw(gui_s* gui, __unused xorgEvent_s* unset){
	iassert(gui->type == GUI_TYPE_LABEL);
	gui_label_redraw(gui, gui->background[0], gui->control);
	return 0;
}


