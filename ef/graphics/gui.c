#include <ef/gui.h>
#include <ef/memory.h>
#include <ef/vector.h>
#include <ef/str.h>
#include <ef/err.h>
#include <ef/str.h>
#include <ef/os.h>
#include <ef/rbhash.h>
#include <ef/phq.h>
#include <ef/delay.h>

#define GUI_CHILD_INITIAL   8
#define GUI_KEY_SIZE        32
#define GUI_HASH_SIZE       32
#define GUI_HASH_MIN        10
#define GUI_TIMERS_SIZE     16
#define GUI_BACKGROUND_SIZE 3

__private rbhash_s* allgui;
__private xorg_s* X;
__private phq_s* timergui;
__private deadpoll_s* dpgui;

void gui_begin(){
	os_begin();
	ft_begin();
	gui_resources_init();
	allgui = rbhash_new(GUI_HASH_SIZE, GUI_HASH_MIN, GUI_KEY_SIZE, NULL, NULL);
	if( !allgui ) err_fail("on init allgui");
	timergui = phq_new(GUI_TIMERS_SIZE, GUI_TIMERS_SIZE, phq_cmp_asc);
	if( !timergui ) err_fail("creating timer gui");
	X = xorg_client_new(NULL, 0);
	dpgui = deadpoll_new();
	if( !dpgui ) err_fail("deadpoll");
	gui_deadpoll_register(dpgui);
}

void gui_end(){
	ft_end();
	rbhash_free(allgui);
	phq_free(timergui);
	xorg_client_free(X);
	gui_resources_free();
	deadpoll_free(dpgui);
}

__private void allgui_add(gui_s* gui){
	dbg_info("register gui: %u", gui->id);
	char key[32];
	int len = sprintf(key,"%u",(uint32_t)gui->id);
	if( rbhash_add_hash(allgui, gui->id, key, len, gui) ){
		err_fail("add gui %d::%s::%s on allgui", (uint32_t)gui->id, gui->name, gui->class);
	}
}

__private void allgui_remove(gui_s* gui){
	char key[32];
	int len = sprintf(key,"%u",(uint32_t)gui->id);
	if( rbhash_remove_hash(allgui, gui->id, key, len) ){
		err_fail("add gui %d::%s::%s on allgui", (uint32_t)gui->id, gui->name, gui->class);
	}
}

__private gui_s* allgui_find(xcb_window_t id){	
	char key[32];
	int len = sprintf(key,"%u",(uint32_t)id);
	return rbhash_find_hash(allgui, id, key, len);
}

void gui_register_root_event(void){
	xorg_register_events(X, xorg_root(X), XCB_EVENT_MASK_PROPERTY_CHANGE);
}

gui_s* gui_new(
		gui_s* parent, 
		const char* name, const char* class, 
		int border, int x, int y, int width, int height, 
		g2dColor_t colorBorder, guiBackground_s* bk,
		int genericSize, void* userdata)
{
	gui_s* gui = mem_new(gui_s);
	if( !gui ) err_fail("malloc");
	
	gui->name = NULL;
	gui->class = NULL;
	gui->parent = parent;
	gui->childs = vector_new(gui_s*, GUI_CHILD_INITIAL, GUI_CHILD_INITIAL);
	xcb_window_t xcbParent = xorg_root(X);
	if(parent){
		gui_child_add(parent, gui);
		xcbParent = parent->id;
	}
	gui->control = NULL;
	gui->userdata = userdata;
	gui->create = NULL;
	gui->destroy = NULL;
	gui->free = NULL;
	gui->redraw = gui_event_redraw;
	gui->draw = gui_event_draw;
	gui->key = gui->parent ? gui_event_key :  NULL;
	gui->mouse = NULL;
	gui->focus = gui->parent ? NULL : gui_event_focus;
	gui->map = NULL;
	gui->move = gui_event_move;
	gui->atom = NULL;
	gui->client = NULL;
	gui->position.x = x;
	gui->position.y = y;
	gui->position.w = width;
	gui->position.h = height;
	gui->background = vector_new(guiBackground_s*, GUI_BACKGROUND_SIZE, GUI_BACKGROUND_SIZE);
	vector_push_back(gui->background, bk);
	gui->surface = NULL;
	gui->type = GUI_TYPE_WINDOW;
	gui->focusable = 1;
	gui->childFocus = -1;
	gui->bordersize = border;
	gui->bordersizefocused = border + GUI_FOCUS_BORDER_SIZE;
	gui->id = xorg_win_new(&gui->surface, X, xcbParent, x, y, width, height, border, colorBorder, gui->background[0]->color);
	gui_name(gui, name);
	gui_class(gui, class);
	gui->genericSize = genericSize;

	allgui_add(gui);

	return gui;
}

