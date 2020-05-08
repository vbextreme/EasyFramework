#include <ef/guiButton.h>
#include <ef/memory.h>
#include <ef/vector.h>
#include <ef/str.h>
#include <ef/err.h>

guiButton_s* gui_button_new(guiCaption_s* caption, guiImage_s* press, guiImage_s* hover, guiEvent_f onclick, int flags){
	if( !caption ) return NULL;
	guiButton_s* btn = mem_new(guiButton_s);
	if( !btn ) err_fail("eom");
	btn->caption = caption;
	btn->state[GUI_BUTTON_STATE_NORMAL] = NULL;
	btn->state[GUI_BUTTON_STATE_PRESS] = press;
	btn->state[GUI_BUTTON_STATE_HOVER] = hover;
	btn->onclick = onclick;
	btn->flags = flags;
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
	gui->themes = gui_button_event_themes;
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
		if( !gui_focus_have(gui) ){
			gui_focus(gui);
		}
		gui_button_redraw(gui, 0);
		gui_draw(gui);
		guiButton_s* btn = gui->control;
		if( btn->onclick ) btn->onclick(gui, event);
	}
	else if( event->mouse.event == XORG_MOUSE_ENTER ){
		guiButton_s* btn = gui->control;
		if( btn->flags & GUI_BUTTON_FLAGS_HOVER ){
			gui_button_redraw(gui, 2);
			gui_draw(gui);
		}
	}
	else if( event->mouse.event == XORG_MOUSE_LEAVE ){
		guiButton_s* btn = gui->control;
		if( btn->flags & GUI_BUTTON_FLAGS_HOVER ){
			gui_button_redraw(gui, 0);
			gui_draw(gui);
		}
	}
	return 0;
}

int gui_button_event_move(gui_s* gui, xorgEvent_s* event){
	iassert(gui->type == GUI_TYPE_BUTTON);
	guiButton_s* btn = gui->control;
	dbg_info("button.move:%s %u*%u", gui->name, event->move.w, event->move.h);
	if( gui->img->img[btn->compindex] != btn->state[0] ){
		gui_image_resize(gui, btn->state[0], event->move.w, event->move.h , -1);
	}
	if( gui->img->img[btn->compindex] != btn->state[1] ){
		gui_image_resize(gui, btn->state[1], event->move.w, event->move.h , -1);
	}
	if( gui->img->img[btn->compindex] != btn->state[2] ){
		gui_image_resize(gui, btn->state[2], event->move.w, event->move.h , -1);
	}
	btn->caption->flags |= GUI_CAPTION_RENDERING;
	gui_event_move(gui, event);
	return 0;
}

int gui_button_event_themes(gui_s* gui, xorgEvent_s* ev){
	iassert(gui->type == GUI_TYPE_BUTTON);
	guiButton_s* btn = gui->control;

	char* name = ev->data.data;
	gui_caption_themes(gui, btn->caption, name);

	int vbool;
	if( gui_themes_bool_set(name, GUI_THEMES_BUTTON_HOVER, &vbool) ){
		if( vbool ) btn->flags |= GUI_BUTTON_FLAGS_HOVER;
		else        btn->flags &= ~GUI_BUTTON_FLAGS_HOVER;
	}

	char* iname = str_printf("%s.%s", name, GUI_THEME_BUTTON_PRESS);
	gui_themes_gui_image(gui, iname, &btn->state[GUI_BUTTON_STATE_PRESS]);
	free(iname);

	iname = str_printf("%s.%s", name, GUI_THEME_BUTTON_HOVER);
	gui_themes_gui_image(gui, iname, &btn->state[GUI_BUTTON_STATE_HOVER]);
	free(iname);

	return 0;
}

