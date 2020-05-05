#include <ef/guiDiv.h>
#include <ef/memory.h>
#include <ef/vector.h>
#include <ef/str.h>
#include <ef/err.h>

__private int div_internal_focus(gui_s* gui, xorgEvent_s* ev){
	gui_div_event_redraw(gui, ev);
	gui_draw(gui);
	return 0;
}

guiDiv_s* gui_div_new(guiDivMode_e mode, guiImage_s* select, unsigned flags){
	guiDiv_s* div = mem_new(guiDiv_s);
	if( !div ) return NULL;
	div->mode = mode;
	div->padding.left = div->padding.right = div->padding.top = div->padding.bottom = GUI_DIV_DEFAULT_PADDING; 
	div->selectpad.left = div->selectpad.right = div->selectpad.top = div->selectpad.bottom = GUI_DIV_DEFAULT_PADDING;
	div->scroll.x = 0;
	div->scroll.y = 0;
	div->flags = flags;
	div->select = select;
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
	gui->themes = gui_div_event_themes;
	if( gui->parent ) gui->focus = NULL;
	return gui;
ERR:
	if( div ) gui_div_free(div);
	if( gui ) gui_free(gui);
	return NULL;
}

void gui_div_apply_select(gui_s* gui){
	iassert( gui->type == GUI_TYPE_DIV );
	guiDiv_s* div = gui->control;
	if( !(div->flags & GUI_DIV_FLAGS_SELECT) ) return;
	div->idselect = vector_count(gui->img->img);
	gui_composite_add(gui->img, div->select);
	gui_register_internal_focus_event(gui, div_internal_focus);
}