void gui_free(gui_s* gui){
	allgui_remove(gui);
	if( gui->free ) gui->free(gui, NULL);

	if( gui->parent ){
		gui_child_remove(gui->parent, gui);
	}
	vector_foreach(gui->childs, i){
		gui_free(gui->childs[i]);
	}
	vector_free(gui->childs);

	if( gui->name ) free(gui->name);
	if( gui->class ) free(gui->class);
	xorg_surface_destroy(X,  gui->surface);
	xorg_win_destroy(X, gui->id);
	free(gui);
}

void gui_child_add(gui_s* parent, gui_s* child){
	vector_push_back(parent->childs, child);
}

gui_s* gui_child_remove(gui_s* parent, gui_s* child){
	vector_foreach(parent->childs, i){
		if(parent->childs[i] == child){
			gui_s* ret = parent->childs[i];
			vector_remove(parent->childs, i);
			return ret;
		}
	}
	return NULL;
}

gui_s* gui_main_parent(gui_s* gui){
	iassert(gui);
	while( gui->parent ) gui = gui->parent;
	return gui;
}

void gui_name(gui_s* gui, const char* name){
	if( !name ) return;
	if( gui->name ) free(gui->name);
	gui->name = str_dup(name, 0);
	xorg_win_title(X, gui->id, gui->name);
}

void gui_class(gui_s* gui, const char* class){
	if( !class ) return;
	if( gui->class ) free(gui->class);
	gui->class = str_dup(class, 0);
	xorg_win_class(X, gui->id, gui->name);
}

void gui_show(gui_s* gui, int show){
	xorg_win_show(X, gui->id, show);
	xorg_win_state_set(X, gui->id, XORG_WINDOW_STATE_INVISIBLE + show);
}

/* event move is raised*/
void gui_move(gui_s* gui, int x, int y){
	xorg_win_move(X, gui->id, x, y);
}

/* event move is raised*/
void gui_resize(gui_s* gui, int w, int h){
	xorg_win_resize(X, gui->id, w, h);
}

void gui_border(gui_s* gui, int border){
	gui->bordersize = border;
	xorg_win_border(X, gui->id, border);
}

void gui_focus_from_parent(gui_s* gui, int id){
	if( id < 0 ) return;
	if( (size_t)id >= vector_count(gui->childs) ) return;
	if( !gui ) return;
	if( gui->focusable < 1 ) return;
	
	gui->childFocus = id;
	dbg_info("set focus: %s", gui->childs[id]->name);
	xorg_win_focus(X, gui->childs[id]->id);
}

__private int gui_focus_search(gui_s* gui, gui_s* find){
	vector_foreach(gui->childs, i){
		if( gui->childs[i] == find ){
			gui_focus_from_parent(gui, i);
			return 1;
		}
		if( vector_count(gui->childs[i]->childs) && gui_focus_search(gui->childs[i], find) ) return 1;
	}
	return 0;
}

void gui_focus(gui_s* gui){
	iassert(gui);
	gui_focus_search(gui_main_parent(gui), gui);
}

int gui_focus_next_id(gui_s* parent){
	if( !parent ){
		dbg_warning("no parent");
		return -1;
	}
	int focusid = parent->childFocus;
	int childs = vector_count(parent->childs);
	if( focusid < 0 ){
		dbg_warning("no id");
		return -1;
	}
	do{
		++focusid;
		if( focusid >= childs ) focusid = 0;

	}while( parent->childs[focusid]->focusable < 1 );
	dbg_info("focus id: %d", focusid);
	return focusid;
}

