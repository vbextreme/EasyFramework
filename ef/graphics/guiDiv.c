#include <ef/guiDiv.h>
#include <ef/memory.h>
#include <ef/vector.h>
#include <ef/err.h>

guiDiv_s* gui_div_new(guiDivMode_e mode, unsigned flags){
	guiDiv_s* div = mem_new(guiDiv_s);
	if( !div ) return NULL;
	div->mode = mode;
	div->padding.left = div->padding.right = div->padding.top = div->padding.bottom = GUI_DIV_DEFAULT_PADDING; 
	div->scroll.x = 0;
	div->scroll.y = 0;
	div->flags = flags;
	if( mode == GUI_DIV_TABLE && !(div->vrows = vector_new(guiDivRow_s, 4, 4)) ) err_fail("eom");
	return div;
}

gui_s* gui_div_attach(gui_s* gui, guiDiv_s* div){
	if( !gui ) goto ERR;
	if( !div ) goto ERR;
	gui->control = div;
	gui->type = GUI_TYPE_DIV;
	gui->key = gui_div_child_event_key;
	gui->free = gui_div_event_free;
	gui->redraw = gui_div_event_redraw;
	gui->move = gui_div_event_move;
	if( gui->parent ) gui->focus = NULL;
	return gui;
ERR:
	if( div ) gui_div_free(div);
	if( gui ) gui_free(gui);
	return NULL;
}

void gui_div_free(guiDiv_s* div){
	vector_foreach(div->vrows, i){
		vector_free(div->vrows[i].vcols);
	}
	vector_free(div->vrows);
	free(div);
}

void gui_div_padding_top(gui_s* gui, int top){
	iassert( gui->type == GUI_TYPE_DIV );
	guiDiv_s* div = gui->control;
	div->padding.top = top;
}

void gui_div_padding_bottom(gui_s* gui, int bottom){
	iassert( gui->type == GUI_TYPE_DIV );
	guiDiv_s* div = gui->control;
	div->padding.bottom = bottom;
}

void gui_div_padding_left(gui_s* gui, int left){
	iassert( gui->type == GUI_TYPE_DIV );
	guiDiv_s* div = gui->control;
	div->padding.left = left;
}

void gui_div_padding_right(gui_s* gui, int right){
	iassert( gui->type == GUI_TYPE_DIV );
	guiDiv_s* div = gui->control;
	div->padding.right = right;
}

guiDivRow_s* gui_div_table_create_row(gui_s* tab, double raph, unsigned cols){
	iassert( tab->type == GUI_TYPE_DIV );
	guiDiv_s* div = tab->control;
	guiDivRow_s* row = vector_get_push_back(div->vrows);
	row->proph = raph;
	row->vcols = vector_new(guiDivCols_s, cols+1, 1);
	if( !row->vcols ) err_fail("eom");
	for( size_t c = 0; c < cols; ++c){
		guiDivCols_s* col = vector_get_push_back(row->vcols);
		col->flags = div->flags;
		col->gui = NULL;
		col->propw = 100.0/cols;
	}
	return row;
}

guiDivRow_s* gui_div_table_row_get(gui_s* tab, unsigned idrow){
	iassert( tab->type == GUI_TYPE_DIV );
	guiDiv_s* div = tab->control;
	return idrow < vector_count(div->vrows) ? &div->vrows[idrow] : NULL;
}
	
void gui_div_table_attach(guiDivRow_s* row, gui_s* child, unsigned idcol, double propw, int flags){
	if( !row ) return;
	if( idcol >= vector_count(row->vcols) ) return;
	guiDivCols_s* col = &row->vcols[idcol];
	col->gui = child;
	col->propw = propw;
	if( flags != -1 ) col->flags = flags;
}

__private void div_align_vertical(gui_s* gui, guiDiv_s* div){
	const unsigned x = div->padding.left;
	int y = div->padding.top - div->scroll.y;
	const int fit = div->flags & GUI_DIV_FLAGS_FIT;

	vector_foreach(gui->childs, i){
		gui_s* child = gui->childs[i];
		y += child->userMargin.top + child->bordersize;
		gui_move(child, x + child->userMargin.left, y);
		if( fit ) 
			gui_resize(child, 
					gui->surface->img->w - (div->padding.left + div->padding.right + child->userMargin.left + child->userMargin.right + child->bordersize* 2 ),
					child->position.h
			);
		y += child->position.h + child->userMargin.bottom + child->bordersize;
	}
}

