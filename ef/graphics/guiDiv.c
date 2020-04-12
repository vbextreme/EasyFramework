#include <ef/guiDiv.h>
#include <ef/memory.h>
#include <ef/vector.h>
#include <ef/err.h>

guiDiv_s* gui_div_new(guiDivMode_e mode){
	guiDiv_s* div = mem_new(guiDiv_s);
	if( !div ) return NULL;
	div->mode = mode;
	div->sep.x = GUI_DIV_DEFAULT_X;
	div->sep.y = GUI_DIV_DEFAULT_Y;
	return div;
}

gui_s* gui_div_attach(gui_s* gui, guiDiv_s* div){
	if( !gui ) goto ERR;
	if( !div ) goto ERR;
	gui->control = div;
	gui->type = GUI_TYPE_DIV;
	//gui->redraw = gui_button_event_redraw;
	//gui->mouse = gui_button_event_mouse;
	//gui->key = gui_button_event_key;
	//gui->focus = gui_event_focus;
	gui->free = gui_div_event_free;
	return gui;
ERR:
	if( div ) gui_div_free(div);
	if( gui ) gui_free(gui);
	return NULL;
}

void gui_div_free(guiDiv_s* div){
	free(div);
}

void gui_div_align(gui_s* gui, guiDiv_s* div){
	unsigned x = div->sep.x;
	unsigned y = div->sep.y;
	
	vector_foreach(gui->childs, i){
		
	}
}

int gui_div_event_free(gui_s* gui, __unused xorgEvent_s* ev){
	iassert(gui->type == GUI_TYPE_DIV);
	gui_div_free(gui->control);
	return 0;
}

/*
guiLabel_s* gui_button_label(guiButton_s* button){
	return button->label;
}

void gui_button_redraw(gui_s* gui, guiButton_s* btn, int press){
	if( press ){
		gui_label_redraw(gui, &btn->bkpress, btn->label);
	}
	else{
		gui_label_redraw(gui, &gui->background, btn->label);
	}
}



int gui_button_event_redraw(gui_s* gui, __unused xorgEvent_s* unset){
	iassert(gui->type == GUI_TYPE_BUTTON);
	gui_button_redraw(gui, gui->control, 0);
	return 0;
}

int gui_button_event_key(gui_s* gui, xorgEvent_s* event){
	iassert(gui->type == GUI_TYPE_BUTTON);

	if( event->keyboard.event == XORG_KEY_PRESS && event->keyboard.keysym == XKB_KEY_Return ){
		gui_button_redraw(gui, gui->control, 1);
		gui_draw(gui);
		return 0;
	}
	else if( event->keyboard.event == XORG_KEY_RELEASE && event->keyboard.keysym == XKB_KEY_Return ){
		gui_button_redraw(gui, gui->control, 0);
		gui_draw(gui);
		guiButton_s* btn = gui->control;
		if( btn->onclick ) btn->onclick(gui, event);
		return 0;
	}

	gui_event_key(gui, event);
	return 0;
}

int gui_button_event_mouse(gui_s* gui, xorgEvent_s* event){
	iassert(gui->type == GUI_TYPE_BUTTON);

	if( event->mouse.event == XORG_MOUSE_PRESS && event->mouse.button == 1 ){
		gui_button_redraw(gui, gui->control, 1);
		gui_draw(gui);
		return 0;
	}
	else if( (event->mouse.event == XORG_MOUSE_RELEASE || event->mouse.event == XORG_MOUSE_CLICK || event->mouse.event == XORG_MOUSE_DBLCLICK) 
			&& event->mouse.button == 1 )
	{
		gui_button_redraw(gui, gui->control, 0);
		gui_draw(gui);
		guiButton_s* btn = gui->control;
		if( btn->onclick ) btn->onclick(gui, event);
	}
	return 0;
}

int gui_event_focus(gui_s* gui, xorgEvent_s* event){
	if( event->focus.outin ){
		gui_border(gui, gui->bordersizefocused);
	}
	else{
		gui_border(gui, gui->bordersize);
	}
	return 0;
}


*/
