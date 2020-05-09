#include <ef/guiOptionBox.h>
#include <ef/memory.h>
#include <ef/vector.h>
#include <ef/str.h>
#include <ef/err.h>

guiOption_s* gui_option_new(guiCaption_s* caption, guiImage_s* off, guiImage_s* on, guiImage_s* hover, int flags){
	if( !caption ) return NULL;
	guiOption_s* opt = mem_new(guiOption_s);
	if( !opt ) err_fail("eom");
	opt->caption = caption;
	opt->state[GUI_OPTION_STATE_Z] = NULL;
	opt->state[GUI_OPTION_STATE_OFF] = off;
	opt->state[GUI_OPTION_STATE_ON] = on;
	opt->state[GUI_OPTION_STATE_HOVER]  = hover;
	opt->flags = flags;
	return opt;
}

gui_s* gui_option_attach(gui_s* gui, guiOption_s* opt){
	if( !gui ) goto ERR;
	if( !opt ) goto ERR;
	opt->parentKey = gui->key;
	gui->control = opt;
	gui->type = GUI_TYPE_OPTION;
	gui->redraw = gui_option_event_redraw;
	gui->mouse = gui_option_event_mouse;
	gui->key = gui_option_event_key;
	gui->free = gui_option_event_free;	
	gui->move = gui_option_event_move;
	gui->themes = gui_option_event_themes;
	opt->zindex = vector_count(gui->img->img)-1;
	opt->state[GUI_OPTION_STATE_Z] = gui->img->img[opt->zindex];
	gui_composite_add(gui->img, opt->caption->render);
	gui_composite_add(gui->img, opt->state[GUI_OPTION_STATE_OFF]);
	opt->vindex = vector_count(gui->img->img)-1;
	return gui;
ERR:
	dbg_error("an error occur");
	if( opt ) gui_option_free(opt);
	if( gui ) gui_free(gui);
	return NULL;
}

void gui_option_free(guiOption_s* opt){
	gui_caption_free(opt->caption);
	if( opt->flags & GUI_OPTION_FLAGS_ACTIVE ){
		gui_image_free(opt->state[GUI_OPTION_STATE_OFF]);
	}
	else{
		gui_image_free(opt->state[GUI_OPTION_STATE_ON]);
	}
	if( opt->flags & GUI_OPTION_FLAGS_HOVER ){
		gui_image_free(opt->state[GUI_OPTION_STATE_Z]);
	}
	else{
		gui_image_free(opt->state[GUI_OPTION_STATE_HOVER]);
	}
	free(opt);
}

void gui_option_text_set(gui_s* gui, const utf8_t* text){
	iassert(gui->type == GUI_TYPE_OPTION);
	guiOption_s* opt = gui->control;
	gui_caption_text_set(gui, opt->caption, text);
}

__private void option_find_and_deactivate(gui_s* parent){
	vector_foreach(parent->childs, i){
		gui_s* child = parent->childs[i];
		if( child->type != GUI_TYPE_OPTION ) continue;
		guiOption_s* chopt = child->control;
		if( chopt->flags & GUI_OPTION_FLAGS_UNIQUE && chopt->flags & GUI_OPTION_FLAGS_ACTIVE ){
			chopt->flags &= ~ GUI_OPTION_FLAGS_ACTIVE;
			gui_redraw(child);
			gui_draw(child);
		}
	}
}

void gui_option_active(gui_s* gui, int value){
	iassert(gui->type == GUI_TYPE_OPTION);
	iassert(gui->parent);

	guiOption_s* opt = gui->control;
	if( value ){
		if( opt->flags & GUI_OPTION_FLAGS_UNIQUE ) option_find_and_deactivate(gui->parent);
		opt->flags |= GUI_OPTION_FLAGS_ACTIVE;
	}
	else{
		opt->flags &= ~GUI_OPTION_FLAGS_ACTIVE;
	}
}

int gui_option_activated(gui_s* gui){
	iassert(gui->type == GUI_TYPE_OPTION);
	guiOption_s* opt = gui->control;
	return opt->flags & GUI_OPTION_FLAGS_ACTIVE;
}

void gui_option_toggle(gui_s* gui){
	if( gui_option_activated(gui) ){
		gui_option_active(gui, 0);
	}
	else{
		gui_option_active(gui, 1);
	}
}