void gui_div_deapply_select(gui_s* gui){
	iassert( gui->type == GUI_TYPE_DIV );
	guiDiv_s* div = gui->control;
	div->flags &= ~GUI_DIV_FLAGS_SELECT;
	gui_image_free(gui->img->img[div->idselect]);
	vector_remove(gui->img->img, div->idselect);
	gui_unregister_internal_focus_event(gui);
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

void gui_div_sel_padding_top(gui_s* gui, int top){
	iassert( gui->type == GUI_TYPE_DIV );
	guiDiv_s* div = gui->control;
	div->selectpad.top = top;
}

void gui_div_sel_padding_bottom(gui_s* gui, int bottom){
	iassert( gui->type == GUI_TYPE_DIV );
	guiDiv_s* div = gui->control;
	div->selectpad.bottom = bottom;
}

void gui_div_sel_padding_left(gui_s* gui, int left){
	iassert( gui->type == GUI_TYPE_DIV );
	guiDiv_s* div = gui->control;
	div->selectpad.left = left;
}

void gui_div_sel_padding_right(gui_s* gui, int right){
	iassert( gui->type == GUI_TYPE_DIV );
	guiDiv_s* div = gui->control;
	div->selectpad.right = right;
}

guiDivRow_s* gui_div_table_create_row(gui_s* tab, double raph){
	iassert( tab->type == GUI_TYPE_DIV );
	guiDiv_s* div = tab->control;
	guiDivRow_s* row = vector_get_push_back(div->vrows);
	row->proph = raph;
	row->vcols = vector_new(guiDivCols_s, 3, 3);
	if( !row->vcols ) err_fail("eom");
	return row;
}

guiDivRow_s* gui_div_table_row_get(gui_s* tab, unsigned idrow){
	iassert( tab->type == GUI_TYPE_DIV );
	guiDiv_s* div = tab->control;
	return idrow < vector_count(div->vrows) ? &div->vrows[idrow] : NULL;
}

guiDivRow_s* gui_div_table_row_last(gui_s* tab){
	iassert( tab->type == GUI_TYPE_DIV );
	guiDiv_s* div = tab->control;
	return vector_count(div->vrows) ? &div->vrows[vector_count(div->vrows)-1] : NULL;
}

void gui_div_table_col_new(gui_s* gui, guiDivRow_s* row, gui_s* child, double propw, int flags){
	guiDiv_s* div = gui->control;
	if( !row ) return;
	guiDivCols_s* col = vector_get_push_back(row->vcols);
	col->gui = child;
	col->propw = propw;
	col->flags = flags != -1 ? (unsigned)flags : div->flags;
}

void gui_div_table_attach(guiDivRow_s* row, unsigned idcol, gui_s* child, double propw, int flags){
	if( !row ) return;
	if( idcol >= vector_count(row->vcols) ) return;
	guiDivCols_s* col = &row->vcols[idcol];
	col->gui = child;
	col->propw = propw;
	if( flags != -1 ) col->flags = flags;
}

__private void div_child_move(gui_s* gdiv, guiDiv_s* div, gui_s* child, const guiPosition_s* const guiPos){
	const int fit = div->flags & GUI_DIV_FLAGS_FIT;
	const int sel = div->flags & GUI_DIV_FLAGS_SELECT;
	
	if( sel && gui_focus_have(child) ){
		assert(div->select);
		div->select->pos.x = guiPos->x;
		div->select->pos.y = guiPos->y;
		gui_image_resize(gdiv, div->select, guiPos->w, guiPos->h, -1);
		gui_move(child, 
			guiPos->x + div->selectpad.left + child->userMargin.left, 
			guiPos->y + div->selectpad.top + child->userMargin.top
		);
		if( fit ){
			gui_resize(child,
				guiPos->w - (div->selectpad.left + div->selectpad.right + child->userMargin.left + child->userMargin.right + child->bordersize*2),
				guiPos->h - (div->selectpad.top + div->selectpad.bottom + child->userMargin.top + child->userMargin.bottom + child->bordersize*2)
			);
		}
	}
	else{
		gui_move(child, 
			guiPos->x + child->userMargin.left,
			guiPos->y + child->userMargin.top
		);
		if( fit ){
			gui_resize(child, 
				guiPos->w - (child->userMargin.left + child->userMargin.right + child->bordersize*2),
				guiPos->h - (child->userMargin.top + child->userMargin.bottom + child->bordersize*2)
			);
		}
	}
}

__private void div_align_vertical(gui_s* gui, guiDiv_s* div){
	guiPosition_s pos = {
		.x = div->padding.left,
		.y = div->padding.top - div->scroll.y,
		.w = gui->surface->img->w,
		.h = 0
	};

	vector_foreach(gui->childs, i){
		gui_s* child = gui->childs[i];
		pos.h = child->position.h;
		div_child_move(gui, div, child, &pos);
		pos.y += pos.h + child->userMargin.top + child->userMargin.bottom + child->bordersize * 2;
	}
}

__private void div_align_horizontal(gui_s* gui, guiDiv_s* div){
	guiPosition_s pos = {
		.x = div->padding.left - div->scroll.x,
		.y = div->padding.top,
		.w = 0,
		.h = gui->surface->img->h
	};

	vector_foreach(gui->childs, i){
		gui_s* child = gui->childs[i];
		pos.h = child->position.h;
		div_child_move(gui, div, child, &pos);
		pos.x += child->bordersize * 2 + child->position.w + child->userMargin.right + child->userMargin.left;
	}
}

__private void div_align_table(gui_s* gui, guiDiv_s* div){
	const size_t rowscount = vector_count(div->vrows);
	const size_t avw = gui->surface->img->w - (div->padding.left+div->padding.right);
	const size_t avh = gui->surface->img->h - (div->padding.top+div->padding.bottom);
	const int x = div->padding.left - div->scroll.x;

	guiPosition_s pos = {
		.x = 0,
		.y = div->padding.top - div->scroll.y,
		.w = 0,
		.h = 0
	};

	for( size_t r = 0; r < rowscount; ++r){
		pos.x = x;
		const guiDivRow_s* const row = &div->vrows[r];
		const size_t colscount = vector_count(row->vcols);
		pos.h = ((double)avh * row->proph)/100.0;
		for( size_t c = 0; c < colscount; ++c){
			const guiDivCols_s* const col = &row->vcols[c];
			pos.w = ((double)avw* col->propw)/100.0;
			if( col->gui ) div_child_move(gui, div, col->gui, &pos);
			pos.x += pos.w;
		}
		pos.y += pos.h;
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
	guiDiv_s* div = gui->control;
	if( div->flags & GUI_DIV_FLAGS_SELECT ) gui_div_deapply_select(gui);
	gui_div_free(div);
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
	iassert(gui->type == GUI_TYPE_DIV);
	gui_div_align(gui);
	gui_composite_redraw(gui, gui->img);
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

int gui_div_event_themes(gui_s* gui, xorgEvent_s* ev){
	iassert(gui->type == GUI_TYPE_DIV);
	guiDiv_s* div = gui->control;
	char* name = ev->data.data;

	gui_themes_int_set(name, GUI_THEME_DIV_PAD_LEFT, &div->padding.left);
	gui_themes_int_set(name, GUI_THEME_DIV_PAD_RIGHT, &div->padding.right);
	gui_themes_int_set(name, GUI_THEME_DIV_PAD_TOP, &div->padding.top);
	gui_themes_int_set(name, GUI_THEME_DIV_PAD_BOTTOM, &div->padding.bottom);
	gui_themes_int_set(name, GUI_THEME_DIV_SEL_PAD_LEFT, &div->selectpad.left);
	gui_themes_int_set(name, GUI_THEME_DIV_SEL_PAD_RIGHT, &div->selectpad.right);
	gui_themes_int_set(name, GUI_THEME_DIV_SEL_PAD_TOP, &div->selectpad.top);
	gui_themes_int_set(name, GUI_THEME_DIV_SEL_PAD_BOTTOM, &div->selectpad.bottom);

	__mem_free char* selname = str_printf("%s.%s", name, GUI_THEME_DIV_SELECTION);
	if( !gui_themes_gui_image(gui, selname, &div->select) ) div->flags |= GUI_DIV_FLAGS_SELECT;

	if( div->mode != GUI_DIV_TABLE ) return 0;

	vector_foreach(div->vrows, i){
		__mem_free char* prname = str_printf("%s.%lu.%s", GUI_THEME_DIV_ELEMENT, i, GUI_THEME_DIV_PROPH);
		char* dval = gui_themes_string(name, prname);
		if( dval ){
			div->vrows[i].proph = strtod(dval, NULL);
			free(dval);
		}
		vector_foreach(div->vrows[i].vcols, j){
			__mem_free char* prname = str_printf("%s.%lu.%lu.%s", GUI_THEME_DIV_ELEMENT, i, j, GUI_THEME_DIV_PROPW);
			char* dval = gui_themes_string(name, prname);
			if( dval ){
				div->vrows[i].vcols[j].propw = strtod(dval, NULL);
				free(dval);
			}
		}
	}

	return 0;
}


