#include "test.h"
#include <ef/ft.h>
#include <ef/image.h>
#include <ef/imageFiles.h>
#include <ef/imageGif.h>
#include <ef/media.h>
#include <ef/xorg.h>
#include <ef/os.h>
#include <ef/utf8.h>
#include <ef/deadpoll.h>
#include <ef/delay.h>

/*@test -g --gui 'test gui'*/

typedef void (*imgDraw_f)(g2dImage_s* img);
typedef int (*xev_f)(xorgEvent_s* ev);

g2dImage_s* bkimg;
gif_s* gif;
media_s* media;

typedef struct xwin{
	xorg_s* x;
	xorgSurface_s* surf;
	xcb_window_t id;
	int state;
	int px,py;
	imgDraw_f redraw;
	xev_f draw;
	xev_f key;
	xev_f move;
	xev_f mouse;
	struct xwin* child;
}xwin_s;

__private int child_key(xorgEvent_s* ev){
	if( *ev->keyboard.utf8 == '\x1B' ){
		dbg_info("destroy simple app");
		xorg_send_destroy(ev->x, ev->win);
	}
	return 0;
}

__private int main_key(xorgEvent_s* ev){
	xwin_s* win = ev->userdata;

	if( *ev->keyboard.utf8 == '\x1B' ){
		dbg_info("destroy simple app");
		xorg_send_destroy(ev->x, ev->win);
	}
	else if( ev->keyboard.keycode == 113 ){
		dbg_info("left");
		win->child->px -= 5;
		xorg_win_move(win->x, win->child->id, win->child->px, win->child->py);
	}
	else if( ev->keyboard.keycode == 114 ){
		dbg_info("right");
		win->child->px += 5;
		xorg_win_move(win->x, win->child->id, win->child->px, win->child->py);
	}
	else if( ev->keyboard.keycode == 111 ){
		dbg_info("up");
		win->child->py -= 5;
		xorg_win_move(win->x, win->child->id, win->child->px, win->child->py);
	}
	else if( ev->keyboard.keycode == 116 ){
		dbg_info("down");
		win->child->py += 5;
		xorg_win_move(win->x, win->child->id, win->child->px, win->child->py);
	}
	return 0;
}

__private int main_mouse_move(xorgEvent_s* ev){
	xwin_s* win = ev->userdata;

	win->child->px = ev->mouse.relative.x;
	win->child->py = ev->mouse.relative.y;
	xorg_win_move(win->x, win->child->id, win->child->px, win->child->py);

	return 0;
}


__private int main_move(xorgEvent_s* ev){
	xwin_s* win = ev->userdata;
	if( ev->move.coord.w != win->surf->img->w || ev->move.coord.h != win->surf->img->h){
		dbg_info("redraw because: %d != %d && %d != %d",  ev->move.coord.w, win->surf->img->w, ev->move.coord.h, win->surf->img->h);
	   	xorg_surface_resize(win->surf, ev->move.coord.w, ev->move.coord.h);
		win->redraw(win->surf->img);
		xorg_win_surface_redraw(win->x, win->id, win->surf);
	}
	return 0;
}

__private int main_draw(xorgEvent_s* ev){
	xwin_s* win = ev->userdata;
	xorg_win_surface_redraw(ev->x, ev->win, win->surf);
	return 0;
}

__private void main_redraw(g2dImage_s* img){
	g2dColor_t bkcol = g2d_color_gen(X_COLOR_MODE, 255, 125,125,125); 
	g2dCoord_s gw = { .x = 0, .y = 0, .w = img->w, .h = img->h };
	g2d_clear(img, bkcol, &gw);
}

__private void child_redraw(g2dImage_s* img){
	
//	__g2d_free g2dImage_s* resize = g2d_resize(bkimg, img->w, img->h);
//	g2dCoord_s s = { .x = 0, .y =0, .w =img->w, .h = img->h };
//	g2dCoord_s d = { .x = 0, .y =0, .w =img->w, .h = img->h };
//	g2d_bitblt(img, &d, resize, &s);	
//	g2dColor_t bkcol = g2d_color_gen(X_COLOR_MODE, 255, 0, 200, 125); 
//	g2d_clear(img, bkcol, &d);
//	g2d_bitblt_alpha(img, &d, bkimg, &s);

}

__private int child_draw(xorgEvent_s* ev){
/*
	dbg_warning("REDRAW GIF");
	xwin_s* win = ev->userdata;
	vector_foreach(gif->frames,i){
		g2dCoord_s s = { .x = 0, .y =0, .w = gif->frames[i].img->w, .h = gif->frames[i].img->h };
		g2dCoord_s d = { .x = 0, .y =0, .w = win->surf->img->w, .h = win->surf->img->h };
		g2d_bitblt_alpha(win->surf->img, &d, gif->frames[i].img, &s);	

		xorg_win_surface_redraw(ev->x, ev->win, win->surf);
		xorg_client_flush(ev->x);
		xorg_client_sync(ev->x);
		delay_ms(gif->frames[i].delay);
		//delay_ms(1500);
	}
*/
	dbg_warning("REDRAW MEDIA");
	xwin_s* win = ev->userdata;
	media_resize_set(media, win->surf->img);	

	int ret = 0;
	while( ret >= 0 ){
		const size_t ts = time_us();
		ret=media_decode(media);
		if( ret > 0 ){
			xorg_win_surface_redraw(ev->x, ev->win, win->surf);
			const size_t te = time_us();
			media_sleep(media);
			const size_t ted = time_us();
			const size_t T = te-ts;
			const size_t D = ted-ts;
			printf("time:: max:%lu fps:%f del:%lu fps:%f\n", T, 1.0/(T/1000000.0), D, 1.0/(D/1000000.0) );
		}
	}

	return 0;
}

