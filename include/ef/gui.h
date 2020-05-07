#ifndef __EF_GUI_H__
#define __EF_GUI_H__

#include <ef/type.h>
#include <ef/xorg.h>
#include <ef/deadpoll.h>
#include <ef/phq.h>
#include <ef/ft.h>
#include <ef/guiImage.h>

/*
 * 1.2.6 -----
 * 1.3.0 broken previous version
 *		gui:
 *			themes
 *			bar:
 *				circle
 *			
 *			round
 *			simple
 *		g2d:
 *			arc
 *			supersampling
 *
 *
 * ??? 
 *		option button
 *		radio button
 *		calendar
 *		combobox
 *		tinybutton
 *		msgbox
 */

#define GUI_TYPE_WINDOW 0
#define GUI_TYPE_LABEL  1
#define GUI_TYPE_BUTTON 2
#define GUI_TYPE_TEXT   4
#define GUI_TYPE_DIV    5
#define GUI_TYPE_BAR    6
#define GUI_TYPE_USER   999

#define GUI_BK_NO_OP 0x00
#define GUI_BK_COLOR 0x01
#define GUI_BK_CPOS  0x02
#define GUI_BK_IMAGE 0x04
#define GUI_BK_ALPHA 0x08
#define GUI_BK_FN    0x10

#define GUI_TIMER_FREE  -1
#define GUI_TIMER_NEXT   0
#define GUI_TIMER_CUSTOM 1

#define GUI_SHOW_VISIBLE 1
#define GUI_SHOW_HIDE    0

#define GUI_FOCUS_BORDER_SIZE 3

#define GUI_THEME_WM_ALPHA         "wm.alpha"

#define GUI_THEME_BORDER           "border"
#define GUI_THEME_ROUND            "round"
#define GUI_THEME_SUPERSAMPLING    "supersampling"
#define GUI_THEME_GENERIC          "generic"
#define GUI_THEME_X                "position.x"
#define GUI_THEME_Y                "position.y"
#define GUI_THEME_W                "position.w"
#define GUI_THEME_H                "position.h"
#define GUI_THEME_FONT_GROUP       "font.group"
#define GUI_THEME_FONT_NAME        "font.name."
#define GUI_THEME_FONT_SIZE        "font.size."

#define GUI_THEME_COMPOSITE        "composite"
#define GUI_THEME_COMPOSITE_COLOR  "color"
#define GUI_THEME_COMPOSITE_IMAGE  "image"
#define GUI_THEME_COMPOSITE_GIF    "gif"
#define GUI_THEME_COMPOSITE_VIDEO  "video"
#define GUI_THEME_COMPOSITE_ALPHA  "alpha"
#define GUI_THEME_COMPOSITE_PLAY   "play"
#define GUI_THEME_COMPOSITE_LOOP   "loop"
#define GUI_THEME_COMPOSITE_RATIO  "ratio"
#define GUI_THEME_COMPOSITE_DEST_X "dest.x"
#define GUI_THEME_COMPOSITE_DEST_Y "dest.y"
#define GUI_THEME_COMPOSITE_DEST_W "dest.w"
#define GUI_THEME_COMPOSITE_DEST_H "dest.h"
#define GUI_THEME_COMPOSITE_SRC_X  "src.x"
#define GUI_THEME_COMPOSITE_SRC_Y  "src.y"
#define GUI_THEME_COMPOSITE_SRC_W  "src.w"
#define GUI_THEME_COMPOSITE_SRC_H  "src.h"

typedef enum {GUI_MODE_NORMAL, GUI_MODE_MODAL, GUI_MODE_DOCK_TOP, GUI_MODE_DOCK_BOTTOM, GUI_MODE_DOCK_LEFT, GUI_MODE_DOCK_RIGHT} guiMode_e;

typedef struct gui gui_s;

typedef void(*guiBackgroundFN_f)(gui_s* gui);

typedef int(*guiEvent_f)(gui_s* gui, xorgEvent_s* event);

typedef struct guiTimer_s guiTimer_s;

typedef int(*guiTimer_f)(guiTimer_s* timer);

typedef struct guiPosition{
	int x,y;
	unsigned w,h;
}guiPosition_s;

typedef struct guiMargin{
	int left, right, top, bottom;
}guiMargin_s;

typedef struct guiTimer_s{
	size_t ms;
	size_t raisedon;
	gui_s* gui;
	void* userdata;
	guiTimer_f fn;
	phqElement_s* el;
}guiTimer_s;

typedef struct guiRound{
	unsigned radius;
	unsigned border;
}guiRound_s;

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
	int bordersize;
	int bordersizefocused;
	int genericSize;
	g2dColor_t borderColor;

	xorgSurface_s* surface;
	guiPosition_s position;
	guiComposite_s* img;
	guiMargin_s userMargin;

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
	guiEvent_f clipboard;
	guiEvent_f themes;
}gui_s;

#define gui_color(A,R,G,B) g2d_color_gen(X_COLOR_MODE, A, R, G, B)

/** before use gui*/
void gui_begin(void);

/** after use gui*/
void gui_end();

/** grab root event*/
void gui_register_root_event(void);

/** get screen width*/
unsigned gui_screen_width(void);

/** get screen height*/
unsigned gui_screen_height(void);

void gui_register_internal_focus_event(gui_s* gui, guiEvent_f fn);
void gui_unregister_internal_focus_event(gui_s* gui);
void gui_internal_focus_event(void);