__private void div_align_horizontal(gui_s* gui, guiDiv_s* div){
	int x = div->padding.left - div->scroll.x;
	const unsigned y = div->padding.top;
	const int fit = div->flags & GUI_DIV_FLAGS_FIT;

	vector_foreach(gui->childs, i){
		gui_s* child = gui->childs[i];
		x += child->userMargin.left + child->bordersize;
		gui_move(child, x, y + child->userMargin.top);
		if( fit ) 
			gui_resize(child, 
					child->position.w, 
					gui->surface->img->h - (child->bordersize * 2 + div->padding.top + div->padding.bottom + child->userMargin.top+child->userMargin.bottom)
			);
		x += child->bordersize + child->position.w + child->userMargin.right;
	}
}

__private void div_align_table(gui_s* gui, guiDiv_s* div){
	int y = div->padding.top - div->scroll.y;
	const size_t rowscount = vector_count(div->vrows);
	const size_t avw = gui->surface->img->w - (div->padding.left+div->padding.right);
	const size_t avh = gui->surface->img->h - (div->padding.top+div->padding.bottom);

	for( size_t r = 0; r < rowscount; ++r){
		int x = div->padding.left - div->scroll.x;
		const guiDivRow_s* row = &div->vrows[r];
		const size_t colscount = vector_count(row->vcols);
		const unsigned ph = ((double)avh * row->proph)/100.0;
		for( size_t c = 0; c < colscount; ++c){
			const guiDivCols_s* col = &row->vcols[c];
			const unsigned pw = ((double)avw* col->propw)/100.0;
			dbg_info("%s move to %u %u", col->gui->name, x + col->gui->userMargin.left, y + col->gui->userMargin.top);
			gui_move(col->gui, x + col->gui->userMargin.left, y + col->gui->userMargin.top);
			if( col->flags & GUI_DIV_FLAGS_FIT ){
				dbg_info("%s resize to %u*%u->%u*%u", col->gui->name,
						col->gui->position.w,
						col->gui->position.h,
						pw - (col->gui->userMargin.left + col->gui->userMargin.right),
						ph - (col->gui->userMargin.top + col->gui->userMargin.bottom)
				);

				gui_resize(
						col->gui, 
						pw - (col->gui->userMargin.left + col->gui->userMargin.right), 
						ph - (col->gui->userMargin.top + col->gui->userMargin.bottom)
				);
			}
			x += pw;
		}
		y += ph;
	}
}

void gui_div_align(gui_s* gui){
	iassert(gui->type == GUI_TYPE_DIV);
	guiDiv_s* div = gui->control;
	switch( div->mode ){
		default: case GUI_DIV_NONE: break;
		case GUI_DIV_VERTICAL: div_align_vertical(gui, div); break;
		case GUI_DIV_HORIZONTAL: div_align_horizontal(gui, div); break;
		case GUI_DIV_TABLE: div_align_table(gui, div); break;
	}
}

int gui_div_event_free(gui_s* gui, __unused xorgEvent_s* ev){
	iassert(gui->type == GUI_TYPE_DIV);
	gui_div_free(gui->control);
	return 0;
}

__private err_t div_table_rc(size_t* outr, size_t* outc, guiDiv_s* div, gui_s* find){
	const size_t rowscount = vector_count(div->vrows);
	for( size_t r = 0; r < rowscount; ++r){
		const guiDivRow_s* row = &div->vrows[r];
		const size_t colscount = vector_count(row->vcols);
		for( size_t c = 0; c < colscount; ++c){
			const guiDivCols_s* col = &row->vcols[c];
			if( col->gui == find ){
				*outr = r;
				*outc = c;
				return 0;
			}
		}
	}
	return -1;
}

__private void div_autoscroll(gui_s* gui, guiDiv_s* div, gui_s* on){
	int realign = 0;
	if( on->position.y < 0 ){
		div->scroll.y -= abs(on->position.y) + on->userMargin.top + div->padding.top;
		++realign;
	}
	else if( on->position.y + on->position.h > gui->surface->img->h ){
		div->scroll.y += (on->position.y + on->position.h + on->userMargin.bottom + div->padding.bottom) - gui->surface->img->h;
		gui_div_align(gui);
		++realign;
	}
	if( on->position.x < 0 ){
		div->scroll.x -= abs(on->position.x) + on->userMargin.left + div->padding.left;
		++realign;
	}
	else if( on->position.x + on->position.w > gui->surface->img->w ){
		div->scroll.x += (on->position.x + on->position.h + on->userMargin.right + div->padding.right) - gui->surface->img->w;
		++realign;
	}
	if( realign ) gui_div_align(gui);
}

__private void div_focus_right(gui_s* gdiv, guiDiv_s* div, gui_s* gui){
	if( div->mode == GUI_DIV_TABLE ){
		size_t r,c;
		if( div_table_rc(&r, &c, div, gui) ) return;
		if( c < vector_count(div->vrows[r].vcols) - 1 ){
			++c;
		}
		else{
			if( r < vector_count(div->vrows) - 1 ){
				++r;
			}
			else{
				r = 0;
			}
			c = 0;
		}
		gui = div->vrows[r].vcols[c].gui;
		gui_focus(gui);
	}
	else{
		gui_focus_next(gui);
	}
	if( gui ) div_autoscroll(gdiv, div, gui);
}