void gui_focus_next(gui_s* gui){
	iassert(gui);
	gui_focus_from_parent(gui->parent, gui_focus_next_id(gui->parent));
}

int gui_focus_prev_id(gui_s* parent){
	if( !parent ) return -1;
	int focusid = parent->childFocus;
	int childs = vector_count(parent->childs);
	if( focusid < 0 ) return -1;
	do{
		if( focusid == 0 ) focusid = childs-1;
		else --focusid;
	}while( parent->childs[focusid]->focusable < 1 );
	return focusid;
}

void gui_focus_prev(gui_s* gui){
	iassert(gui);
	gui_focus_from_parent(gui->parent, gui_focus_prev_id(gui->parent));
}

void gui_draw(gui_s* gui){
	if( gui->draw ) gui->draw(gui, NULL);
}

void gui_redraw(gui_s* gui){
	if( gui->redraw ) gui->redraw(gui, NULL);
}

void gui_opacity(gui_s* gui, double op){
	unsigned lop = (unsigned)0xFFFFFFFF * op;
	xorg_win_opacity_set(X, gui->id, lop);;
}

void gui_round_unset(gui_s* gui){
	if( !gui->parent ){
		xcb_window_t win = xorg_parent(X,gui->id);
		if( win ) xorg_win_round_remove(X, win);
	}
	xorg_win_round_remove(X, gui->id);
}

void gui_round_set(gui_s* gui, int radius){
	if( !gui->parent ){
		xcb_window_t win = xorg_parent(X,gui->id);
		if( win ) xorg_win_round_border(X, win, gui->position.w, gui->position.h, radius);
	}
	xorg_win_round_border(X, gui->id, gui->position.w, gui->position.h, radius);
}

void gui_round_antialiasing_set(gui_s* gui, int radius){
	g2dImage_s* mask = g2d_copy(gui->surface->img);

	g2dColor_t col = gui_color(0, 0, 0, 0);
	
	g2dPoint_s cx;
	g2dCoord_s rc;

	cx.x = radius;
	cx.y = radius;
	g2d_circle(mask, &cx, radius, col, 1);
	g2d_circle_fill(mask, &cx, radius, col);

	cx.x = (mask->w) - radius;
	g2d_circle(mask, &cx, radius, col, 1);
	g2d_circle_fill(mask, &cx, radius, col);

	cx.y = (mask->h) - radius;
	g2d_circle(mask, &cx, radius, col, 1);
	g2d_circle_fill(mask, &cx, radius, col);

	cx.x = radius;
	g2d_circle(mask, &cx, radius, col, 1);
	g2d_circle_fill(mask, &cx, radius, col);

	rc.x = radius + 2;
	rc.y = 0;
	rc.w = mask->w - radius*2;
	rc.h = mask->h;
	g2d_clear(mask, col, &rc);

	rc.x = 0;
	rc.y = radius+2;
	rc.w = mask->w;
	rc.h = mask->h - radius*2;
	g2d_clear(mask, col, &rc);

	rc.x=0;
	rc.y=0;
	rc.w=mask->w;
	rc.h=mask->h;
	g2d_bitblt_xor(gui->surface->img,&rc,mask, &rc);

	g2d_free(mask);
}

void gui_remove_decoration(gui_s* gui){
	if( gui->parent ) return;
	xorg_win_decoration_remove(X, gui->id);
}

int gui_event_redraw(gui_s* gui, __unused xorgEvent_s* unset){
	gui_background_redraw(gui, gui->background[0]);
	return 0;
}

int gui_event_draw(gui_s* gui, __unused xorgEvent_s* evdamage){
	xorg_win_surface_redraw(X, gui->id, gui->surface);
	return 0;
}

__private int gui_search_childfocus(gui_s* gui){
	if( gui->childFocus >=0 ){
		gui_focus_from_parent(gui, gui->childFocus);
		return 1;
	}

	vector_foreach(gui->childs, i){
		if( gui_search_childfocus(gui->childs[i]) ) return 1;
	}
	return 0;
}

int gui_event_focus(gui_s* gui, xorgEvent_s* event){
	if( event->focus.outin) gui_search_childfocus(gui_main_parent(gui));
	return 0;
}

