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
#include <ef/imageFiles.h>
#include <ef/file.h>
#include <ef/guiResources.h>
#include <ef/guiImage.h>
#include <ef/spawn.h>

#define GUI_CHILD_INITIAL   8
#define GUI_KEY_SIZE        32
#define GUI_HASH_SIZE       32
#define GUI_HASH_MIN        10
#define GUI_TIMERS_SIZE     16
#define GUI_BACKGROUND_SIZE 3

typedef struct guiInternalFocusEvent{
	gui_s* gui;
	guiEvent_f fn;
}guiInternalFocusEvent_s;

typedef struct guiInternalFd{
	gui_s* gui;
	guiEvent_f fn;
	int fd;
}guiInternalFd_s;

__private rbhash_s* allgui;
__private xorg_s* X;
__private phq_s* timergui;
__private deadpoll_s* dpgui;
__private gui_s* focused;
__private guiInternalFocusEvent_s* vgife;
__private const char* oldlocale;

void gui_begin(){
	oldlocale = os_setlocale(LC_NUMERIC, "C");
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
	vgife = vector_new(guiInternalFocusEvent_s, 2, NULL);
}

void gui_end(){
	ft_end();
	rbhash_free(allgui);
	phq_free(timergui);
	xorg_client_free(X);
	gui_resources_free();
	deadpoll_free(dpgui);
	vector_free(vgife);
	os_setlocale(LC_NUMERIC, oldlocale);
}