/** create gui*/
gui_s* gui_new(
		gui_s* parent, 
		const char* name, const char* class, guiMode_e mode,
		int border, int x, int y, int width, int height, 
		g2dColor_t colorBorder, guiComposite_s* img,
		int genericSize, void* userdata);

/** free gui, remove gui from parent*/
void gui_free(gui_s* gui);

/** add child to gui*/
void gui_child_add(gui_s* parent, gui_s* child);

/** remove child from gui*/
gui_s* gui_child_remove(gui_s* parent, gui_s* child);

/** get main parent*/
gui_s* gui_main_parent(gui_s* gui);

/** set gui name*/
void gui_name(gui_s* gui, const char* name);

/** set gui class*/
void gui_class(gui_s* gui, const char* class);

/** show gui*/
void gui_show(gui_s* gui, int show);

/** move gui*/
void gui_move(gui_s* gui, int x, int y);

/** resize gui*/
void gui_resize(gui_s* gui, int w, int h);

/** set border*/
void gui_border(gui_s* gui, int border);

/** get id */
ssize_t gui_id(gui_s* gui);

/** get gui from name and class*/
gui_s* gui_by_name(gui_s* gui, const char* name, const char* class);

/** check if gui have focus*/
int gui_focus_have(gui_s* gui);

/** set focus on gui*/
void gui_focus(gui_s* gui);

/** set focus on next gui*/
gui_s* gui_focus_next(gui_s* gui);

/** set previous focus*/
gui_s* gui_focus_prev(gui_s* gui);

/** enable clipboard copy*/
void gui_clipboard_copy(gui_s* gui, int primary);

/** request paste*/
void gui_clipboard_paste(gui_s* gui, int primary);

/** send clipboard data*/
void gui_clipboard_send(xorgClipboard_s* clipboard, void* data, size_t size);

/** draw gui*/
void gui_draw(gui_s* gui);

/** redraw gui*/
void gui_redraw(gui_s* gui);

/** set opacity*/
void gui_opacity(gui_s* gui, double op);

/** remove round*/
void gui_round_unset(gui_s* gui);

/** set round border*/
void gui_round_set(gui_s* gui, int radius);

/** set antialiased round border*/
void gui_round_antialiasing_set(gui_s* gui, int radius);

/** request to remove decorations*/
void gui_remove_decoration(gui_s* gui);

/** default event for redraw*/
int gui_event_redraw(gui_s* gui, __unused xorgEvent_s* unset);

/** default event for draw*/
int gui_event_draw(gui_s* gui, __unused xorgEvent_s* evdamage);

/** default event for focus*/
int gui_event_focus(gui_s* gui, xorgEvent_s* event);

/** default event for mouse*/
int gui_event_mouse(gui_s* gui, xorgEvent_s* event);

/** default event for move*/
int gui_event_move(gui_s* gui, xorgEvent_s* event);

/** default event for key*/
int gui_event_key(gui_s* gui, xorgEvent_s* event);

/** get event*/
xorgEvent_s* gui_event_get(int async);

/** release event*/
void gui_event_release(xorgEvent_s* ev);

/** callback event*/
int gui_event_call(xorgEvent_s* ev);

/** deadpoll */
err_t gui_deadpoll_event_callback(__unused deadpoll_s* dp, __unused int ev, __unused void* arg);

/** deadpoll */
err_t gui_deadpoll_fd_callback(__unused deadpoll_s* dp, int ev, void* arg);

/** deadpoll */
void gui_deadpoll_unregister(deadpoll_s* dp);

/** deadpoll */
void gui_deadpoll_register(deadpoll_s* dp);

/** deadpoll */
int gui_deadpoll_event(deadpoll_s* dp);

/** loop gui*/
void gui_loop(void);

/** create new timer*/
guiTimer_s* gui_timer_new(gui_s* gui, size_t ms, guiTimer_f fn, void* userdata);

/** change timer*/
int gui_timer_change(guiTimer_s* timer, size_t ms);

/** free timer*/
void gui_timer_free(guiTimer_s* timer);

/** reister event fd */
void gui_fd_register(gui_s* gui, int fd, int event, guiEvent_f fn);

/** unregister event fd*/
void gui_fd_unregister(int fd);

void gui_background_supersampling_fn(gui_s* gui, __unused guiImage_s** img, __unused void* generic);

void gui_background_main_round_fn(gui_s* gui, __unused guiImage_s** img, void* generic);

void gui_background_round_fn(gui_s* gui, __unused guiImage_s** img, __unused void* generic);

/** get themes name*/
char* gui_themes_name(gui_s* gui, const char* appName);

/** get string */
char* gui_themes_string(const char* name, const char* property);

/** set bool*/
err_t gui_themes_bool_set(const char* name, const char* property, int* set);

/** set int */
err_t gui_themes_int_set(const char* name, const char* property, int* set);

/** set uint*/
err_t gui_themes_uint_set(const char* name, const char* property, unsigned* set);

/** set long*/
err_t gui_themes_long_set(const char* name, const char* property, long* set);

/** set fonts */
err_t gui_themes_fonts_set(const char* name, ftFonts_s** controlFonts);

/** load gui image */
err_t gui_themes_gui_image(gui_s* gui, const char* name, guiImage_s** ptrimg);

/** set theme composite*/
void gui_themes_composite(gui_s* gui, const char* name, const char* compname);

/** set gui themes */
void gui_themes(gui_s* gui, const char* appName);

/** set themes for all gui from one parent*/
void gui_themes_all(gui_s* gui, const char* appName);

#endif
