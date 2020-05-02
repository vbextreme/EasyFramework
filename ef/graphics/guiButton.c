#include <ef/guiButton.h>
#include <ef/memory.h>
#include <ef/vector.h>
#include <ef/str.h>
#include <ef/err.h>

guiButton_s* gui_button_new(guiCaption_s* caption, guiImage_s* press, guiImage_s* hover, guiEvent_f onclick){
	if( !caption ) return NULL;
	guiButton_s* btn = mem_new(guiButton_s);
	if( !btn ) err_fail("eom");
	btn->caption = caption;
	btn->state[GUI_BUTTON_STATE_NORMAL] = NULL;
	btn->state[GUI_BUTTON_STATE_PRESS] = press;
	btn->state[GUI_BUTTON_STATE_HOVER] = hover;
	btn->onclick = onclick;
	return btn;
}

gui_s* gui_button_attach(gui_s* gui, guiButton_s* btn){
	if( !gui ) goto ERR;
	if( !btn ) goto ERR;
	btn->parentKey = gui->key;
	gui->control = btn;
	gui->type = GUI_TYPE_BUTTON;
	gui->redraw = gui_button_event_redraw;
	gui->mouse = gui_button_event_mouse;
	gui->key = gui_button_event_key;
	gui->free = gui_button_event_free;	
	gui->move = gui_button_event_move;
	btn->compindex = vector_count(gui->img->img)-1;
	btn->state[GUI_BUTTON_STATE_NORMAL] = gui->img->img[btn->compindex];
	gui_composite_add(gui->img, btn->caption->render);
	return gui;
ERR:
	dbg_error("an error occur");
	if( btn ) gui_button_free(btn);
	if( gui ) gui_free(gui);
	return NULL;
}

void gui_button_free(guiButton_s* btn){
	gui_caption_free(btn->caption);
	gui_image_free(btn->state[1]);
	gui_image_free(btn->state[2]);
	free(btn);
}

void gui_button_text_set(gui_s* gui, const utf8_t* text){
	iassert(gui->type == GUI_TYPE_BUTTON);
	guiButton_s* btn = gui->control;
	gui_caption_text_set(gui, btn->caption, text);
}

void gui_button_redraw(gui_s* gui, unsigned normalPressHover){
	iassert(gui->type == GUI_TYPE_BUTTON);
	guiButton_s* btn = gui->control;
	gui_caption_render(gui, btn->caption);
	if( normalPressHover < GUI_BUTTON_STATE_COUNT){
		gui->img->img[btn->compindex] = btn->state[normalPressHover];
	}
	gui_composite_redraw(gui, gui->img);
}

int gui_button_event_free(gui_s* gui, __unused xorgEvent_s* ev){
	iassert(gui->type == GUI_TYPE_BUTTON);
	guiButton_s* btn = gui->control;
	gui->img->img[btn->compindex] = btn->state[GUI_BUTTON_STATE_NORMAL];
	gui_button_free(gui->control);
	return 0;
}

int gui_button_event_redraw(gui_s* gui, __unused xorgEvent_s* unset){
	iassert(gui->type == GUI_TYPE_BUTTON);
	gui_button_redraw(gui, 0);
	return 0;
}

int gui_button_event_key(gui_s* gui, xorgEvent_s* event){
	iassert(gui->type == GUI_TYPE_BUTTON);

	if( event->keyboard.event == XORG_KEY_PRESS && event->keyboard.keysym == XKB_KEY_Return ){
		gui_button_redraw(gui, 1);
		gui_draw(gui);
		return 0;
	}
	else if( event->keyboard.event == XORG_KEY_RELEASE && event->keyboard.keysym == XKB_KEY_Return ){
		gui_button_redraw(gui, 0);
		gui_draw(gui);
		guiButton_s* btn = gui->control;
		if( btn->onclick ) btn->onclick(gui, event);
		return 0;
	}

	guiButton_s* btn = gui->control;
	if( btn->parentKey && gui->parent ) btn->parentKey(gui, event);

	return 0;
}

int gui_button_event_mouse(gui_s* gui, xorgEvent_s* event){
	iassert(gui->type == GUI_TYPE_BUTTON);

	if( event->mouse.event == XORG_MOUSE_PRESS && event->mouse.button == 1 ){
		gui_button_redraw(gui, 1);
		gui_draw(gui);
		return 0;
	}
	else if( (event->mouse.event == XORG_MOUSE_RELEASE || event->mouse.event == XORG_MOUSE_CLICK || event->mouse.event == XORG_MOUSE_DBLCLICK) 
			&& event->mouse.button == 1 )
	{
		if( !gui_focuse_have(gui) ){
			gui_focus(gui);
		}
		gui_button_redraw(gui, 0);
		gui_draw(gui);
		guiButton_s* btn = gui->control;
		if( btn->onclick ) btn->onclick(gui, event);
	}
	else if( event->mouse.event == XORG_MOUSE_ENTER ){
		gui_button_redraw(gui, 2);
		gui_draw(gui);
	}
	else if( event->mouse.event == XORG_MOUSE_LEAVE ){
		gui_button_redraw(gui, 0);
		gui_draw(gui);
	}
	return 0;
}

int gui_button_event_move(gui_s* gui, xorgEvent_s* event){
	//TODO this not works with image
	iassert(gui->type == GUI_TYPE_BUTTON);
	guiButton_s* btn = gui->control;
	gui_event_move(gui, event);
	btn->state[1]->pos.w = gui->surface->img->w;
	btn->state[1]->pos.h = gui->surface->img->h;
	btn->state[2]->pos.w = gui->surface->img->w;
	btn->state[2]->pos.h = gui->surface->img->h;
	return 0;
}

/*
int gui_button_event_themes(gui_s* gui, xorgEvent_s* ev){
	guiButton_s* btn = ev->data.request;
	ev->data.request = btn->label;
	gui_label_event_themes(gui, ev);

	__mem_free char* bkpress = str_printf("%s%s.", (char*)ev->data.data, "press");
	__mem_free char* bkhover = str_printf("%s%s.", (char*)ev->data.data, "hover");

	gui_themes_background(gui, bkpress, gui->background[GUI_BUTTON_BACKGROUND_PRESS]);
	gui_themes_background(gui, bkhover, gui->background[GUI_BUTTON_BACKGROUND_HOVER]);

	return 0;
}
*/
