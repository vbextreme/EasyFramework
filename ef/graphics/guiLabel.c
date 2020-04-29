#include <ef/guiLabel.h>
#include <ef/memory.h>
#include <ef/str.h>
#include <ef/ft.h>
#include <ef/err.h>

guiLabel_s* gui_label_new(ftFonts_s* font, g2dColor_t foreground, unsigned flags){
	guiLabel_s* lbl = mem_new(guiLabel_s);
	if( !lbl ) err_fail("eom");
	lbl->text = NULL;
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
	//gui->themes = gui_label_event_themes;
	gui->free = gui_label_event_free;
	gui->focusable = 0;
	lbl->render = gui_image_custom_new(
			g2d_new(gui->surface->img->w, gui->surface->img->h, -1), 
			GUI_IMAGE_FLAGS_ALPHA
	);
	gui_composite_add(gui->img, lbl->render);
	return gui;
ERR:
	if( lbl ) gui_label_free(lbl);
	if( gui ) gui_free(gui);
	return NULL;
}

void gui_label_free(guiLabel_s* lbl){
	if( lbl->text ) free(lbl->text);
	free(lbl);
}

void gui_label_text_set(gui_s* gui, const utf8_t* text){
	iassert(gui->type == GUI_TYPE_LABEL);
	guiLabel_s* lbl = gui->control;
	if( lbl->text ) free(lbl->text);
	lbl->text = (utf8_t*)str_dup((const char*)text, 0);
	if( lbl->flags & GUI_LABEL_AUTOWRAP ){
		lbl->textWidth = gui->surface->img->w;
		lbl->textHeight =  ft_autowrap_height(lbl->fonts, lbl->text, lbl->textWidth);
	}
	else{
		lbl->textHeight = ft_multiline_height(lbl->fonts, lbl->text);
		lbl->textWidth  = ft_multiline_lenght(lbl->fonts, lbl->text);
	}
	lbl->flags |= GUI_LABEL_RENDERING;
}

__private void label_render(guiLabel_s* lbl){
	if( !lbl->text ) return;
	lbl->flags &= ~GUI_LABEL_RENDERING;
	
	if( lbl->render->img->w < lbl->textWidth || lbl->render->img->h < lbl->textHeight ){
		g2d_free(lbl->render->img);
		lbl->render->img = g2d_new(lbl->textWidth, lbl->textHeight, -1);
	}

	g2dCoord_s pen = {
		.x = 0,
		.y = 0,
		.w = lbl->textWidth,
		.h = lbl->textHeight
	};
	g2d_clear(lbl->render->img, gui_color( 0, 255, 255, 255), &pen);

	if( lbl->flags & GUI_LABEL_AUTOWRAP ){
		g2d_string_autowrap(lbl->render->img, &pen, lbl->fonts, lbl->text, lbl->foreground, pen.x, 1);
	}
	else{
		const utf8_t* txt = lbl->text;
		while( (txt=g2d_string(lbl->render->img, &pen, lbl->fonts, txt, lbl->foreground, pen.x,1)) );
	}
}

void gui_label_redraw(gui_s* gui){
	iassert(gui->type == GUI_TYPE_LABEL);
	guiLabel_s* lbl = gui->control;
	if( lbl->flags & GUI_LABEL_RENDERING ) label_render(lbl);
	unsigned x = lbl->scroll.x;
	unsigned y = lbl->scroll.y;
	if( lbl->flags & GUI_LABEL_CENTER_X ){
		x = abs((int)gui->surface->img->w / 2 - (int)lbl->textWidth / 2);
	}
	if( lbl->flags & GUI_LABEL_CENTER_Y ){
		y = abs((int)gui->surface->img->h / 2 - (int)lbl->textHeight / 2);
	}
	gui_image_xy_set(lbl->render, x, y);
	gui_composite_redraw(gui, gui->img);
}

void gui_label_scroll(gui_s* gui, unsigned x, unsigned y){
	iassert(gui->type == GUI_TYPE_LABEL);
	guiLabel_s* lbl = gui->control;
	if( x < gui->surface->img->w - 5 ) lbl->scroll.x = x;
	if( y < gui->surface->img->h - 5 ) lbl->scroll.y = y;
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
