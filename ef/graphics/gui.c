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

#define GUI_CHILD_INITIAL (8)
#define GUI_KEY_SIZE (sizeof(uint32_t))
#define GUI_HASH_SIZE 32
#define GUI_HASH_MIN 10
#define GUI_TIMERS_SIZE 16

__private rbhash_s* allgui;
__private xorg_s* X;
__private phq_s* timergui;

__private unsigned nohash_window(const char* name, __unused size_t len){
	return *(uint32_t*)name;
}

void gui_begin(){
	os_begin();
	ft_begin();
	gui_resources_init();
	allgui = rbhash_new(GUI_HASH_SIZE, GUI_HASH_MIN, GUI_KEY_SIZE, nohash_window, NULL);
	if( !allgui ) err_fail("on init allgui");
	timergui = phq_new(GUI_TIMERS_SIZE, GUI_TIMERS_SIZE, phq_cmp_asc);
	if( !timergui ) err_fail("creating timer gui");
	X = xorg_client_new(NULL, 0);
	
}

void gui_end(){
	ft_end();
	rbhash_free(allgui);
	phq_free(timergui);
	xorg_client_free(X);
	gui_resources_free();
}

__private void allgui_add(gui_s* gui){
	if( rbhash_add(allgui, (char*)&gui->id, GUI_KEY_SIZE, gui) ){
		err_fail("add gui %d::%s::%s on allgui", (uint32_t)gui->id, gui->name, gui->class);
	}
}

__private void allgui_remove(gui_s* gui){
	if( rbhash_remove(allgui, (char*)&gui->id, GUI_KEY_SIZE) ){
		err_fail("add gui %d::%s::%s on allgui", (uint32_t)gui->id, gui->name, gui->class);
	}
}

__private gui_s* allgui_find(xcb_window_t id){
	return rbhash_find(allgui, (char*)&id, GUI_KEY_SIZE);
}

void gui_register_root_event(void){
	xorg_register_events(X, xorg_root(X), XCB_EVENT_MASK_PROPERTY_CHANGE);
}

gui_s* gui_new(
		gui_s* parent, 
		const char* name, const char* class, 
		int border, int x, int y, int width, int height, 
		g2dColor_t color, 
		void* control, void* userdata)
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
	gui->control = control;
	gui->userdata = userdata;
	gui->create = NULL;
	gui->destroy = NULL;
	gui->free = NULL;
	gui->redraw = gui_event_redraw;
	gui->draw = gui_event_draw;
	gui->key = NULL;
	gui->mouse = NULL;
	gui->focus = NULL;
	gui->map = NULL;
	gui->move = gui_event_move;
	gui->atom = NULL;
	gui->client = NULL;
	gui->position.x = x;
	gui->position.y = y;
	gui->position.w = width;
	gui->position.h = height;
	gui->background.color = color;
	gui->background.img = NULL;
	gui->background.mode = GUI_BK_COLOR;
	gui->surface = NULL;

	gui->id = xorg_win_new(&gui->surface, X, xcbParent, &gui->position, border, gui->background.color);
	gui_name(gui, name);
	gui_class(gui, class);
	gui->redraw(gui, NULL);
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

void gui_name(gui_s* gui, const char* name){
	if( gui->name ) free(gui->name);
	gui->name = str_dup(name, 0);
	xorg_win_title(X, gui->id, gui->name);
}

void gui_class(gui_s* gui, const char* class){
	if( gui->class ) free(gui->class);
	gui->class = str_dup(class, 0);
	xorg_win_class(X, gui->id, gui->name);
}

void gui_show(gui_s* gui, int show){
	if( show && gui->draw ) gui->draw(gui, NULL);
	xorg_win_show(X, gui->id, show);
}

void gui_move(gui_s* gui, int x, int y){
	xorg_win_move(X, gui->id, x, y);
}

void gui_resize(gui_s* gui, int w, int h){
	xorg_win_resize(X, gui->id, w, h);
}

void gui_focus(gui_s* gui){
	xorg_win_focus(X, gui->id);
}

int gui_event_redraw(gui_s* gui, __unused xorgEvent_s* unset){
	if( gui->background.mode == GUI_BK_NO_OP ) return 0;

	if( gui->background.mode & GUI_BK_COLOR ){
		g2dCoord_s origin;
		origin.x = 0;
		origin.y = 0;
		origin.w = gui->surface->img->w;
		origin.h = gui->surface->img->h;
		g2d_clear(gui->surface->img, gui->background.color, &origin);
	}

	if( gui->background.mode & GUI_BK_IMAGE ){
		if( gui->surface->img->w != gui->background.img->w || gui->surface->img->h != gui->background.img->h ){
			g2d_resize_to(gui->surface->img, gui->background.img);
		}
		else{
			g2dCoord_s src = { .x = 0, .y = 0, .w = gui->background.img->w, .h = gui->background.img->h};
			g2dCoord_s dst = { .x = 0, .y = 0, .w = gui->surface->img->w, .h = gui->surface->img->h};
			if( gui->background.mode & GUI_BK_ALPHA ){
				g2d_bitblt_alpha(gui->surface->img, &dst, gui->background.img, &src);
			}
		}
	}

	return 0;
}

int gui_event_draw(gui_s* gui, __unused xorgEvent_s* evdamage){
	xorg_win_surface_redraw(X, gui->id, gui->surface);
	return 0;
}

int gui_event_move(gui_s* gui, xorgEvent_s* event){
	iassert( event->type == XORG_EVENT_MOVE );
	if( gui->surface->img->w != event->move.coord.w || gui->surface->img->h != event->move.coord.h ){
		xorg_surface_resize(gui->surface, event->move.coord.w, event->move.coord.h);
		if( gui->redraw ) gui->redraw(gui, NULL);
		if( gui->draw ) gui->draw(gui, NULL);
	}
	gui->position = event->move.coord;
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
	switch( timer->fn(timer) ){
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
		guiTimer_s* timer = el->data;
		timems = timer->raisedon - time_ms();
		if( timems <= 0 ){
			gui_timer_timeout(timer);
			return 1;
		}
	}
	int ret = deadpoll_event(dp, &timems);
	if( ret == DEADPOLL_TIMEOUT && el ){
		gui_timer_timeout(el->data);
		return 1;
	}
	if( ret == DEADPOLL_EVENT ) return 1;
	return -1;
}

void gui_loop(void){
	deadpoll_s* dp = deadpoll_new();
	if( !dp ) err_fail("deadpoll");

	gui_deadpoll_register(dp);
	while( gui_deadpoll_event(dp) > 0 );
	gui_deadpoll_unregister(dp);
	deadpoll_free(dp);
}

guiTimer_s* gui_timer_new(gui_s* gui, size_t ms, void* userdata){
	guiTimer_s* timer = mem_new(guiTimer_s);
	timer->raisedon = time_ms() + ms;
	timer->ms = ms;
	timer->gui = gui;
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