__private void redraw_all(gui_s* gui){
	if( gui->redraw ) gui->redraw(gui, NULL);
	if( gui->draw ) gui->draw(gui, NULL);
	vector_foreach(gui->childs, i){
		redraw_all(gui->childs[i]);
	}
}

int gui_event_move(gui_s* gui, xorgEvent_s* event){
	iassert( event->type == XORG_EVENT_MOVE );
	dbg_info("move event");
	if( gui->surface->img->w != event->move.w || gui->surface->img->h != event->move.h ){
		dbg_info("resize surface, redraw && draw");
		xorg_surface_resize(X, gui->surface, event->move.w, event->move.h);
		redraw_all(gui);
	}
	gui->position.x = event->move.x;
	gui->position.y = event->move.y;
	gui->position.w = event->move.w;
	gui->position.h = event->move.h;

	return 0;
}

int gui_event_key(gui_s* gui, xorgEvent_s* event){
	if( event->keyboard.event != XORG_KEY_RELEASE ) return 0;

	switch( event->keyboard.keysym ){
		case XKB_KEY_Left:
			gui_focus_next(gui);
		break;

		case XKB_KEY_Right:
			gui_focus_next(gui);
		break;

		case XKB_KEY_Tab:
			gui_focus_next(gui);
		break;
	}

	return 0;
}

xorgEvent_s* gui_event_get(int async){
	xorgEvent_s* event = xorg_event_new(X, async);
	if( event ){
		gui_s* gr = allgui_find(event->win);
		if( !gr ){
			dbg_warning("unknow window id");
			xorg_event_free(event);
			return NULL;
		}
		event->userdata = gr;
	}
	return event;
}

void gui_event_release(xorgEvent_s* ev){
	if( ev ) xorg_event_free(ev);
}

int gui_event_call(xorgEvent_s* ev){
	if( !ev ) return 0;
	gui_s* gui = ev->userdata;
	iassert(gui);

	dbg_info("event for id %u name:%s", gui->id, gui->name);

	switch( ev->type ){
		case XORG_EVENT_CREATE:         if( gui->create )  return gui->create(gui,ev);  break;
		case XORG_EVENT_DESTROY:        if( gui->destroy ) return gui->destroy(gui,ev); break;
		case XORG_EVENT_DRAW:           if( gui->draw )    return gui->draw(gui,ev);    break;
		case XORG_EVENT_KEY_PRESS:      if( gui->key )     return gui->key(gui,ev);     break;
		case XORG_EVENT_KEY_RELEASE:    if( gui->key )     return gui->key(gui,ev);     break;
		case XORG_EVENT_BUTTON_PRESS:   if( gui->mouse )   return gui->mouse(gui,ev);   break;
		case XORG_EVENT_BUTTON_RELEASE: if( gui->mouse )   return gui->mouse(gui,ev);   break;
		case XORG_EVENT_MOTION:         if( gui->mouse )   return gui->mouse(gui,ev);   break;
		case XORG_EVENT_ENTER:          if( gui->mouse )   return gui->mouse(gui,ev);   break;
		case XORG_EVENT_LEAVE:          if( gui->mouse )   return gui->mouse(gui,ev);   break;
		case XORG_EVENT_FOCUS_IN:       if( gui->focus )   return gui->focus(gui,ev);   break;
		case XORG_EVENT_FOCUS_OUT:      if( gui->focus )   return gui->focus(gui,ev);   break;
		case XORG_EVENT_MAP:            if( gui->map )     return gui->map(gui,ev);     break;
		case XORG_EVENT_UNMAP:          if( gui->map )     return gui->map(gui,ev);     break;
		case XORG_EVENT_MOVE:           if( gui->move )    return gui->move(gui,ev);    break;
		case XORG_EVENT_ATOM:           if( gui->atom )    return gui->atom(gui,ev);    break;
		case XORG_EVENT_CLIENT:         if( gui->client )  return gui->client(gui,ev);  break;
	}
	dbg_warning("not event called");
	return 0;
}

