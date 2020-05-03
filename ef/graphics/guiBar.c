#include <ef/guiBar.h>
#include <ef/memory.h>
#include <ef/str.h>
#include <ef/err.h>

guiBar_s* gui_bar_new(guiCaption_s* caption, guiImage_s* fill, double min, double max, double start, unsigned flags){
	if( !caption ) return NULL;
	guiBar_s* bar = mem_new(guiBar_s);
	if( !bar ) return NULL;
	bar->caption = caption;
	bar->min = min;
	bar->max = max;
	bar->current = start;
	bar->flags = flags;
	bar->fill = fill;
	bar->textdescript = NULL;
	return bar;
}

gui_s* gui_bar_attach(gui_s* gui, guiBar_s* bar){
	if( !gui ) goto ERR;
	if( !bar ) goto ERR;

	gui->control = bar;
	gui->type = GUI_TYPE_BAR;
	gui->redraw = gui_bar_event_redraw;
	gui->free = gui_bar_event_free;
	gui->move = gui_bar_event_move;
	gui_composite_add(gui->img, bar->fill);
	gui_composite_add(gui->img, bar->caption->render);
	return gui;
ERR:
	if( bar ) gui_bar_free(bar);
	if( gui ) gui_free(gui);
	return NULL;
}

void gui_bar_free(guiBar_s* bar){
	gui_caption_free(bar->caption);
	if( bar->textdescript ) free(bar->textdescript);
	free(bar);
}

void gui_bar_text_set(gui_s* gui, const utf8_t* text){
	iassert(gui->type == GUI_TYPE_BAR);
	guiBar_s* bar = gui->control;
	if( text ){
		free(bar->textdescript);
		bar->textdescript = (utf8_t*)str_dup((char*)text,0);
	}
	double current = bar->flags & GUI_BAR_SHOW_PERCENT ? (100.0 * bar->current) / (bar->max - bar->min) : bar->current;
	char* p = bar->flags & GUI_BAR_SHOW_PERCENT ? "%" : "";
	
	char txtmin[128] = {[0]=0};
	if( bar->flags & GUI_BAR_SHOW_MIN ) sprintf(txtmin, "  %.1f", bar->min);
	char txtcur[128] = {[0]=0};
	if( bar->flags & GUI_BAR_SHOW_CURRENT ) sprintf(txtcur, "  %.1f%s", current, p);
	char txtmax[128] = {[0]=0};
	if( bar->flags & GUI_BAR_SHOW_MAX ) sprintf(txtmax, "  %.1f", bar->max);

	__mem_free utf8_t* t = (utf8_t*)str_printf("%s%s%s%s", bar->textdescript ? (char*)bar->textdescript : "", txtmin, txtcur, txtmax);
	gui_caption_text_set(gui, bar->caption, t);
}

__private void bar_hori(gui_s* gui){
	iassert(gui->type == GUI_TYPE_BAR);
	guiBar_s* bar = gui->control;
	const double max = bar->max - bar->min;
	const double cur = bar->current - bar->min;
	bar->fill->pos.x = 0;
	bar->fill->pos.y = 0;
	bar->fill->pos.w = (gui->surface->img->w * cur) / max;
	bar->fill->pos.h = gui->surface->img->h;
	bar->fill->src = bar->fill->pos;
	if( bar->fill->pos.w > gui->surface->img->w ) bar->fill->pos.w = gui->surface->img->w;
}

__private void bar_vert(gui_s* gui){
	iassert(gui->type == GUI_TYPE_BAR);
	guiBar_s* bar = gui->control;
	const double max = bar->max - bar->min;
	const double cur = bar->current - bar->min;
	unsigned height = (gui->surface->img->h * cur) / max;
	if( height > gui->surface->img->h ) height = gui->surface->img->h;
	bar->fill->pos.x = 0;
	bar->fill->pos.y = gui->surface->img->h - height;
	bar->fill->pos.w = gui->surface->img->w;
	bar->fill->pos.h = height;
	bar->fill->src = bar->fill->pos;
}

void gui_bar_circle_fn(gui_s* gui, __unused guiImage_s* img, void* generic){
	iassert(gui->type == GUI_TYPE_BAR);
	g2dColor_t* color = generic;
	guiBar_s* bar = gui->control;
	const int usemin = bar->flags & GUI_BAR_MIN_ANGLE ? 0 : 1;
	const float max = usemin ? bar->max - bar->min : bar->max;
	const float cur = usemin ? bar->current - bar->min : bar->current;
	const float ea = (cur*360.0)/max;
	const float sa = usemin ? 0 : (bar->min*360)/max;
	g2dPoint_s cx = {
		.x = gui->surface->img->w / 2,
		.y = gui->surface->img->h / 2
	};
	unsigned r = MTH_MIN(gui->surface->img->w, gui->surface->img->h);
	r /= 2;
	g2d_pieslice_fill(gui->surface->img, &cx, r, sa, ea, *color);
}

void gui_bar_current_set(gui_s* gui, double current){
	iassert(gui->type == GUI_TYPE_BAR);
	guiBar_s* bar = gui->control;
	if( current > bar->max ) current = bar->max;
	if( current < bar->min ) current = bar->min;
	bar->current = current;
	if( bar->flags & GUI_BAR_HORIZONTAL ) bar_hori(gui);
	else if( bar->flags & GUI_BAR_VERTICAL ) bar_vert(gui);
	gui_bar_text_set(gui, NULL);
}

double gui_bar_current(gui_s* gui){
	iassert(gui->type == GUI_TYPE_BAR);
	guiBar_s* bar = gui->control;
	return bar->current;
}

void gui_bar_max_set(gui_s* gui, double max){
	iassert(gui->type == GUI_TYPE_BAR);
	guiBar_s* bar = gui->control;
	bar->max = max;
	if( bar->current < bar->max ) bar->current = bar->max;
	if( bar->flags & GUI_BAR_HORIZONTAL ) bar_hori(gui);
	else if( bar->flags & GUI_BAR_VERTICAL ) bar_vert(gui);
	gui_bar_text_set(gui, NULL);
}

double gui_bar_max(gui_s* gui){
	iassert(gui->type == GUI_TYPE_BAR);
	guiBar_s* bar = gui->control;
	return bar->max;
}

void gui_bar_min_set(gui_s* gui, double min){
	iassert(gui->type == GUI_TYPE_BAR);
	guiBar_s* bar = gui->control;
	bar->min = min;
	if( bar->current < bar->min ) bar->current = bar->min;
	if( bar->flags & GUI_BAR_HORIZONTAL ) bar_hori(gui);
	else if( bar->flags & GUI_BAR_VERTICAL ) bar_vert(gui);
	gui_bar_text_set(gui, NULL);
}

double gui_bar_min(gui_s* gui){
	iassert(gui->type == GUI_TYPE_BAR);
	guiBar_s* bar = gui->control;
	return bar->min;
}

void gui_bar_redraw(gui_s* gui){
	iassert(gui->type == GUI_TYPE_BAR);
	guiBar_s* bar = gui->control;
	gui_caption_render(gui, bar->caption);
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

int gui_bar_event_move(gui_s* gui, xorgEvent_s* event){
	iassert(gui->type == GUI_TYPE_BAR);
	guiBar_s* bar = gui->control;
	gui_event_move(gui, event);
	bar->caption->flags |= GUI_CAPTION_RENDERING;
	gui_bar_redraw(gui);
	gui_draw(gui);
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
