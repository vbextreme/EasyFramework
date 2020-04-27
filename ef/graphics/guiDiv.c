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
	div->scroll.x = 0;
	div->scroll.y = 0;
	return div;
}

gui_s* gui_div_attach(gui_s* gui, guiDiv_s* div){
	if( !gui ) goto ERR;
	if( !div ) goto ERR;
	gui->control = div;
	gui->type = GUI_TYPE_DIV;
	gui->key = gui_div_event_key;
	gui->free = gui_div_event_free;
	if( gui->parent ) gui->focus = NULL;
	return gui;
ERR:
	if( div ) gui_div_free(div);
	if( gui ) gui_free(gui);
	return NULL;
}

void gui_div_free(guiDiv_s* div){
	free(div);
}

__private void div_align_vertical(gui_s* gui, guiDiv_s* div){
	const unsigned x = div->sep.x;
	unsigned y = div->sep.y - div->scroll.y;
	
	vector_foreach(gui->childs, i){
		gui_move(gui->childs[i], x, y);
		y += gui->childs[i]->surface->img->h + div->sep.y;
	}
	div->lastElement.x = x;
	div->lastElement.y = y;
}

__private void div_align_horizontal(gui_s* gui, guiDiv_s* div){
	unsigned x = div->sep.x - div->scroll.x;
	const unsigned y = div->sep.y;
	
	vector_foreach(gui->childs, i){
		gui_move(gui->childs[i], x, y);
		x += gui->childs[i]->surface->img->w + div->sep.x;
	}
	div->lastElement.x = x;
	div->lastElement.y = y;
}

__private void div_align_grid(gui_s* gui, guiDiv_s* div){
	unsigned x = div->sep.x;
	unsigned y = div->sep.y - div->scroll.y;
	unsigned maxH = 0;

	vector_foreach(gui->childs, i){
		if( x + gui->childs[i]->surface->img->w > gui->surface->img->w ){
			x = div->sep.x;
			y += maxH;
		}
		if( gui->childs[i]->surface->img->h > maxH ) maxH = gui->childs[i]->surface->img->h;
		gui_move(gui->childs[i], x, y);
		x += gui->childs[i]->surface->img->w + div->sep.x;
	}
	div->lastElement.x = x;
	div->lastElement.y = y;
}

void gui_div_align(gui_s* gui, guiDiv_s* div){
	switch( div->mode ){
		default: case GUI_DIV_NONE: break;
		case GUI_DIV_VERTICAL: div_align_vertical(gui, div); break;
		case GUI_DIV_HORIZONTAL: div_align_horizontal(gui, div); break;
		case GUI_DIV_GRID: div_align_grid(gui, div); break;
	}
}

int gui_div_event_free(gui_s* gui, __unused xorgEvent_s* ev){
	iassert(gui->type == GUI_TYPE_DIV);
	gui_div_free(gui->control);
	return 0;
}

__private void div_focus_leftright(gui_s* gui, guiDiv_s* div, int right){
	const int id = right ? gui_focus_next_id(gui) :gui_focus_prev_id(gui);
	if( id < 0 ) return;

	switch( div->mode ){
		default: case GUI_DIV_NONE:	break;

		case GUI_DIV_VERTICAL: return;
		
		case GUI_DIV_HORIZONTAL:
			if( gui->childs[id]->position.x + gui->childs[id]->position.w > gui->position.w ){
				div->scroll.x += (gui->childs[id]->position.x + gui->childs[id]->position.w + div->sep.x) - gui->position.w ;
				div_align_horizontal(gui, div);
			}
			else if( gui->childs[id]->position.x < 0 ){
				div->scroll.x -= (-gui->childs[id]->position.x) + div->sep.x;
				div_align_horizontal(gui, div);		
			}
		break;

		case GUI_DIV_GRID:
			if( gui->childs[id]->position.y + gui->childs[id]->position.h > gui->position.h ){
				div->scroll.y += (gui->childs[id]->position.y + gui->childs[id]->position.h + div->sep.y) - gui->position.h ;
				div_align_grid(gui, div);
			}
			else if( gui->childs[id]->position.y < 0 ){
				div->scroll.y -= (-gui->childs[id]->position.y) + div->sep.y;
				div_align_grid(gui, div);		
			}
		break;
	}

	gui_focus_from_parent(gui, id);
}

__private void div_grid_current_id(unsigned* curRow, unsigned *curCol, gui_s* gui, guiDiv_s* div){
	unsigned x = div->sep.x;
	unsigned y = div->sep.y - div->scroll.y;
	unsigned maxH = 0;
	int curId = gui->childFocus;
	unsigned row = 0;
	unsigned col = 0;

	vector_foreach(gui->childs, i){
		if( x + gui->childs[i]->surface->img->w > gui->surface->img->w ){
			x = div->sep.x;
			y += maxH;
			col = 0;
			++row;
		}
		if( gui->childs[i]->surface->img->h > maxH ) maxH = gui->childs[i]->surface->img->h;
		if( curId == (int)i ){
			*curRow = row;
			*curCol = col;
			return;
		}		
		x += gui->childs[i]->surface->img->w + div->sep.x;
	}
}