err_t gui_deadpoll_event_callback(__unused deadpoll_s* dp, __unused int ev, __unused void* arg){
	xorgEvent_s* event;
	int ret = 0;
   	while( (event = gui_event_get(1)) ){
		ret	|= gui_event_call(event);
		gui_event_release(event);
	}
	xorg_client_flush(X);
	return ret;
}

void gui_deadpoll_unregister(deadpoll_s* dp){
	deadpoll_unregister(dp, xorg_fd(X));
}

void gui_deadpoll_register(deadpoll_s* dp){
	deadpoll_register(dp, xorg_fd(X), gui_deadpoll_event_callback, NULL, 0, NULL);
}

__private void gui_timer_timeout(guiTimer_s* timer){
	iassert( timer->fn );
	switch( timer->fn(timer) ){
		case GUI_TIMER_CUSTOM: break;
		case GUI_TIMER_FREE: 
			gui_timer_free(timer);
		break;
		default: case GUI_TIMER_NEXT:
			timer->raisedon = time_ms() + timer->ms;
			phq_change_priority(timergui, timer->raisedon, timer->el);
		break;
	}
}

int gui_deadpoll_event(deadpoll_s* dp){
	long timems = -1;

	phqElement_s* el = phq_peek(timergui);
	if( el ){
		dbg_info("timer peek");
		guiTimer_s* timer = el->data;
		timems = timer->raisedon - time_ms();
		if( timems <= 0 ){
			dbg_info("timer overflow");
			gui_timer_timeout(timer);
			return 1;
		}
	}
	dbg_info("check event, timeout %ld", timems);
	int ret = deadpoll_event(dp, &timems);
	if( ret == DEADPOLL_TIMEOUT && el ){
		dbg_info("timeout");
		gui_timer_timeout(el->data);
		return 1;
	}
	if( ret == DEADPOLL_EVENT ){
		dbg_info("event end");
		return 1;
	}
	dbg_error("deadpoll");
	return -1;
}

void gui_loop(void){
	xorg_client_flush(X);
	while( gui_deadpoll_event(dpgui) > 0 ) xorg_client_flush(X);
}

guiTimer_s* gui_timer_new(gui_s* gui, size_t ms, guiTimer_f fn, void* userdata){
	guiTimer_s* timer = mem_new(guiTimer_s);
	timer->raisedon = time_ms() + ms;
	timer->ms = ms;
	timer->gui = gui;
	timer->fn = fn;
	timer->userdata = userdata;
	timer->el = phq_element_new(timer->raisedon, timer, NULL);
	if( !timer->el ) err_fail("eom");
	if( phq_insert(timergui, timer->el) ){
		err_fail("insert timer");
	}
	return timer;
}

int gui_timer_change(guiTimer_s* timer, size_t ms){
	if( timer->ms == ms ) return GUI_TIMER_NEXT;
	timer->raisedon += (ssize_t)ms - (ssize_t)timer->ms;
	phq_change_priority(timergui, timer->raisedon, timer->el);
	return GUI_TIMER_CUSTOM;
}

void gui_timer_free(guiTimer_s* timer){
	phq_remove(timergui, timer->el);
	phq_element_free(timer->el);
	free(timer);
}

guiBackground_s* gui_background_new(g2dColor_t color, g2dImage_s* img, g2dCoord_s* pos, guiBackgroundFN_f fn, int mode){
	guiBackground_s* bk = mem_new(guiBackground_s);
	if( !bk ) err_fail("eom");
	bk->color = color;
	bk->img = img;
	bk->mode = mode;
	bk->fn = fn;
	if( pos ){
		bk->pdest = *pos;
	}
	else if ( img ){
		bk->pdest.x = 0;
		bk->pdest.y = 0;
		bk->pdest.w = img->w;
		bk->pdest.h = img->h;
	}
	else{
		bk->pdest.x = 0;
		bk->pdest.y = 0;
		bk->pdest.w = 0;
		bk->pdest.h = 0;
	}
	
	return bk;
}

