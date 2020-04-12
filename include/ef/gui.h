#ifndef __EF_GUI_H__
#define __EF_GUI_H__

#include <ef/type.h>
#include <ef/xorg.h>
#include <ef/guiResources.h>
#include <ef/deadpoll.h>
#include <ef/phq.h>

#define GUI_TYPE_WINDOW 0
#define GUI_TYPE_LABEL  1
#define GUI_TYPE_BUTTON 2
#define GUI_TYPE_LIST   3
#define GUI_TYPE_CHECK  4
#define GUI_TYPE_TEXT   5

#define GUI_BK_NO_OP    0x00
#define GUI_BK_COLOR    0x01
#define GUI_BK_IMAGE    0x02
#define GUI_BK_ALPHA    0x08

#define GUI_TIMER_FREE  -1
#define GUI_TIMER_NEXT   0
#define GUI_TIMER_CUSTOM 1

#define GUI_FOCUS_BORDER_SIZE 3

typedef struct gui gui_s;

typedef int(*guiEvent_f)(gui_s* gui, xorgEvent_s* event);

typedef struct guiTimer_s guiTimer_s;

typedef int(*guiTimer_f)(guiTimer_s* timer);

typedef struct guiTimer_s{
	size_t ms;
	size_t raisedon;
	gui_s* gui;
	void* userdata;
	guiTimer_f fn;
	phqElement_s* el;
}guiTimer_s;

typedef struct guiBackground{
	g2dColor_t color;
	g2dImage_s* img;
	unsigned mode;
}guiBackground_s;
	
typedef struct gui{
	char* name;
	char* class;
	struct gui* parent;
	struct gui** childs;
	void* control;
	void* userdata;

	int type;
	xcb_window_t id;
	int focusable;
	int childFocus;
	int bordersize;
	int bordersizefocused;

	xorgSurface_s* surface;
	g2dCoord_s position;
	guiBackground_s background;

	guiEvent_f create;
	guiEvent_f destroy;
	guiEvent_f free;
	guiEvent_f redraw;
	guiEvent_f draw;
	guiEvent_f key;
	guiEvent_f mouse;
	guiEvent_f focus;
	guiEvent_f map;
	guiEvent_f move;
	guiEvent_f atom;
	guiEvent_f client;

}gui_s;

#define gui_color(A,R,G,B) g2d_color_gen(X_COLOR_MODE, A, R, G, B)

void gui_begin(void);
void gui_end();

gui_s* gui_new(
		gui_s* parent, 
		const char* name, const char* class, 
		int border, int x, int y, int width, int height, 
		g2dColor_t color, 
		void* userdata);

void gui_free(gui_s* gui);

void gui_child_add(gui_s* parent, gui_s* child);
gui_s* gui_child_remove(gui_s* parent, gui_s* child);

void gui_name(gui_s* gui, const char* name);
void gui_class(gui_s* gui, const char* class);
void gui_show(gui_s* gui, int show);
void gui_move(gui_s* gui, int x, int y);
void gui_resize(gui_s* gui, int w, int h);
void gui_border(gui_s* gui, int border);
void gui_focus(gui_s* gui);
void gui_focus_next(gui_s* gui);
void gui_focus_prev(gui_s* gui);
void gui_draw(gui_s* gui);

int gui_event_redraw(gui_s* gui, __unused xorgEvent_s* unset);
int gui_event_draw(gui_s* gui, __unused xorgEvent_s* evdamage);
int gui_event_move(gui_s* gui, xorgEvent_s* event);
int gui_event_key(gui_s* gui, xorgEvent_s* event);

xorgEvent_s* gui_event_get(int async);
void gui_event_release(xorgEvent_s* ev);
int gui_event_call(xorgEvent_s* ev);

err_t gui_deadpoll_event_callback(__unused deadpoll_s* dp, __unused int ev, __unused void* arg);
void gui_deadpoll_unregister(deadpoll_s* dp);
void gui_deadpoll_register(deadpoll_s* dp);
int gui_deadpoll_event(deadpoll_s* dp);
void gui_loop(void);

guiTimer_s* gui_timer_new(gui_s* gui, size_t ms, void* userdata);
int gui_timer_change(guiTimer_s* timer, size_t ms);
void gui_timer_free(guiTimer_s* timer);

void gui_background_redraw(gui_s* gui, guiBackground_s* bkg);

#endif
