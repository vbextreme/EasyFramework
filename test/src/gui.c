#include "test.h"
#include <ef/ft.h>
#include <ef/image.h>
#include <ef/imageFiles.h>
#include <ef/xorg.h>
#include <ef/os.h>
#include <ef/utf8.h>
#include <ef/deadpoll.h>

/*@test -g --gui 'test gui'*/

typedef void (*imgDraw_f)(g2dImage_s* img);

typedef struct xwin{
	xorg_s* x;
	xorgCallbackEvent_s xce;
	xorgSurface_s* surf;
	xcb_window_t id;
	imgDraw_f draw;
	struct xwin* child;
}xwin_s;

__private void x_def_key(xorgKeyboard_s* k){
	xwin_s* win = k->user;

	if( *k->utf8 == '\x1B' ){
		dbg_info("destroy simple app");
		xorg_send_destroy(k->x, win->id);
	}	
}

__private void x_def_move(xorgMove_s* move){
	xwin_s* win = move->user;
	if( move->coord.w != win->surf->img->w || move->coord.h != win->surf->img->h)
	   	xorg_surface_resize(win->surf, move->coord.w, move->coord.h);
	win->draw(win->surf->img);
}

__private void x_def_draw(__unused xorg_s* x, void* user, __unused g2dCoord_s* damaged){
	xwin_s* win = user;
	xorg_win_surface_redraw(win->x, win->id, win->surf);
}

void simple_draw(g2dImage_s* img){
	g2dColor_t bkcol = g2d_color_gen(X_COLOR_MODE, 255, 125,125,125); 
	g2dCoord_s gw = { .x = 0, .y = 0, .w = img->w, .h = img->h };
	g2d_clear(img, bkcol, &gw);
}

void simple_win(xwin_s* win){
	g2dCoord_s pos = { 
		.x = 100,
		.y = 100,
		.w = 800,
		.h = 600,
	};
	g2dColor_t bkcol = g2d_color_gen(X_COLOR_MODE, 255, 125,125,125); 
	win->id = xorg_win_new(&win->surf, win->x, xorg_root(win->x), &pos, 1, bkcol);
	win->draw = simple_draw;

	win->child = mem_new(xwin_s);
	pos.x = 10;
	pos.y = 10;
	pos.w = 80;
	pos.h = 60;
	bkcol = g2d_color_gen(X_COLOR_MODE, 255, 25,200,25); 
	win->child->id = xorg_win_new(&win->child->surf, win->x, win->id, &pos, 1, bkcol);
	win->child->draw = simple_draw;

	xorg_win_show(win->x, win->id, 1);
	xorg_win_show(win->x, win->child->id, 1);
}

__private err_t x_events(__unused deadpoll_s* dp, __unused int ev, void* arg){
	xwin_s* win = arg;
	
	xcb_generic_event_t* event;
	while( (event=xorg_event_get(win->x, 1)) ){
		if( xorg_parse_event(event, win->x, &win->xce) ) return -1;	
	}
	return 0;
}

/*@fn*/
void test_gui(__unused const char* argA, __unused const char* argB){
	utf_begin();
	os_begin();	
	ft_begin();
	
	deadpoll_s* dp = deadpoll_new();

	xorg_s* x = xorg_client_new(NULL, 0);
	xorg_register_events(x, xorg_root(x), XCB_EVENT_MASK_PROPERTY_CHANGE);
	
	xwin_s win = {0};
	win.xce.user = &win;
	win.xce.keyboard = x_def_key;
	win.xce.redraw = x_def_draw;
	win.xce.move = x_def_move;
	win.x = x;
	simple_win(&win);
	
	deadpoll_register(dp, xorg_fd(x), x_events, &win, 0, NULL);
	deadpoll_loop(dp, -1);
	deadpoll_free(dp);
	
	ft_end();
	xorg_surface_destroy(x, win.surf);
	xorg_client_free(x);
}