__private void allgui_add(gui_s* gui){
	dbg_info("register gui(%u): %s", gui->id, gui->name);
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

unsigned gui_screen_width(void){
	return xorg_root_width(X);
}

unsigned gui_screen_height(void){
	return xorg_root_height(X);
}

void gui_register_internal_focus_event(gui_s* gui, guiEvent_f fn){
	guiInternalFocusEvent_s* ev = vector_get_push_back(vgife);
	ev->gui = gui;
	ev->fn = fn;
}

void gui_unregister_internal_focus_event(gui_s* gui){
	vector_foreach(vgife, i){
		if( vgife[i].gui == gui ){
			vector_remove(vgife, i);
			break;
		}
	}
}

void gui_internal_focus_event(void){
	vector_foreach(vgife, i){
		vgife[i].fn(vgife[i].gui, NULL);
	}
}

gui_s* gui_new(
		gui_s* parent, 
		const char* name, const char* class, guiMode_e mode, 
		int border, int x, int y, int width, int height, 
		g2dColor_t colorBorder, guiComposite_s* img,
		int genericSize, void* userdata)
{
	gui_s* gui = mem_new(gui_s);
	if( !gui ) err_fail("malloc");
	
	gui->name = NULL;
	gui->class = NULL;
	gui->parent = parent;
	gui->childs = vector_new(gui_s*, GUI_CHILD_INITIAL, NULL);
	xcb_window_t xcbParent = xorg_root(X);
	if(parent){
		gui_child_add(parent, gui);
		xcbParent = parent->id;
	}

	gui->userdata = userdata;
	gui->userMargin.left = gui->userMargin.right = gui->userMargin.top = gui->userMargin.bottom = 0;
	gui->control = NULL;
	gui->create = NULL;
	gui->destroy = NULL;
	gui->free = NULL;
	gui->map = NULL;
	gui->atom = NULL;
	gui->client = NULL;
	gui->clipboard = NULL;
	gui->themes = NULL;
	gui->redraw = gui_event_redraw;
	gui->draw = gui_event_draw;
	gui->key = gui->parent ? gui->parent->key :  NULL;
	gui->focus = !gui->parent ? gui_event_focus : NULL;
	gui->mouse = gui->parent ? gui_event_mouse : NULL;
	gui->move = gui_event_move;

	gui->position.x = x;
	gui->position.y = y;
	gui->position.w = width;
	gui->position.h = height;
	gui->img = img;
	
	gui->surface = NULL;
	gui->type = GUI_TYPE_WINDOW;
	gui->focusable = 1;
	gui->bordersize = border;
	gui->bordersizefocused = border + GUI_FOCUS_BORDER_SIZE;
	gui->borderColor = colorBorder;
	gui->id = xorg_win_new(&gui->surface, X, xcbParent, x, y, width, height, border, colorBorder, colorBorder);
	gui_name(gui, name);
	gui_class(gui, class);
	gui->genericSize = genericSize;

	allgui_add(gui);

	switch( mode ){
		case GUI_MODE_MODAL:
			xorg_win_type_set(X, gui->id, XORG_WINDOW_TYPE_DIALOG);
			xorg_win_state_set(X, gui->id, XORG_WINDOW_STATE_MODAL);
			xorg_win_action_set(X, gui->id, XORG_WINDOW_ACTION_CLOSE | XORG_WINDOW_ACTION_MOVE);
			xorg_win_set_top(X, xorg_parent(X, gui->id), gui->id, 1);
		break;

		case GUI_MODE_DOCK_TOP:
			xorg_win_type_set(X, gui->id, XORG_WINDOW_TYPE_DOCK);
			xorg_wm_reserve_dock_space_on_top(X, gui->id, gui->position.x, gui->position.w, gui->position.h);
		break;

		case GUI_MODE_DOCK_BOTTOM:
			xorg_win_type_set(X, gui->id, XORG_WINDOW_TYPE_DOCK);
			xorg_wm_reserve_dock_space_on_bottom(X, gui->id, gui->position.x, gui->position.w, gui->position.h);
		break;

		case GUI_MODE_DOCK_LEFT:
			xorg_win_type_set(X, gui->id, XORG_WINDOW_TYPE_DOCK);
			xorg_wm_reserve_dock_space_on_left(X, gui->id, gui->position.y, gui->position.w, gui->position.h);
		break;

		case GUI_MODE_DOCK_RIGHT:
			xorg_win_type_set(X, gui->id, XORG_WINDOW_TYPE_DOCK);
			xorg_wm_reserve_dock_space_on_right(X, gui->id, gui->position.y, gui->position.w, gui->position.h);
		break;

		default: case GUI_MODE_NORMAL: break;
	}

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
	gui_composite_free(gui->img);
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
	if( x != gui->position.x || y != gui->position.y )
		xorg_win_move(X, gui->id, x, y);
}

/* event move is raised*/
void gui_resize(gui_s* gui, int w, int h){
	if( (unsigned)w != gui->position.w || (unsigned)h != gui->position.h ){
		xorg_win_resize(X, gui->id, w, h);
	}
}

void gui_border(gui_s* gui, int border){
	dbg_info("%s border change: %d -> %d", gui->name, gui->bordersize, border);
	gui->bordersize = border;
	xorg_win_border(X, gui->id, border);
}

ssize_t gui_id(gui_s* gui){
	if( !gui->parent ) return -1;
	vector_foreach(gui->parent->childs, i){
	if( gui->parent->childs[i] == gui )
		return i;
	}
	return -1;	
}

gui_s* gui_by_name(gui_s* gui, const char* name, const char* class){
	if( !strcmp(gui->name, name) && !strcmp(gui->class, class) ){
		return gui;
	}

	gui_s* ret;
	vector_foreach(gui->childs, i){
		if( (ret=gui_by_name(gui->childs[i], name, class)) ) return ret;
	}
	return NULL;
}

int gui_focus_have(gui_s* gui){
	return gui == focused;
}

void gui_focus(gui_s* gui){
	dbg_info("set focus: %s", gui->name);
	focused = gui;
	gui_internal_focus_event();
	xorg_win_focus(X, gui->id);
}

gui_s* gui_focus_next(gui_s* gui){
	gui_s* parent = gui->parent;
	if( !parent ){
		dbg_warning("no parent");
		return NULL;
	}
	int focusid = gui_id(gui);
	int childs = vector_count(parent->childs);
	if( focusid < 0 ){
		dbg_warning("no id");
		return NULL;
	}
	do{
		++focusid;
		if( focusid >= childs ) focusid = 0;
	}while( parent->childs[focusid]->focusable < 1 );
	gui_focus(parent->childs[focusid]);
	return parent->childs[focusid];
}

gui_s* gui_focus_prev(gui_s* gui){
	gui_s* parent = gui->parent;
	if( !parent ){
		dbg_error("no parent");
		return NULL;
	}
	int focusid = gui_id(gui);
	int childs = vector_count(parent->childs);
	if( focusid < 0 ){
		dbg_error("parent no childfocus");
		return NULL;
	}
	do{
		if( focusid == 0 ) focusid = childs-1;
		else --focusid;
	}while( parent->childs[focusid]->focusable < 1 );
	gui_focus(parent->childs[focusid]);
	return parent->childs[focusid];
}

void gui_clipboard_copy(gui_s* gui, int primary){
	if( primary ){
		xorg_clipboard_copy(X, gui->id, X->atom[XORG_ATOM_PRIMARY]);
	}
	else{
		xorg_clipboard_copy(X, gui->id, X->atom[XORG_ATOM_CLIPBOARD]);
	}
}

void gui_clipboard_paste(gui_s* gui, int primary){
	if( primary ){
		xorg_clipboard_paste(X, gui->id, X->atom[XORG_ATOM_PRIMARY]);
	}
	else{
		xorg_clipboard_paste(X, gui->id, X->atom[XORG_ATOM_CLIPBOARD]);
	}
}

void gui_clipboard_send(xorgClipboard_s* clipboard, void* data, size_t size){
	xorg_send_copy(X, clipboard, data, size);
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
	g2d_circle_fill_antialiased(mask, &cx, radius, col);

	cx.x = (mask->w) - radius;
	g2d_circle_fill_antialiased(mask, &cx, radius, col);

	cx.y = (mask->h) - radius;
	g2d_circle_fill_antialiased(mask, &cx, radius, col);

	cx.x = radius;
	g2d_circle_fill_antialiased(mask, &cx, radius, col);

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
	gui_composite_redraw(gui, gui->img);
	return 0;
}

int gui_event_draw(gui_s* gui, __unused xorgEvent_s* evdamage){
	xorg_win_surface_redraw(X, gui->id, gui->surface);
	return 0;
}

int gui_event_focus(__unused gui_s* gui, xorgEvent_s* event){
	if( event->focus.outin){
		if( focused )
			gui_focus(focused);
	}
	return 0;
}

int gui_event_mouse(gui_s* gui, xorgEvent_s* event){
	if( 
		(
			event->mouse.event == XORG_MOUSE_RELEASE || 
			event->mouse.event == XORG_MOUSE_CLICK || 
			event->mouse.event == XORG_MOUSE_DBLCLICK
		) 
		&& event->mouse.button == 1 
	){
		if( !gui_focus_have(gui) ){
			gui_focus(gui);
		}
	}

	return 0;
}

int gui_event_move(gui_s* gui, xorgEvent_s* event){
	iassert( event->type == XORG_EVENT_MOVE );
	dbg_info("move event:%s", gui->name);
	if( gui->surface->img->w != event->move.w || gui->surface->img->h != event->move.h ){
		dbg_info("resize composite && surface, redraw && draw");
		gui_composite_resize(gui, gui->img, event->move.w, event->move.h);
		xorg_surface_resize(X, gui->surface, event->move.w, event->move.h);
		gui_redraw(gui);
		gui_draw(gui);
	}

	gui->position.x = event->move.x;
	gui->position.y = event->move.y;
	gui->position.w = event->move.w;
	gui->position.h = event->move.h;

	return 0;
}

int gui_event_key(gui_s* gui, xorgEvent_s* event){
	if( !gui->parent ) return 0;

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
		event->userdata = gr;
	}
	return event;
}