void gui_option_redraw(gui_s* gui){
	iassert(gui->type == GUI_TYPE_OPTION);
	guiOption_s* opt = gui->control;
	gui_caption_render(gui, opt->caption);
	
	if( opt->flags & GUI_OPTION_FLAGS_HOVER ){
		gui->img->img[opt->zindex] = opt->state[GUI_OPTION_STATE_HOVER];
	}
	else{
		gui->img->img[opt->zindex] = opt->state[GUI_OPTION_STATE_Z];
	}

	if( opt->flags & GUI_OPTION_FLAGS_ACTIVE ){
		gui->img->img[opt->vindex] = opt->state[GUI_OPTION_STATE_ON];
	}
	else{
		gui->img->img[opt->vindex] = opt->state[GUI_OPTION_STATE_OFF];
	}
	
	gui_composite_redraw(gui, gui->img);
}

int gui_option_event_free(gui_s* gui, __unused xorgEvent_s* ev){
	iassert(gui->type == GUI_TYPE_OPTION);
	gui_option_free(gui->control);
	return 0;
}

int gui_option_event_redraw(gui_s* gui, __unused xorgEvent_s* unset){
	iassert(gui->type == GUI_TYPE_OPTION);
	gui_option_redraw(gui);
	return 0;
}

int gui_option_event_key(gui_s* gui, xorgEvent_s* event){
	iassert(gui->type == GUI_TYPE_OPTION);
	guiOption_s* opt = gui->control;

	if( event->keyboard.event == XORG_KEY_PRESS && event->keyboard.keysym == XKB_KEY_Return ){
		gui_option_toggle(gui);
		gui_option_redraw(gui);
		gui_draw(gui);
		return 0;
	}

	if( opt->parentKey && gui->parent ) opt->parentKey(gui, event);

	return 0;
}

int gui_option_event_mouse(gui_s* gui, xorgEvent_s* event){
	iassert(gui->type == GUI_TYPE_OPTION);
	guiOption_s* opt = gui->control;

	if( event->mouse.event == XORG_MOUSE_CLICK && event->mouse.button == 1 ){
		if( !gui_focus_have(gui) ) gui_focus(gui);
		gui_option_toggle(gui);
		gui_option_redraw(gui);
		gui_draw(gui);
	}
	else if( event->mouse.event == XORG_MOUSE_ENTER && opt->flags & GUI_OPTION_FLAGS_HOVER_ENABLE ){
		opt->flags |= GUI_OPTION_FLAGS_HOVER;
		gui_option_redraw(gui);
		gui_draw(gui);
	}
	else if( event->mouse.event == XORG_MOUSE_LEAVE && opt->flags & GUI_OPTION_FLAGS_HOVER_ENABLE ){
		opt->flags &=  ~GUI_OPTION_FLAGS_HOVER;
		gui_option_redraw(gui);
		gui_draw(gui);
	}
	return 0;
}

int gui_option_event_move(gui_s* gui, xorgEvent_s* event){
	iassert(gui->type == GUI_TYPE_OPTION);
	guiOption_s* opt = gui->control;
	opt->caption->flags |= GUI_CAPTION_RENDERING;
	for( size_t i = 0; i < GUI_OPTION_STATE_COUNT; ++i){
		if( gui->img->img[opt->zindex] != opt->state[i] && gui->img->img[opt->vindex] != opt->state[i] ){
			gui_image_resize(gui, opt->state[i], event->move.w, event->move.h , -1);
		}
	}
	gui_event_move(gui, event);
	return 0;
}

int gui_option_event_themes(gui_s* gui, xorgEvent_s* ev){
	iassert(gui->type == GUI_TYPE_OPTION);
	guiOption_s* opt = gui->control;

	char* name = ev->data.data;
	gui_caption_themes(gui, opt->caption, name);

	int vbool;
	if( gui_themes_bool_set(name, GUI_THEMES_OPTION_HOVER, &vbool) ){
		if( vbool ) opt->flags |= GUI_OPTION_FLAGS_HOVER_ENABLE;
		else        opt->flags &= ~GUI_OPTION_FLAGS_HOVER_ENABLE;
	}

	char* iname = str_printf("%s.%s", name, GUI_THEMES_OPTION_HOVER);
	gui_themes_gui_image(gui, iname, &opt->state[GUI_OPTION_STATE_HOVER]);
	free(iname);

	iname = str_printf("%s.%s", name, GUI_THEMES_OPTION_ON);
	gui_themes_gui_image(gui, iname, &opt->state[GUI_OPTION_STATE_ON]);
	free(iname);

	iname = str_printf("%s.%s", name, GUI_THEMES_OPTION_OFF);
	gui_themes_gui_image(gui, iname, &opt->state[GUI_OPTION_STATE_OFF]);
	free(iname);
	
	if( opt->flags & GUI_OPTION_FLAGS_ACTIVE ){
		gui->img->img[opt->vindex] = opt->state[GUI_OPTION_STATE_ON];
	}
	else{
		gui->img->img[opt->vindex] = opt->state[GUI_OPTION_STATE_OFF];
	}

	return 0;
}