void gui_background_redraw(gui_s* gui, guiBackground_s* bkg){
	if( bkg->mode == GUI_BK_NO_OP ) return;

	if( bkg->mode & GUI_BK_COLOR ){
		g2dCoord_s origin;
		if( bkg->mode & GUI_BK_CPOS ){
			dbg_info("redraw bk positional mode");
			origin = bkg->pdest;
		}
		else{
			dbg_info("redraw bk");
			origin.x = 0;
			origin.y = 0;
			origin.w = gui->surface->img->w;
			origin.h = gui->surface->img->h;
		}
		g2d_clear(gui->surface->img, bkg->color, &origin);
	}

	if( bkg->mode & GUI_BK_IMAGE ){
		dbg_info("redraw img");
		g2dImage_s* rs = NULL;
		g2dCoord_s src = { .x = 0, .y = 0, .w = bkg->img->w, .h = bkg->img->h};

		if( bkg->img->w !=  bkg->pdest.w || bkg->img->h != bkg->pdest.h ){
			dbg_info("resize src img because != pdest");
			rs = g2d_resize(bkg->img, bkg->pdest.w, bkg->pdest.h);
			src.w = bkg->pdest.w;
			src.h = bkg->pdest.h;
		}
		
		if( bkg->mode & GUI_BK_ALPHA ){
			g2d_bitblt_alpha(gui->surface->img, &bkg->pdest, rs ? rs : bkg->img, &src);
		}
		else{
			g2d_bitblt(gui->surface->img, &bkg->pdest, rs ? rs : bkg->img, &src);
		}
	}
	
	if( bkg->mode & GUI_BK_FN ){
		if( bkg->fn ) bkg->fn(gui);
	}
}

guiBackground_s* gui_background_get(gui_s* gui, size_t id){
	if( id > vector_count(gui->background) ) return NULL;
	return gui->background[id];
}

void gui_background_add(gui_s* gui, guiBackground_s* bk){
	vector_push_back(gui->background, bk);
}

void gui_background_main_round_fn(gui_s* gui){
	gui_round_antialiasing_set(gui, gui->genericSize);
}

void gui_background_round_fn(gui_s* gui){
	iassert(gui->parent);
	unsigned radius = gui->genericSize;

	g2dImage_s* orig = g2d_copy(gui->surface->img);
	g2dImage_s* mask = g2d_new(gui->surface->img->w, gui->surface->img->h, -1);
	
	g2dPoint_s cx;
	g2dCoord_s rc,mc;
	g2dColor_t col = gui_color(0, 0, 0, 0);

	rc.x = 0;
	rc.y = 0;
	rc.w = mask->w;
	rc.h = mask->h;
	g2d_clear(mask, col, &rc); 

	mc.x = gui->position.x;
	mc.y = gui->position.y;
	mc.w = mask->w;
	mc.h = mask->h;
	g2d_bitblt(gui->surface->img, &rc, gui->parent->surface->img, &mc);

	col = gui_color(255, 0, 0, 0);

	cx.x = radius;
	cx.y = radius;
	g2d_circle(mask, &cx, radius, col, 1);
	g2d_circle_fill(mask, &cx, radius, col);

	cx.x = (mask->w) - radius;
	g2d_circle(mask, &cx, radius, col, 1);
	g2d_circle_fill(mask, &cx, radius, col);

	cx.y = (mask->h) - radius;
	g2d_circle(mask, &cx, radius, col, 1);
	g2d_circle_fill(mask, &cx, radius, col);

	cx.x = radius;
	g2d_circle(mask, &cx, radius, col, 1);
	g2d_circle_fill(mask, &cx, radius, col);

	rc.x = radius + 2;
	rc.y = 0;
	rc.w = mask->w - radius*2;
	rc.h = mask->h;
	g2d_clear(mask, col, &rc);

	rc.x = 0;
	rc.y = radius+2;
	rc.w = mask->w;
	rc.h = mask->h - radius*2;
	g2d_clear(mask, col, &rc);

	rc.x=0;
	rc.y=0;
	rc.w=mask->w;
	rc.h=mask->h;

	g2d_bitblt_channel(orig, &rc, mask, &rc, mask->ma);
	g2d_bitblt_alpha(gui->surface->img, &rc, orig, &rc);

	g2d_free(mask);
	g2d_free(orig);
}