void gui_event_release(xorgEvent_s* ev){
	if( ev ) xorg_event_free(ev);
}

int gui_event_call(xorgEvent_s* ev){
	if( !ev ) return 0;
	if( !ev->userdata ) return 0;
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
		case XORG_EVENT_CLIPBOARD_PASTE:if( gui->clipboard)return gui->clipboard(gui,ev);break;
		case XORG_EVENT_CLIPBOARD_COPY: if( gui->clipboard)return gui->clipboard(gui,ev);break;
	}
	dbg_info("not event called");
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

err_t gui_deadpoll_fd_callback(__unused deadpoll_s* dp, int ev, void* arg){
	guiInternalFd_s* gif = arg;
	xorgEvent_s xev;
	xev.type = XORG_EVENT_USERDATA;
	xev.data.data = &gif->fd;
	xev.data.request = &ev;
	gif->fn(gif->gui, &xev);
	return 0;
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
	while( gui_deadpoll_event(dpgui) > 0 ){ xorg_client_flush(X);}
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

__private void gif_free(void* data){
	pollEvent_s* pe = data;
	free(pe->arg);
}
	
void gui_fd_register(gui_s* gui, int fd, int event, guiEvent_f fn){
	guiInternalFd_s* gif = mem_new(guiInternalFd_s);
	gif->gui = gui;
	gif->fd = fd;
	gif->fn = fn;
	deadpoll_register(dpgui, fd, gui_deadpoll_fd_callback, gif, event, gif_free);
}

void gui_fd_unregister(int fd){
	deadpoll_unregister(dpgui, fd);
}

void gui_background_supersampling_fn(gui_s* gui, __unused guiImage_s** img, __unused void* generic){
	g2d_supersampling_to(gui->surface->img, 1);
}

void gui_background_main_round_fn(gui_s* gui, __unused guiImage_s** img, void* generic){
	guiRound_s* gr = generic;
	gui_round_antialiasing_set(gui, gr->radius);
}

__private void invert_alpha(g2dImage_s* quad){
	for( unsigned y = 0; y < quad->h; ++y){
		unsigned row = g2d_row(quad, y);
		g2dColor_t* pixels = g2d_color(quad, row, 0);
		for( unsigned x = 0; x < quad->w; ++x){
			unsigned alpha = g2d_color_alpha(quad, pixels[x]);
			alpha = 255 - alpha;
			pixels[x] = g2d_color_alpha_set(quad, pixels[x], alpha);
		}
	}
}

void gui_background_round_fn(gui_s* gui, __unused guiImage_s** img, __unused void* generic){
	iassert(gui->parent);
	guiRound_s* ropt = generic;
	
	g2dColor_t empty = gui_color(0,0,0,0);
	g2dColor_t full  = gui_color(255,255,0,0);
	g2dPoint_s cx;
	g2dPoint_s sx,ex;
	g2dCoord_s qcard = { .x = 0, .y = 0, .w = ropt->radius, .h = ropt->radius};
	g2dImage_s* mask = g2d_new(qcard.w, qcard.h, -1);
	g2dImage_s* quad = g2d_new(qcard.w, qcard.h, -1);

	struct roco_s{
		g2dPoint_s cx;
		int incx;
		int incy;
	} roco[4];
	roco[0].cx.x = gui->surface->img->w - ropt->radius;
	roco[0].cx.y = ropt->radius - 1;
	roco[0].incx = -1;
	roco[0].incy = 1;
	roco[1].cx.x = cx.x = gui->surface->img->w - ropt->radius;
	roco[1].cx.y = (gui->surface->img->h - ropt->radius)+1;
	roco[1].incx = -1,
	roco[1].incy = -1,
	roco[2].cx.x = ropt->radius;
	roco[2].cx.y = (gui->surface->img->h - ropt->radius)+1;
	roco[2].incx = 1;
	roco[2].incy = -1;
	roco[3].cx.x = ropt->radius;
	roco[3].cx.y = ropt->radius - 1;
	roco[3].incx = 1;
	roco[3].incy = 1;

	double angle = 0;
	for( unsigned i = 0; i < 4; ++i, angle += 90.0 ){
		for( unsigned r = 0; r < ropt->border; ++r){
			roco[i].cx.x += roco[i].incx;
			roco[i].cx.y += roco[i].incy;
			g2d_arc(gui->surface->img, &roco[i].cx, ropt->radius+r, angle, angle+90.0, gui->borderColor, 0);
		}
	}
	
	sx.x = ropt->radius-1;
	sx.y = 0;
	ex.x = (gui->surface->img->w - ropt->radius)+1;
	ex.y = 0;
	if( ropt->border == 1 ){
		g2d_line(gui->surface->img, &sx, &ex, gui->borderColor, 1);
	}
	else{
		for( unsigned r = 0; r < ropt->border/2 ; ++r){
			g2d_line(gui->surface->img, &sx, &ex, gui->borderColor, 1);
			++sx.y;
			++ex.y;
			--sx.x;
			++ex.x;
		}	
	}

	sx.x = ropt->radius-1;
	sx.y = gui->surface->img->h - 1;
	ex.x = (gui->surface->img->w - ropt->radius)+1;
	ex.y = gui->surface->img->h - 1;
	if( ropt->border == 1 ){
		g2d_line(gui->surface->img, &sx, &ex, gui->borderColor, 1);
	}
	else{
		for( unsigned r = 0; r < ropt->border/2 ; ++r){
			g2d_line(gui->surface->img, &sx, &ex, gui->borderColor, 1);
			++sx.y;
			--ex.y;
			--sx.x;
			--ex.x;
		}	
	}

	sx.x = 1;
	sx.y = ropt->radius-1;
	ex.x = 1;
	ex.y = (gui->surface->img->h - ropt->radius)+1;
	if( ropt->border == 1 ){
		g2d_line(gui->surface->img, &sx, &ex, gui->borderColor, 1);
	}
	else{
		for( unsigned r = 0; r < ropt->border/2 ; ++r){
			g2d_line(gui->surface->img, &sx, &ex, gui->borderColor, 1);
			--sx.y;
			++ex.y;
			++sx.x;
			++ex.x;
		}	
	}

	sx.x = gui->surface->img->w - 2;
	sx.y = ropt->radius-1;
	ex.x = gui->surface->img->w - 2;
	ex.y = (gui->surface->img->h - ropt->radius)+1;
	if( ropt->border == 1 ){
		g2d_line(gui->surface->img, &sx, &ex, gui->borderColor, 1);
	}
	else{
		for( unsigned r = 0; r < ropt->border/2 ; ++r){
			g2d_line(gui->surface->img, &sx, &ex, gui->borderColor, 1);
			--sx.y;
			++ex.y;
			--sx.x;
			--ex.x;
		}	
	}

	struct poco_s{
		g2dPoint_s cx;
		g2dCoord_s co;
	} poco[4] = {
		[0].cx.x = ropt->radius,
		[0].cx.y = ropt->radius-1,
		[0].co.x = 0,
		[0].co.y = 0,
		[0].co.w = qcard.w,
		[0].co.h = qcard.h,

		[1].cx.x = 0,
		[1].cx.y = ropt->radius-1,
		[1].co.x = gui->surface->img->w - ropt->radius,
		[1].co.y = 0,
		[1].co.w = qcard.w,
		[1].co.h = qcard.h,

		[2].cx.x = 0,
		[2].cx.y = 0,
		[2].co.x = gui->surface->img->w - ropt->radius,
		[2].co.y = gui->surface->img->h - ropt->radius,
		[2].co.w = qcard.w,
		[2].co.h = qcard.h,

		[3].cx.x = ropt->radius,
		[3].cx.y = 0,
		[3].co.x = 0,
		[3].co.y = gui->surface->img->h - ropt->radius,
		[3].co.w = qcard.w,
		[3].co.h = qcard.h,

	};

	for( unsigned i = 0; i < 4; ++i ){
		g2d_clear(mask, empty, &qcard);
		g2d_circle_fill_antialiased(mask, &poco[i].cx, ropt->radius, full);
		g2dCoord_s org = {.x = gui->position.x + poco[i].co.x, .y = gui->position.y + poco[i].co.y, .w = qcard.w, .h = qcard.h };
		g2d_bitblt( quad, &qcard, gui->parent->surface->img, &org);
		g2d_bitblt_channel(quad, &qcard, mask, &qcard, quad->ma);
		invert_alpha(quad);
		g2d_bitblt_alpha(gui->surface->img, &poco[i].co, quad, &qcard);
	}


	g2d_free(quad);
	g2d_free(mask);
}

__private char* themes_name(gui_s* gui, char* name){
	if( gui->parent ){
		name = gui_themes_name(gui->parent, name);
	}
	char* n = str_printf("%s.%s", name, gui->name ? gui->name : gui->class);
	free(name);
	return n;
}

char* gui_themes_name(gui_s* gui, const char* appName){
	if( !gui || !appName ) return NULL;
	
	char* n = str_dup(appName, 0);
	return themes_name(gui, n);
}

char* gui_themes_string(const char* name, const char* property){
	char* ret = NULL;
	__mem_free char* p = str_printf("%s.%s", name, property);
	dbg_info("request property:'%s'", p);
	xorg_resources_string_get(X, p, NULL, &ret);
	return ret;
}

err_t gui_themes_bool_set(const char* name, const char* property, int* set){
	bool v;
	__mem_free char* p = str_printf("%s.%s", name, property);
	dbg_info("request property:'%s'", p);
	if( !xorg_resources_bool_get(X, p, NULL, &v) ){
		*set = v;
		return 0;
	}
	return -1;
}

err_t gui_themes_int_set(const char* name, const char* property, int* set){
	long v;
	__mem_free char* p = str_printf("%s.%s", name, property);
	dbg_info("request property:'%s'", p);
	if( !xorg_resources_long_get(X, p, NULL, &v) ){
		*set = v;
		return 0;
	}
	return -1;
}

err_t gui_themes_uint_set(const char* name, const char* property, unsigned* set){
	long v;
	__mem_free char* p = str_printf("%s.%s", name, property);
	dbg_info("request property:'%s'", p);
	if( !xorg_resources_long_get(X, p, NULL, &v) ){
		*set = v;
		return 0;
	}
	return -1;
}

err_t gui_themes_long_set(const char* name, const char* property, long* set){
	long v;
	__mem_free char* p = str_printf("%s.%s", name, property);
	dbg_info("request property:'%s'", p);
	if( !xorg_resources_long_get(X, p, NULL, &v) ){
		*set = v;
		return 0;
	}
	return -1;
}

err_t gui_themes_double_set(const char* name, const char* property, double* set){
	char* dv = gui_themes_string(name, property);
	if( !dv ) return -1;
	*set = strtod(dv,NULL);
	free(dv);
	return 0;
}

err_t gui_themes_fonts_set(const char* name, ftFonts_s** controlFonts){
	int size;
	char* fontname;
	char gtfn[64] = GUI_THEME_FONT_NAME;
	char gtfs[64] = GUI_THEME_FONT_SIZE;
	char* gtfnid = &gtfn[strlen(GUI_THEME_FONT_NAME)];
	char* gtfsid = &gtfs[strlen(GUI_THEME_FONT_SIZE)];
	unsigned id = 0;
	sprintf(gtfnid, "%u", id);
	sprintf(gtfsid, "%u", id++);
	
	dbg_info("name::%s",name);
	__mem_free char* fontref = gui_themes_string(name, GUI_THEME_FONT_GROUP);
	if( !fontref ) return -1;
	guiResource_s* res = gui_resource(fontref);
	if( res ){
		*controlFonts = res->fonts;
		return 0;
	}

	dbg_info("create new fonts:%s", fontref);
	ftFonts_s* fonts = ft_fonts_new(fontref);
	dbg_error("gtfn:%s", gtfn);
	dbg_error("gtfs:%s", gtfs);
   	while( (fontname=gui_themes_string(name, gtfn)) ){
		dbg_info("font name:%s", fontname);
		if( gui_themes_int_set(name, gtfs, &size) ){
			dbg_error("loading size");
			ft_fonts_free(fonts);
			return -1;
		}
		ftFont_s* font = ft_fonts_load(fonts, fontname, fontname);
		if( !font ){
			dbg_error("on loading fonts");
			ft_fonts_free(fonts);
			return -1;
		}
		ft_font_size(font, size, size);
		free(fontname);
		sprintf(gtfnid, "%u", id);
		sprintf(gtfsid, "%u", id++);
		dbg_error("gtfn:%s", gtfn);
		dbg_error("gtfs:%s", gtfs);
	}
	gui_resource_new(fontref, fonts);

	*controlFonts = fonts;

	return 0;
}

err_t gui_themes_gui_image(gui_s* gui, const char* name, guiImage_s** ptrimg){
	char* image = NULL;
	int alpha = 0;
	g2dColor_t color = gui_color(255,0,0,0);
	int colorset = 0;
	int dx = -1;
	int dy = -1;
	int dw = -1;
	int dh = -1;
	double px = 0;
	double py = 0;
	double pw = 0;
	double ph = 0;

	unsigned flags;
	int perenable = 0;
	guiImage_s* img = NULL;

	dbg_error("loading image resources: '%s'", name);

	if( !gui_themes_uint_set(name, GUI_THEME_COMPOSITE_COLOR, &color) ) colorset = 1;
		
	image = gui_themes_string(name, GUI_THEME_COMPOSITE_IMAGE);
	if( !image ){
		image = gui_themes_string(name, GUI_THEME_COMPOSITE_GIF);
		if( !image ){
			image = gui_themes_string(name, GUI_THEME_COMPOSITE_VIDEO);
		}
	}

	if( !colorset && !image ) return -1;
		
	gui_themes_bool_set(name, GUI_THEME_COMPOSITE_ALPHA, &alpha);
	flags = alpha ? GUI_IMAGE_FLAGS_ALPHA : 0;

	gui_themes_int_set(name, GUI_THEME_COMPOSITE_DEST_X, &dx);
	gui_themes_int_set(name, GUI_THEME_COMPOSITE_DEST_Y, &dy);
	gui_themes_int_set(name, GUI_THEME_COMPOSITE_DEST_W, &dw);
	gui_themes_int_set(name, GUI_THEME_COMPOSITE_DEST_H, &dh);
	
	if( 
		!gui_themes_double_set(name, GUI_THEME_COMPOSITE_PER_X, &px) &&
		!gui_themes_double_set(name, GUI_THEME_COMPOSITE_PER_Y, &py) &&
		!gui_themes_double_set(name, GUI_THEME_COMPOSITE_PER_W, &pw) &&
		!gui_themes_double_set(name, GUI_THEME_COMPOSITE_PER_H, &ph)
	){
		perenable = 1;
	}
	
	if( image ){
		int play = 0;
		int loop = 0;
		int ratio = -1;
		int sx = -1;
		int sy = -1;
		int sw = -1;
		int sh = -1;

		gui_themes_bool_set(name, GUI_THEME_COMPOSITE_PLAY, &play);
		if( play ) flags |= GUI_IMAGE_FLAGS_PLAY;
		
		gui_themes_bool_set(name, GUI_THEME_COMPOSITE_LOOP, &loop);
		if( loop ) flags |= GUI_IMAGE_FLAGS_LOOP;

		gui_themes_int_set(name, GUI_THEME_COMPOSITE_RATIO, &ratio);

		gui_themes_int_set(name, GUI_THEME_COMPOSITE_SRC_X, &sx);
		gui_themes_int_set(name, GUI_THEME_COMPOSITE_SRC_Y, &sy);
		gui_themes_int_set(name, GUI_THEME_COMPOSITE_SRC_W, &sw);
		gui_themes_int_set(name, GUI_THEME_COMPOSITE_SRC_H, &sh);

		img = gui_image_load(color, image, sw != -1 ? sw : (int)gui->surface->img->w, sh != -1 ? sh : (int)gui->surface->img->h, flags, ratio);
		if( !img ) err_fail("gui image new");
		if( sx != -1 ) img->src.x = sx;
		if( sy != -1 ) img->src.y = sy;
		if( dx != -1 ) img->pos.x = dx;
		if( dy != -1 ) img->pos.y = dy;
	}
	else{
		img = gui_image_color_new(color, dw != -1 ? dw : (int)gui->surface->img->w, dh != -1 ? dh : (int)gui->surface->img->h, flags);
	}
	if( !img ) return -1;

	if( perenable ){
		gui_image_perc_set(img, px, py, pw, ph);
		gui_image_resize(gui, img, img->src.w, img->src.h, -1);
	}

	if( *ptrimg ){
		gui_image_free(*ptrimg);
	}
	*ptrimg = img;
	return 0;
}

void gui_themes_composite(gui_s* gui, const char* name, const char* compname){
	guiImage_s* img = NULL;

	dbg_info("name:%s composite:%s",name,compname);

	vector_foreach(gui->img->img, i){
		__mem_free char* cname = str_printf("%s.%s.%lu", name, compname, i);
		dbg_info("composite name:%s",cname);
		if( gui_themes_gui_image(gui, cname, &gui->img->img[i]) ) return;
	}
	
	for( size_t i = vector_count(gui->img->img); i < UINT32_MAX; ++i ){
		guiImage_s* newimg = NULL;
		__mem_free char* cname = str_printf("%s.%s.%lu", name, compname, i);
		dbg_info("composite new name:%s",cname);
		if( gui_themes_gui_image(gui, cname, &img) ) return;
		gui_composite_add(gui->img, newimg);
	}
}

void gui_themes(gui_s* gui, const char* appName){
	__mem_free char* name = gui_themes_name(gui, appName);
	dbg_info("gui themes for:%s", name);
	long vlong;
	g2dCoord_s position = {-1,-1,-1,-1};

	if( !gui->parent ){
		char* stralpha = gui_themes_string(name, GUI_THEME_WM_ALPHA);
		if( stralpha ){
			double alpha = strtod(stralpha, NULL);
			free(stralpha);
			gui_opacity(gui, alpha);
		}
	}

	if( !gui_themes_long_set(name, GUI_THEME_BORDER, &vlong) ) gui_border(gui, vlong);
	gui_themes_int_set(name, GUI_THEME_GENERIC, &gui->genericSize);

	gui_themes_uint_set(name, GUI_THEME_X, &position.x);
	gui_themes_uint_set(name, GUI_THEME_Y, &position.y);
	gui_themes_uint_set(name, GUI_THEME_W, &position.w);
	gui_themes_uint_set(name, GUI_THEME_H, &position.h);

	if( (int)position.x != -1 || (int)position.y != -1 ){
		if( (int)position.x == -1 ) position.x = gui->position.x;
		if( (int)position.y == -1 ) position.y = gui->position.y;
		gui_move(gui, position.x, position.y);
	}
	if( (int)position.w != -1 || (int)position.h != -1 ){
		if( (int)position.w == -1 ) position.w = gui->position.w;
		if( (int)position.h == -1 ) position.h = gui->position.h;
		gui_resize(gui, position.w, position.h);
	}

	gui_themes_composite(gui, name, GUI_THEME_COMPOSITE);	

	if( gui->themes ){
		xorgEvent_s ev;
		ev.type = XORG_EVENT_USERDATA;
		ev.data.request = gui->control;
		ev.data.data = name;
		ev.data.size = 0;
		gui->themes(gui, &ev);
	}

	int vbool;
	if( !gui_themes_bool_set(name, GUI_THEME_SUPERSAMPLING, &vbool) && vbool ){
		gui_composite_add(
			gui->img, 
			gui_image_fn_new(
				gui_background_supersampling_fn,
				NULL,
				NULL,
				0,0,0
			)
		);
	}

	unsigned round;
	if( !gui_themes_uint_set(name, GUI_THEME_ROUND, &round) && round > 0 ){
		guiRound_s* gr = mem_new(guiRound_s);
		gr->radius = round;
		guiImageFN_f fn;
		if( gui->parent ){
			gr->border = gui->bordersize;
			fn = gui_background_round_fn;
		}
		else{
			gr->border = 0;
			fn = gui_background_main_round_fn;
		}
		gui_border(gui, 0);

		gui_composite_add(
			gui->img, 
			gui_image_fn_new(
				fn,
				gr,
				free,
				0,0,0
			)
		);
	}
}

void gui_themes_all(gui_s* gui, const char* appName){
	gui_themes(gui, appName);
	vector_foreach(gui->childs, i){
		gui_themes_all(gui->childs[i], appName);
	}
}


