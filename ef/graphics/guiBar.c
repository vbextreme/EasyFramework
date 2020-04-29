#include <ef/guiBar.h>
#include <ef/memory.h>
#include <ef/str.h>
#include <ef/err.h>

guiBar_s* gui_bar_new(ftFonts_s* font, g2dColor_t foreground, guiImage_s* fill, double min, double max, double start, unsigned flags){
	guiBar_s* bar = mem_new(guiBar_s);
	if( !bar ) return NULL;
	bar->min = min;
	bar->max = max;
	bar->current = start;
	bar->flags = flags;
	bar->fonts = font;
	bar->foreground = foreground;
	bar->text = NULL;
	bar->textWidth = 0;
	bar->textHeight = 0;
	bar->render = NULL;
	bar->fill = fill;
	return bar;
}

gui_s* gui_bar_attach(gui_s* gui, guiBar_s* bar){
	if( !gui ) goto ERR;
	if( !bar ) goto ERR;

	gui->control = bar;
	gui->type = GUI_TYPE_BAR;
	gui->redraw = gui_bar_event_redraw;
	//TODO gui->mouse = gui_button_event_mouse;
	//TODO gui->key = gui_button_event_key;
	gui->free = gui_bar_event_free;
	//TODO add composite fn
	gui_composite_add(gui->img, bar->fill);
	bar->render = gui_image_custom_new(
			g2d_new(gui->surface->img->w, gui->surface->img->h, -1), 
			GUI_IMAGE_FLAGS_ALPHA
	);
	gui_composite_add(gui->img, bar->render);

	return gui;

ERR:
	if( bar ) gui_bar_free(bar);
	if( gui ) gui_free(gui);
	return NULL;
}

void gui_bar_free(guiBar_s* bar){
	if( bar->text ) free(bar->text);
	if( bar->textdescript ) free(bar->textdescript);
	free(bar);
}

void gui_bar_text_set(gui_s* gui, const utf8_t* text){
	iassert(gui->type == GUI_TYPE_BAR);
	guiBar_s* bar = gui->control;
	if( bar->text ) free(bar->text);
	if( text ){
		free(bar->textdescript);
		bar->textdescript = (utf8_t*)str_dup((char*)text,0);
	}
	double current = bar->flags & GUI_BAR_SHOW_PERCENT ? (100.0 * bar->current) / (bar->max - bar->min) : bar->current;
	char* p = bar->flags & GUI_BAR_SHOW_PERCENT ? "%" : "";
	
	char txtmin[128] = {[0]=0};
	if( bar->flags & GUI_BAR_SHOW_MIN ) sprintf(txtmin, " %.1f", bar->min);
	char txtcur[128] = {[0]=0};
	if( bar->flags & GUI_BAR_SHOW_CURRENT ) sprintf(txtcur, " %.1f%s", current, p);
	char txtmax[128] = {[0]=0};
	if( bar->flags & GUI_BAR_SHOW_MIN ) sprintf(txtmax, " %.1f", bar->max);

	bar->text = (utf8_t*)str_printf("%s%s%s%s", (char*)bar->textdescript, txtmin, txtcur, txtmax);
	bar->textHeight = ft_multiline_height(bar->fonts, bar->text);
	bar->textWidth  = ft_multiline_lenght(bar->fonts, bar->text);
	bar->flags |= GUI_BAR_RENDERING;
}

__private void bar_hori(gui_s* gui){
	iassert(gui->type == GUI_TYPE_BAR);
	guiBar_s* bar = gui->control;
	const double max = bar->max - bar->min;
	bar->fill->pos.x = 0;
	bar->fill->pos.y = 0;
	bar->fill->pos.w = (gui->surface->img->w * bar->current) / max;
	bar->fill->pos.h = gui->surface->img->h;
	bar->fill->src = bar->fill->pos;
	if( bar->fill->pos.w > gui->surface->img->w ) bar->fill->pos.w = gui->surface->img->w;
}