xwin_s* main_win(xorg_s* x){
	xwin_s* win = mem_new(xwin_s);
	win->x = x;

	g2dCoord_s pos = { 
		.x = 100,
		.y = 100,
		.w = 800,
		.h = 600,
	};
	g2dColor_t bkcol = g2d_color_gen(X_COLOR_MODE, 255, 125,125,125); 
	win->id = xorg_win_new(&win->surf, win->x, xorg_root(win->x), &pos, 1, bkcol);
	win->redraw = main_redraw;
	win->move = main_move;
	win->key = main_key;
	win->draw = main_draw;
	win->mouse = main_mouse_move;
	win->child = NULL;

	win->child = mem_new(xwin_s);
	win->child->px = 10;
	win->child->py = 10;
	win->child->x = x;
	win->child->draw = child_draw;
	//win->child->draw = main_draw;
	win->child->key = child_key;
	win->child->move = NULL;
	win->child->mouse = NULL;
	win->child->child = NULL;

	pos.x = win->child->px;
	pos.y = win->child->py;
	pos.w = 640;
	pos.h = 480;
	bkcol = g2d_color_gen(X_COLOR_MODE, 255, 0, 0, 0); 
	
	win->child->id = xorg_win_new(&win->child->surf, win->x, win->id, &pos, 0, bkcol);
	win->child->redraw = child_redraw;

	win->redraw(win->surf->img);
	win->child->redraw(win->child->surf->img);

	
	xorg_win_surface_redraw(x, win->id, win->surf);
	xorg_win_surface_redraw(x, win->child->id, win->child->surf);

	xorg_win_show(win->x, win->id, 1);
	xorg_win_show(win->x, win->child->id, 1);

	return win;
}

__private err_t x_events(__unused deadpoll_s* dp, __unused int ev, void* arg){
	xwin_s* win = arg;
	xwin_s* sel;

	xorgEvent_s* event;
	while( (event=xorg_event_new(win->x, 1)) ){
		if( event->win == win->id ){
			sel = win;
		}
		else if( event->win == win->child->id ){
			sel = win->child;
		}
		else{
			xorg_event_free(event);
			continue;
		}
		event->userdata = sel;

		switch( event->type ){
			case XCB_DESTROY_NOTIFY:
				return -1;
			break;

			case XCB_EXPOSE:
				if( sel->draw ) sel->draw(event);
			break;

			case XCB_KEY_RELEASE:
				if( sel->key ) sel->key(event);
			break;
		
			case XCB_MOTION_NOTIFY:
				if( sel->mouse ) sel->mouse(event);
			break;

			case XCB_CONFIGURE_NOTIFY:
				if( sel->move ) sel->move(event);
			break;

		}
		xorg_event_free(event);
	}
	xorg_client_flush(win->x);

	return 0;
}

/*@fn*/
void test_gui(__unused const char* argA, __unused const char* argB){
	err_enable();
	utf_begin();
	os_begin();	
	ft_begin();

	bkimg = g2d_load("/home/vbextreme/Immagini/test/sample.bmp", 0,0);
	
	dbg_warning("TEST GIF");
	//gif = g2d_load_gif("/home/vbextreme/Immagini/test/catto.gif");
	//gif = g2d_load_gif("/home/vbextreme/Immagini/test/catto2.gif");
	//gif = g2d_load_gif("/home/vbextreme/Immagini/test/meme0.gif");
	//if( !gif ) err_fail("gif");
	//g2d_gif_resize(gif, 320, 180);

	//bkimg = g2d_load("/home/vbextreme/Immagini/Sfondi/ALICE_WONDERLAND_fantasy_fairy_adventure_comedy_depp_disney_2560x1440.jpg",0,0);
	if( !bkimg ){
		err_fail("loading image");
	}

	//media = media_load("/home/vbextreme/Immagini/test/small_bunny_1080p_60fps.mp4");
	media = media_load("/home/vbextreme/Video/films/AliceNelPaeseDelleMeraviglie.mp4");
	if( !media ) err_fail("media load");

	deadpoll_s* dp = deadpoll_new();

	xorg_s* x = xorg_client_new(NULL, 0);
	xorg_register_events(x, xorg_root(x), XCB_EVENT_MASK_PROPERTY_CHANGE);
	
	xwin_s* win = main_win(x);
	
	deadpoll_register(dp, xorg_fd(x), x_events, win, 0, NULL);
	deadpoll_loop(dp, -1);
	deadpoll_free(dp);
	
	ft_end();
	//xorg_surface_destroy(x, win.surf);
	xorg_client_free(x);
	err_restore();
}