__private void div_focus_left(gui_s* gdiv, guiDiv_s* div, gui_s* gui){
	if( div->mode == GUI_DIV_TABLE ){
		size_t r,c;
		if( div_table_rc(&r, &c, div, gui) ) return;
		if( c > 0 ){
			--c;
		}
		else{
			if( r > 0 ){
			   --r;
		   	}
			else{
				r = vector_count(div->vrows) -1 ;
			}
			c = vector_count(div->vrows[r].vcols) - 1;
		}
		gui = div->vrows[r].vcols[c].gui;
	}
	else{
		gui = gui_focus_prev(gui);
	}
	if( gui ) div_autoscroll(gdiv, div, gui);
}

__private void div_focus_up(gui_s* gdiv, guiDiv_s* div, gui_s* gui){
	if( div->mode == GUI_DIV_TABLE ){
		size_t r,c;
		if( div_table_rc(&r, &c, div, gui) ) return;
		if( r > 0 ){
			--r;
		}
		else{
			r = vector_count(div->vrows) -1 ;
		}
		if( c >= vector_count(div->vrows[r].vcols) ) c = vector_count(div->vrows[r].vcols) -1;
		gui = div->vrows[r].vcols[c].gui;
	}
	else{
		gui = gui_focus_prev(gui);
	}
	if( gui ) div_autoscroll(gdiv, div, gui);
}

__private void div_focus_down(gui_s* gdiv, guiDiv_s* div, gui_s* gui){
	if( div->mode == GUI_DIV_TABLE ){
		size_t r,c;
		if( div_table_rc(&r, &c, div, gui) ) return;
		if( r <  vector_count(div->vrows) - 1){
			++r;
		}
		else{
			r = 0;
		}
		if( c >= vector_count(div->vrows[r].vcols) ) c = vector_count(div->vrows[r].vcols) -1;
		gui = div->vrows[r].vcols[c].gui;
	}
	else{
		gui = gui_focus_next(gui);
	}
	if( gui ) div_autoscroll(gdiv, div, gui);
}

int gui_div_event_redraw(gui_s* gui, __unused xorgEvent_s* event){
	gui_composite_redraw(gui, gui->img);
	gui_div_align(gui);
	return 0;
}

int gui_div_child_event_key(gui_s* gui, xorgEvent_s* event){
	dbg_info("check focus key");
	gui_s* div = gui->parent;
	while( div && div->type != GUI_TYPE_DIV ) div = div->parent; 
	if( !div ){
		dbg_error("div childs object not have div parent");	
		return 0;
	}

	if( event->keyboard.event == XORG_KEY_PRESS && 
			(event->keyboard.keysym == XKB_KEY_Right || event->keyboard.keysym == XKB_KEY_Tab)
	){
		div_focus_right(div, div->control, gui);
	}
	else if( event->keyboard.event == XORG_KEY_PRESS && event->keyboard.keysym == XKB_KEY_Left ){
		div_focus_left(div, div->control, gui);
	}
	else if( event->keyboard.event == XORG_KEY_PRESS && event->keyboard.keysym == XKB_KEY_Up ){
		div_focus_up(div, div->control, gui);
	}
	else if( event->keyboard.event == XORG_KEY_PRESS && event->keyboard.keysym == XKB_KEY_Down ){
		div_focus_down(div, div->control, gui);
	}

	return 0;
}

int gui_div_event_move(gui_s* gui, xorgEvent_s* event){
	gui_event_move(gui, event);
	gui_div_align(gui);
	return 0;
}

/*
int gui_div_event_themes(gui_s* gui, xorgEvent_s* ev){
	guiDiv_s* div = ev->data.request;*
int gui_div_event_themes(gui_s* gui, xorgEvent_s* ev){
	guiDiv_s* div = ev->data.request;

	char* name = ev->data.data;

	gui_themes_uint_set(name, GUI_THEME_DIV_SEP_X, &div->sep.x);
	gui_themes_uint_set(name, GUI_THEME_DIV_SEP_Y, &div->sep.y);

	__mem_free char* align = gui_themes_string(name, GUI_THEME_DIV_ALIGN);
	if( align ){
		if( !strcmp(align, "none") ) div->mode = GUI_DIV_NONE;
		else if( !strcmp(align, "vertical") ) div->mode = GUI_DIV_VERTICAL;
		else if( !strcmp(align, "horizontal") ) div->mode = GUI_DIV_HORIZONTAL;
		gui_div_align(gui, div);	
	}

	return 0;
}
*/
