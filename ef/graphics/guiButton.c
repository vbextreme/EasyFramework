#include <ef/guiButton.h>
#include <ef/memory.h>
#include <ef/vector.h>
#include <ef/str.h>
#include <ef/err.h>

guiButton_s* gui_button_new(ftFonts_s* font, g2dColor_t foreground, unsigned flags, guiEvent_f onclick){
	guiButton_s* btn = mem_new(guiButton_s);
	if( !btn ) err_fail("eom");
	btn->text = NULL;
	btn->textWidth = 0;
	btn->textHeight = 0;
	btn->fonts = font;
	btn->foreground = foreground;
	btn->render = NULL;
	btn->state[0] = NULL;
	btn->state[1] = NULL;
	btn->state[2] = NULL;
	btn->flags = flags;
	btn->onclick = onclick;
	return btn;
}

gui_s* gui_button_attach(gui_s* gui, guiButton_s* btn, guiImage_s* press, guiImage_s* hover){
	if( !gui ) goto ERR;
	if( !btn ) goto ERR;
	btn->parentKey = gui->key;
	gui->control = btn;
	gui->type = GUI_TYPE_BUTTON;
	gui->redraw = gui_button_event_redraw;
	gui->mouse = gui_button_event_mouse;
	gui->key = gui_button_event_key;
	gui->free = gui_button_event_free;
	btn->render = gui_image_custom_new(
			g2d_new(gui->surface->img->w, gui->surface->img->h, -1), 
			GUI_IMAGE_FLAGS_ALPHA
	);
	btn->compindex = vector_count(gui->img)-1;
	btn->state[GUI_BUTTON_STATE_NORMAL] = gui->img->img[btn->compindex];
	btn->state[GUI_BUTTON_STATE_PRESS] = press;
	btn->state[GUI_BUTTON_STATE_HOVER] = hover;
	gui_composite_add(gui->img, btn->render);
	return gui;
ERR:
	if( btn ) gui_button_free(btn);
	if( gui ) gui_free(gui);
	return NULL;
}

void gui_button_free(guiButton_s* btn){
	if( btn->text ) free(btn->text);
	gui_image_free(btn->state[1]);
	gui_image_free(btn->state[2]);
	free(btn);
}

void gui_button_text_set(gui_s* gui, const utf8_t* text){
	iassert(gui->type == GUI_TYPE_BUTTON);
	guiButton_s* btn = gui->control;
	if( btn->text ) free(btn->text);
	btn->text = (utf8_t*)str_dup((const char*)text, 0);
	btn->textHeight = ft_multiline_height(btn->fonts, btn->text);
	btn->textWidth  = ft_multiline_lenght(btn->fonts, btn->text);
	btn->flags |= GUI_BUTTON_RENDERING;
}

__private void button_render(guiButton_s* btn){
	if( !btn->text ) return;
	btn->flags &= ~GUI_BUTTON_RENDERING;
	
	if( btn->render->img->w < btn->textWidth || btn->render->img->h < btn->textHeight ){
		g2d_free(btn->render->img);
		btn->render->img = g2d_new(btn->textWidth, btn->textHeight, -1);
	}

	g2dCoord_s pen = {
		.x = 0,
		.y = 0,
		.w = btn->textWidth,
		.h = btn->textHeight
	};
	g2d_clear(btn->render->img, gui_color( 0, 255, 255, 255), &pen);

	const utf8_t* txt = btn->text;
	while( (txt=g2d_string(btn->render->img, &pen, btn->fonts, txt, btn->foreground, pen.x,1)) );
}

void gui_button_redraw(gui_s* gui, unsigned normalPressHover){
	iassert(gui->type == GUI_TYPE_BUTTON);
	guiButton_s* btn = gui->control;
	if( btn->flags & GUI_BUTTON_RENDERING ) button_render(btn);
	unsigned x = 0;
	unsigned y = 0;
	if( btn->flags & GUI_BUTTON_CENTER_X ){
		x = abs((int)gui->surface->img->w / 2 - (int)btn->textWidth / 2);
	}
	if( btn->flags & GUI_BUTTON_CENTER_Y ){
		y = abs((int)gui->surface->img->h / 2 - (int)btn->textHeight / 2);
	}
	gui_image_xy_set(btn->render, x, y);
	
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
