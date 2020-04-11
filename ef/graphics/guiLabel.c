#include <ef/guiLabel.h>
#include <ef/memory.h>
#include <ef/str.h>
#include <ef/ft.h>
#include <ef/err.h>

guiLabel_s* label_new(ftFonts_s* font, int autowrap, g2dColor_t foreground){
	guiLabel_s* lbl = mem_new(guiLabel_s);
	lbl->text = NULL;
	lbl->autowrap = autowrap;
	lbl->fonts = font;
	lbl->foreground = foreground;
	return lbl;
}

void label_free(guiLabel_s* lbl){
	if( lbl->text ) free(lbl->text);
	free(lbl);
}

int gui_label_event_free(gui_s* gui, __unused xorgEvent_s* ev){
	iassert(gui->type == GUI_TYPE_LABEL);
	label_free(gui->control);
	return 0;
}

gui_s* gui_label_attach(gui_s* gui, guiLabel_s* lbl){
	if( !gui ) goto ERR;
	if( !lbl ) goto ERR;
	gui->control = lbl;
	gui->type = GUI_TYPE_LABEL;
	gui->redraw = gui_label_event_redraw;
	gui->free = gui_label_event_free;
	return gui;
ERR:
	if( lbl ) label_free(lbl);
	if( gui ) gui_free(gui);
	return NULL;
}

void gui_label_text_set(gui_s* gui, const utf8_t* text){
	iassert(gui->type == GUI_TYPE_LABEL);
	guiLabel_s* lbl = gui->control;

	if( lbl->text ) free(lbl->text);
	lbl->text = (utf8_t*)str_dup((const char*)text, 0);
	
	if( lbl->autowrap ){		
		lbl->position.y = 0;
		lbl->weight = gui->position.w;
		lbl->position.x = 0;
	}
	else{
		const size_t lh = ft_line_height(lbl->fonts);
		if( lh >= gui->position.h ) lbl->position.y = 0;
		else lbl->position.y = (gui->position.h - lh) / 2;

		lbl->weight	= ft_line_lenght(lbl->fonts, text);
		if( lbl->weight > gui->position.w )	lbl->position.x = 0;
		else lbl->position.x = (gui->position.w - lbl->weight) / 2;
	}

	lbl->position.h = gui->position.h;
	lbl->position.w = gui->position.w;

	dbg_info("label %u*%u", lbl->position.w, lbl->position.h);

	gui->redraw(gui, NULL);
}

void gui_label_position_set(gui_s* gui, unsigned x, unsigned y){
	iassert(gui->type == GUI_TYPE_LABEL);
	guiLabel_s* lbl = gui->control;
	lbl->position.x = x;
	lbl->position.y = y;
}

int gui_label_event_redraw(gui_s* gui, __unused xorgEvent_s* unset){
	iassert(gui->type == GUI_TYPE_LABEL);
	guiLabel_s* lbl = gui->control;

	gui_event_redraw(gui, NULL);

	g2dCoord_s pos = lbl->position;
	if( lbl->autowrap ){
		g2d_string_autowrap(gui->surface->img, &pos, lbl->fonts, lbl->text, lbl->foreground, lbl->position.x);
	}
	else{
		g2d_string(gui->surface->img, &pos, lbl->fonts, lbl->text, lbl->foreground, lbl->position.x);
	}

	return 0;
}


