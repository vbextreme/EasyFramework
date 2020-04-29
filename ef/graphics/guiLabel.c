#include <ef/guiLabel.h>
#include <ef/memory.h>
#include <ef/str.h>
#include <ef/ft.h>
#include <ef/err.h>

guiLabel_s* gui_label_new(guiCaption_s* caption){
	if( !caption ) return NULL;
	guiLabel_s* lbl = mem_new(guiLabel_s);
	if( !lbl ) err_fail("eom");
	lbl->caption = caption;
	return lbl;
}

gui_s* gui_label_attach(gui_s* gui, guiLabel_s* lbl){
	if( !gui ) goto ERR;
	if( !lbl ) goto ERR;
	gui->control = lbl;
	gui->type = GUI_TYPE_LABEL;
	gui->redraw = gui_label_event_redraw;
	//gui->themes = gui_label_event_themes;
	gui->free = gui_label_event_free;
	gui->focusable = 0;
	gui_composite_add(gui->img, lbl->caption->render);
	return gui;
ERR:
	if( lbl ) gui_label_free(lbl);
	if( gui ) gui_free(gui);
	return NULL;
}

void gui_label_free(guiLabel_s* lbl){
	gui_caption_free(lbl->caption);
	free(lbl);
}

void gui_label_text_set(gui_s* gui, const utf8_t* text){
	iassert(gui->type == GUI_TYPE_LABEL);
	guiLabel_s* lbl = gui->control;
	gui_caption_text_set(gui, lbl->caption, text);
}

void gui_label_redraw(gui_s* gui){
	iassert(gui->type == GUI_TYPE_LABEL);
	guiLabel_s* lbl = gui->control;
	gui_caption_render(gui, lbl->caption);
	gui_composite_redraw(gui, gui->img);
}

void gui_label_scroll(gui_s* gui, unsigned x, unsigned y){
	iassert(gui->type == GUI_TYPE_LABEL);
	guiLabel_s* lbl = gui->control;
	gui_caption_scroll(gui, lbl->caption, x, y);
}

int gui_label_event_free(gui_s* gui, __unused xorgEvent_s* ev){
	iassert(gui->type == GUI_TYPE_LABEL);
	gui_label_free(gui->control);
	return 0;
}

int gui_label_event_redraw(gui_s* gui, __unused xorgEvent_s* unset){
	iassert(gui->type == GUI_TYPE_LABEL);
	gui_label_redraw(gui);
	return 0;
}

/*
int gui_label_event_themes(gui_s* gui, xorgEvent_s* ev){
	guiLabel_s* lbl = ev->data.request;
	char* name = ev->data.data;
	
	int vbool = 0;
	if( !gui_themes_bool_set(name, GUI_THEME_CAPTION_CENTER_X, &vbool) ){
		if( vbool ) lbl->flags |= GUI_LABEL_CENTER_X;
		else lbl->flags &= ~GUI_LABEL_CENTER_Y;
		lbl->flags |= GUI_LABEL_RENDERING;
	}

	if( !gui_themes_bool_set(name, GUI_THEME_CAPTION_CENTER_Y, &vbool) ){
		if( vbool ) lbl->flags |= GUI_LABEL_CENTER_Y;
		else lbl->flags &= ~GUI_LABEL_CENTER_Y;
		lbl->flags |= GUI_LABEL_RENDERING;
	}

	if( !gui_themes_uint_set(name, GUI_THEME_FOREGROUND, &lbl->foreground) ) lbl->flags |= GUI_LABEL_RENDERING;

	if( gui_themes_int_set(name, GUI_THEME_CAPTION_AUTOWRAP, &lbl->autowrap) ) lbl->flags |= GUI_LABEL_RENDERING;

	gui_themes_font_set(name, &lbl->fonts);

	char* caption = gui_themes_string(name, GUI_THEME_CAPTION);
	if( caption ) gui_label_text_set(gui, lbl, (utf8_t*)caption);

	return 0;
}
*/