__private int div_grid_down_id(gui_s* gui, guiDiv_s* div){
	unsigned x = div->sep.x;
	unsigned y = div->sep.y - div->scroll.y;
	unsigned maxH = 0;
	unsigned row = 0;
	unsigned col = 0;
	unsigned curRow = 0;
	unsigned curCol = 0;
	div_grid_current_id(&curRow, &curCol, gui, div);
	++curRow;

	vector_foreach(gui->childs, i){
		if( x + gui->childs[i]->surface->img->w > gui->surface->img->w ){
			x = div->sep.x;
			y += maxH;
			col = 0;
			++row;
		}
		if( gui->childs[i]->surface->img->h > maxH ) maxH = gui->childs[i]->surface->img->h;
		if( curRow == row && curCol == col ){
			return i;
		}		
		x += gui->childs[i]->surface->img->w + div->sep.x;
	}
	
	return vector_count(gui->childs)-1;
}

__private int div_grid_up_id(gui_s* gui, guiDiv_s* div){
	unsigned x = div->sep.x;
	unsigned y = div->sep.y - div->scroll.y;
	unsigned maxH = 0;
	unsigned row = 0;
	unsigned col = 0;
	unsigned curRow = 0;
	unsigned curCol = 0;

	div_grid_current_id(&curRow, &curCol, gui, div);
	if( curRow == 0 ) return 0;
	--curRow;

	vector_foreach(gui->childs, i){
		if( x + gui->childs[i]->surface->img->w > gui->surface->img->w ){
			x = div->sep.x;
			y += maxH;
			col = 0;
			++row;
		}
		if( gui->childs[i]->surface->img->h > maxH ) maxH = gui->childs[i]->surface->img->h;
		if( curRow == row && curCol == col ){
			return i;
		}		
		x += gui->childs[i]->surface->img->w + div->sep.x;
	}
	return 0;
}

__private void div_focus_updown(gui_s* gui, guiDiv_s* div, int down){
	int id = 0;

	switch( div->mode ){
		default: case GUI_DIV_NONE:
			id = down ? gui_focus_next_id(gui) :gui_focus_prev_id(gui);
			if( id < 0 ) return;
		break;

		case GUI_DIV_VERTICAL: 
			id = down ? gui_focus_next_id(gui) :gui_focus_prev_id(gui);
			if( id < 0 ) return;
			if( gui->childs[id]->position.y + gui->childs[id]->position.h > gui->position.h ){
				div->scroll.y += (gui->childs[id]->position.y + gui->childs[id]->position.h + div->sep.y) - gui->position.h ;
				div_align_grid(gui, div);
			}
			else if( gui->childs[id]->position.y < 0 ){
				div->scroll.y -= (-gui->childs[id]->position.y) + div->sep.y;
				div_align_grid(gui, div);		
			}
		break;
		
		case GUI_DIV_HORIZONTAL: return;

		case GUI_DIV_GRID:
			if( id < 0 ) return;
			id = down ? div_grid_down_id(gui, div) : div_grid_up_id(gui, div);
			if( gui->childs[id]->position.y + gui->childs[id]->position.h > gui->position.h ){
				div->scroll.y += (gui->childs[id]->position.y + gui->childs[id]->position.h + div->sep.y) - gui->position.h ;
				div_align_grid(gui, div);
			}
			else if( gui->childs[id]->position.y < 0 ){
				div->scroll.y -= (-gui->childs[id]->position.y) + div->sep.y;
				div_align_grid(gui, div);		
			}
		break;
	}

	gui_focus_from_parent(gui, id);
}

int gui_div_event_key(gui_s* gui, xorgEvent_s* event){
	iassert(gui->type == GUI_TYPE_DIV);

	if( event->keyboard.event == XORG_KEY_PRESS && event->keyboard.keysym == XKB_KEY_Right ){
		div_focus_leftright(gui, gui->control, 1);
	}
	else if( event->keyboard.event == XORG_KEY_PRESS && event->keyboard.keysym == XKB_KEY_Left ){
		div_focus_leftright(gui, gui->control, 0);
	}
	else if( event->keyboard.event == XORG_KEY_PRESS && event->keyboard.keysym == XKB_KEY_Up ){
		div_focus_updown(gui, gui->control, 0);
	}
	else if( event->keyboard.event == XORG_KEY_PRESS && event->keyboard.keysym == XKB_KEY_Down ){
		div_focus_updown(gui, gui->control, 1);
	}

	return 0;
}

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