__private void bar_vert(gui_s* gui){
	iassert(gui->type == GUI_TYPE_BAR);
	guiBar_s* bar = gui->control;
	const double max = bar->max - bar->min;
	unsigned height = (gui->surface->img->h * bar->current) / max;
	if( height > gui->surface->img->h ) height = gui->surface->img->h;
	bar->fill->pos.x = 0;
	bar->fill->pos.y = gui->surface->img->h - height;
	bar->fill->pos.w = gui->surface->img->w;
	bar->fill->pos.h = height;
	bar->fill->src = bar->fill->pos;
}

void gui_bar_current_set(gui_s* gui, double current){
	iassert(gui->type == GUI_TYPE_BAR);
	guiBar_s* bar = gui->control;
	if( current > bar->max ) current = bar->max;
	if( current < bar->min ) current = bar->min;
	bar->current = current;
	if( bar->flags & GUI_BAR_HORIZONTAL ) bar_hori(gui);
	else if ( bar->flags & GUI_BAR_VERTICAL ) bar_vert(gui);
	gui_bar_text_set(gui, NULL);
}

__private void bar_render_text(guiBar_s* bar){
	if( !bar->text ) return;
	bar->flags &= ~GUI_BAR_RENDERING;
	
	if( bar->render->img->w < bar->textWidth || bar->render->img->h < bar->textHeight ){
		g2d_free(bar->render->img);
		bar->render->img = g2d_new(bar->textWidth, bar->textHeight, -1);
	}

	g2dCoord_s pen = {
		.x = 0,
		.y = 0,
		.w = bar->textWidth,
		.h = bar->textHeight
	};
	g2d_clear(bar->render->img, gui_color( 0, 255, 255, 255), &pen);

	const utf8_t* txt = bar->text;
	while( (txt=g2d_string(bar->render->img, &pen, bar->fonts, txt, bar->foreground, pen.x,1)) );
}

void gui_bar_redraw(gui_s* gui){
	iassert(gui->type == GUI_TYPE_BAR);
	guiBar_s* bar = gui->control;
	if( bar->flags & GUI_BAR_RENDERING ){
		bar_render_text(bar);
		unsigned x = 0;
		unsigned y = 0;
		if( bar->flags & GUI_BAR_CENTER_X ){
			x = abs((int)gui->surface->img->w / 2 - (int)bar->textWidth / 2);
		}
		if( bar->flags & GUI_BAR_CENTER_Y ){
			y = abs((int)gui->surface->img->h / 2 - (int)bar->textHeight / 2);
		}
		gui_image_xy_set(bar->render, x, y);
	}	
	gui_composite_redraw(gui, gui->img);
}

int gui_bar_event_free(gui_s* gui, __unused xorgEvent_s* ev){
	iassert(gui->type == GUI_TYPE_BAR);
	gui_bar_free(gui->control);
	return 0;
}

int gui_bar_event_redraw(gui_s* gui, __unused xorgEvent_s* unset){
	iassert(gui->type == GUI_TYPE_BAR);
	gui_bar_redraw(gui);
	return 0;
}

/*
int gui_bar_event_themes(gui_s* gui, xorgEvent_s* ev){
	guiBar_s* bar = ev->data.request;
	
	gui_themes_uint_set(ev->data.data, GUI_THEMES_BAR_COLOR, &bar->color);
	__mem_free char* mode = gui_themes_string(ev->data.data, GUI_THEMES_BAR_MODE);
	if( mode ){
		if( !strcmp(mode, "horizontal") ) gui->background[0]->fn = gui_bar_hori_color_fn;
		else if( !strcmp(mode, "vertical") ) gui->background[0]->fn = gui_bar_vert_color_fn;
	}
	ev->data.request = bar->label;
	gui_label_event_themes(gui, ev);

	return 0;
}
*/
