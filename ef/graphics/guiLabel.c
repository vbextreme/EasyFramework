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
	gui->themes = gui_label_event_themes;
	gui->free = gui_label_event_free;
	gui->move = gui_label_event_move;
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

int gui_label_event_themes(gui_s* gui, xorgEvent_s* ev){
	iassert(gui->type == GUI_TYPE_LABEL);
	guiLabel_s* lbl = gui->control;
	char* name = ev->data.data;
	gui_caption_themes(gui, lbl->caption, name);
	return 0;
}

int gui_label_event_move(gui_s* gui, xorgEvent_s* event){
	iassert(gui->type == GUI_TYPE_LABEL);
	guiLabel_s* lbl = gui->control;
	gui_event_move(gui, event);
	lbl->caption->flags |= GUI_CAPTION_RENDERING;
	gui_label_redraw(gui);
	gui_draw(gui);
	return 0;
}
