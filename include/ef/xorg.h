#ifndef __EF_XORG_H__
#define __EF_XORG_H__

#include <ef/type.h>
#include <ef/image.h>
#include <ef/utf8.h>
#include <xcb/xcb.h>
#include <xcb/xproto.h>
#include <xcb/xcb_aux.h>
#include <xcb/xcb_image.h>
#include <xcb/xcb_atom.h>
#include <xcb/xcb_icccm.h>
#include <xcb/composite.h>
#include <xcb/xcb_xrm.h>
#include <xkbcommon/xkbcommon.h>

#ifdef XCB_ERROR_ENABLE
	#include <xcb/xcb_errors.h>
#endif

#define XKB_UTF_MAX 32

#define X_COLOR_MODE G2D_MODE_ARGB

#define X_WIN_EVENT XCB_EVENT_MASK_EXPOSURE |\
	XCB_EVENT_MASK_KEY_PRESS |\
	XCB_EVENT_MASK_KEY_RELEASE |\
	XCB_EVENT_MASK_BUTTON_PRESS |\
	XCB_EVENT_MASK_BUTTON_RELEASE |\
	XCB_EVENT_MASK_POINTER_MOTION |\
	XCB_EVENT_MASK_ENTER_WINDOW |\
	XCB_EVENT_MASK_LEAVE_WINDOW |\
	XCB_EVENT_MASK_VISIBILITY_CHANGE |\
	XCB_EVENT_MASK_STRUCTURE_NOTIFY |\
	XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |\
	XCB_EVENT_MASK_FOCUS_CHANGE |\
	XCB_EVENT_MASK_PROPERTY_CHANGE

#define XORG_MOUSE_CLICK_MS 200
#define XORG_MOUSE_DBLCLICL_MS 350

typedef enum {
	XORG_ATOM_NET_ACTIVE_WINDOW = 0,
	XORG_ATOM_NET_NUMBER_OF_DESKTOPS,
	XORG_ATOM_NET_CURRENT_DESKTOP,
	XORG_ATOM_NET_DESKTOP_NAMES,
	XORG_ATOM_NET_ACTIVE_DESKTOP,
	XORG_ATOM_NET_WM_DESKTOP, 
	XORG_ATOM_NET_WM_WINDOW_TYPE, 
	XORG_ATOM_NET_WM_WINDOW_TYPE_DESKTOP, 
	XORG_ATOM_NET_WM_WINDOW_TYPE_DOCK, 
	XORG_ATOM_NET_WM_WINDOW_TYPE_TOOLBAR, 
	XORG_ATOM_NET_WM_WINDOW_TYPE_MENU, 
	XORG_ATOM_NET_WM_WINDOW_TYPE_UTILITY, 
	XORG_ATOM_NET_WM_WINDOW_TYPE_SPLASH, 
	XORG_ATOM_NET_WM_WINDOW_TYPE_DIALOG, 
	XORG_ATOM_NET_WM_WINDOW_TYPE_NORMAL, 
	XORG_ATOM_NET_WM_STATE, 
	XORG_ATOM_NET_WM_STATE_MODAL,
	XORG_ATOM_NET_WM_STATE_STICKY,
	XORG_ATOM_NET_WM_STATE_MAXIMIZED_VERT,
	XORG_ATOM_NET_WM_STATE_MAXIMIZED_HORZ,
	XORG_ATOM_NET_WM_STATE_SHADED,
	XORG_ATOM_NET_WM_STATE_SKIP_TASKBAR,
	XORG_ATOM_NET_WM_STATE_SKIP_PAGER,
	XORG_ATOM_NET_WM_STATE_HIDDEN,
	XORG_ATOM_NET_WM_STATE_FULLSCREEN,
	XORG_ATOM_NET_WM_STATE_ABOVE,
	XORG_ATOM_NET_WM_STATE_BELOW,
	XORG_ATOM_NET_WM_STATE_DEMANDS_ATTENTION,
	XORG_ATOM_NET_WM_VISIBLE_NAME, 
	XORG_ATOM_NET_WM_NAME,
	XORG_ATOM_NET_WM_STRUT,
	XORG_ATOM_NET_WM_STRUT_PARTIAL,
	XORG_ATOM_NET_WM_PID,
	XORG_ATOM_NET_WM_WINDOW_OPACITY,
	XORG_ATOM_NET_WM_ALLOWED_ACTIONS,
	XORG_ATOM_NET_WM_ACTION_MOVE,
	XORG_ATOM_NET_WM_ACTION_RESIZE,
	XORG_ATOM_NET_WM_ACTION_MINIMIZE,
	XORG_ATOM_NET_WM_ACTION_SHADE,
	XORG_ATOM_NET_WM_ACTION_STICK,
	XORG_ATOM_NET_WM_ACTION_MAXIMIZE_HORZ,
	XORG_ATOM_NET_WM_ACTION_MAXIMIZE_VERT,
	XORG_ATOM_NET_WM_ACTION_FULLSCREEN,
	XORG_ATOM_NET_WM_ACTION_CHANGE_DESKTOP,
	XORG_ATOM_NET_WM_ACTION_CLOSE,
	XORG_ATOM_WM_TRANSIENT_FOR,
	XORG_ATOM_WM_STATE,
	XORG_ATOM_XROOTPMAP_ID,
	XORG_ATOM_PRIMARY,
	XORG_ATOM_CLIPBOARD,
	XORG_ATOM_XSEL_DATA,
	XORG_ATOM_UTF8_STRING,
	XORG_ATOM_COUNT
}xorgAtom_e;

typedef enum{
	XORG_WINDOW_TYPE_DESKTOP,
	XORG_WINDOW_TYPE_DOCK,
	XORG_WINDOW_TYPE_TOOLBAR, 
	XORG_WINDOW_TYPE_MENU, 
	XORG_WINDOW_TYPE_UTILITY, 
	XORG_WINDOW_TYPE_SPLASH, 
	XORG_WINDOW_TYPE_DIALOG, 
	XORG_WINDOW_TYPE_NORMAL
}xorgWindowType_e;

typedef enum{
	XORG_WINDOW_STATE_MODAL,
	XORG_WINDOW_STATE_STICKY,
	XORG_WINDOW_STATE_MAXIMIZED_VERT,
	XORG_WINDOW_STATE_MAXIMIZED_HORZ,
	XORG_WINDOW_STATE_SHADED,
	XORG_WINDOW_STATE_SKIP_TASKBAR,
	XORG_WINDOW_STATE_SKIP_PAGER,
	XORG_WINDOW_STATE_HIDDEN,
	XORG_WINDOW_STATE_FULLSCREEN,
	XORG_WINDOW_STATE_ABOVE,
	XORG_WINDOW_STATE_BELOW,
	XORG_WINDOW_STATE_DEMANDS_ATTENTION,
	XORG_WINDOW_STATE_INVISIBLE,
	XORG_WINDOW_STATE_NORMAL,
	XORG_WINDOW_STATE_ICONIZED
}xorgWindowState_e;

typedef enum {
	XORG_WINDOW_ACTION_MOVE          = 0x0001,
	XORG_WINDOW_ACTION_RESIZE        = 0x0002,
	XORG_WINDOW_ACTION_MINIMIZE      = 0x0004,
	XORG_WINDOW_ACTION_SHADE         = 0x0008,
	XORG_WINDOW_ACTION_STICK         = 0x0010,
	XORG_WINDOW_ACTION_MAXIMIZE_HORZ = 0x0020,
	XORG_WINDOW_ACTION_MAXIMIZE_VERT = 0x0040,
	XORG_WINDOW_ACTION_FULLSCREEN    = 0x0080,
	XORG_WINDOW_ACTION_CHANGE_DESKTOP= 0x0100,
	XORG_WINDOW_ACTION_CLOSE         = 0x0200
}xorgWindowAction_e;

typedef struct xkb{
	struct xkb_context* ctx;
	struct xkb_keymap* keymap;
	int device;
}xkb_s;

typedef struct xorgSurface{
	xcb_gcontext_t gc;
	xcb_image_t* ximage;
	g2dImage_s* img;
}xorgSurface_s;

typedef struct monitor{
	char* name;
	bool_t connected;
	g2dCoord_s size;
}monitor_s;

typedef struct xorg{
	xcb_connection_t* connection;
	xcb_screen_t* screen;
	monitor_s* monitor;
	size_t monitorCount;
	monitor_s* monitorCurrent;
	monitor_s* monitorPrimary;
	const char* display;
	xcb_colormap_t colormap;
	xkb_s key;
#ifdef XCB_ERROR_ENABLE
	xcb_errors_context_t* err;
#endif
	xorgAtom_e atom[XORG_ATOM_COUNT];
	xcb_visualid_t visual;
	xcb_xrm_database_t* resources;
	long clickms;
	long dblclickms;
	long _mousetime;
	unsigned _mousestate;
	int screenDefault;
	int screenCurrent;
	uint8_t depth;
}xorg_s;

#define XORG_WINDOW_HINTS_FLAGS_URGENCY(XWPTR)  ((XWPTR)->hints.flags & XCB_ICCCM_WM_HINT_X_URGENCY)
#define XORG_WINDOW_VISIBLE_UNMAP  XCB_MAP_STATE_UNMAPPED
#define XORG_WINDOW_VISIBLE_UNVIEW XCB_MAP_STATE_UNVIEWABLE
#define XORG_WINDOW_VISIBLE_MAP    XCB_MAP_STATE_VIEWABLE

struct xorgWindowStrut{
	unsigned left, right, top, bottom;
} __packed;
typedef struct xorgWindowStrut xorgWindowStrut_s;

struct xorgWindowStrutPartial{
	unsigned left, right, top, bottom;
	unsigned left_start_y, left_end_y;
	unsigned right_start_y, right_end_y;
	unsigned top_start_x, top_end_x;
	unsigned bottom_start_x,bottom_end_x;
} __packed;
typedef struct xorgWindowStrutPartial xorgWindowStrutPartial_s;

typedef struct xorgWindow{
	xcb_window_t idxcb;
	char* class;
	char* name;
	char* netname;
	char* title;
	xcb_icccm_wm_hints_t hints;
	xcb_size_hints_t size;
	unsigned x, y, w, h, border, visible;
	unsigned desktop;
	xcb_atom_t* type;
	size_t typeCount;
	xcb_atom_t* state;
	size_t stateCount;
	xorgWindowStrut_s strut;
	xorgWindowStrutPartial_s partial;
	pid_t pid;
}xorgWindow_s;

typedef enum {XORG_MOUSE_RELEASE, XORG_MOUSE_PRESS, XORG_MOUSE_MOVE, XORG_MOUSE_ENTER, XORG_MOUSE_LEAVE, XORG_MOUSE_CLICK, XORG_MOUSE_DBLCLICK} xorgMouse_e;
typedef enum {XORG_KEY_RELEASE, XORG_KEY_PRESS} xorgKey_e;
typedef enum {
	XORG_EVENT_CREATE         = XCB_CREATE_NOTIFY,
	XORG_EVENT_DESTROY        = XCB_DESTROY_NOTIFY,
	XORG_EVENT_DRAW           = XCB_EXPOSE,
	XORG_EVENT_KEY_PRESS      = XCB_KEY_PRESS,
	XORG_EVENT_KEY_RELEASE    = XCB_KEY_RELEASE,
	XORG_EVENT_BUTTON_PRESS   = XCB_BUTTON_PRESS,
	XORG_EVENT_BUTTON_RELEASE = XCB_BUTTON_RELEASE,
	XORG_EVENT_MOTION         = XCB_MOTION_NOTIFY,
	XORG_EVENT_ENTER          = XCB_ENTER_NOTIFY,
	XORG_EVENT_LEAVE          = XCB_LEAVE_NOTIFY,
	XORG_EVENT_FOCUS_IN       = XCB_FOCUS_IN,
	XORG_EVENT_FOCUS_OUT      = XCB_FOCUS_OUT,
	XORG_EVENT_MAP            = XCB_MAP_NOTIFY,
	XORG_EVENT_UNMAP          = XCB_UNMAP_NOTIFY,
	XORG_EVENT_MOVE           = XCB_CONFIGURE_NOTIFY,
	XORG_EVENT_ATOM           = XCB_PROPERTY_NOTIFY,
	XORG_EVENT_CLIENT         = XCB_CLIENT_MESSAGE,
	XORG_EVENT_CLIPBOARD      = XCB_SELECTION_NOTIFY
}xorgEvent_e;

typedef struct xorgMouse{
	xorgMouse_e event;
	g2dPoint_s absolute;
	g2dPoint_s relative;
	unsigned button;
	unsigned key;
	long time;
}xorgMouse_s;

#define XORG_KEY_MOD_SHIFT_L   0x0001
#define XORG_KEY_MOD_SHIFT_R   0x0002
#define XORG_KEY_MOD_SHIFT     (3<<0)
#define XORG_KEY_MOD_CONTROL_L 0x0004
#define XORG_KEY_MOD_CONTROL_R 0x0008
#define XORG_KEY_MOD_CONTROL   (3<<2)
#define XORG_KEY_MOD_META_L    0x0010
#define XORG_KEY_MOD_META_R	   0x0020
#define XORG_KEY_MOD_META      (3<<4)
#define XORG_KEY_MOD_ALT_L     0x0040
#define XORG_KEY_MOD_ALT_R     0x0080
#define XORG_KEY_MOD_ALT       (3<<6)
#define XORG_KEY_MOD_SUPER_L   0x0100
#define XORG_KEY_MOD_SUPER_R   0x0200
#define XORG_KEY_MOD_SUPER     (3<<8)
#define XORG_KEY_MOD_HYPER_L   0x0400
#define XORG_KEY_MOD_HYPER_R   0x0800
#define XORG_KEY_MOD_HYPER     (3<<10)
#define XORG_KEY_MOD_CAPSLOCK  0x1000

typedef struct xorgKeyboard{
	xorgKey_e event;
	g2dPoint_s absolute;
	g2dPoint_s relative;
	unsigned button;
	unsigned modifier;
	unsigned long keycode;
	unsigned long keysym;
	utf8_t utf8[XKB_UTF_MAX];
	utf_t utf;
	long time;
}xorgKeyboard_s;

typedef struct xorgMove{
	int x;
	int y;
	unsigned w;
	unsigned h;
	unsigned border;
}xorgMove_s;

typedef struct xorgCreate{
	int x;
	int y;
	int w;
	int h;
	int border;
}xorgCreate_s;

typedef struct xorgDraw{
	int x;
	int y;
	int w;
	int h;
}xorgDraw_s;

typedef struct xorgFocus{
	int outin;
}xorgFocus_s;

typedef struct xorgVisible{
	int visible;
}xorgVisible_s;

typedef struct xorgProperty{
	xcb_atom_t atom;
}xorgProperty_s;

typedef struct xorgClient{
	xcb_atom_t type;
	uint8_t format;
	uint8_t data[20];
}xorgClient_s;

typedef struct xorgClipboard{
	xcb_window_t requestor;
	int primary;
	utf8_t* str;
}xorgClipboard_s;

typedef struct xorgEvent{
	int type;
	void* userdata;
	xorg_s* x;
	xcb_window_t win;
	union{
		xorgMouse_s mouse;
		xorgKeyboard_s keyboard;
		xorgMove_s move;
		xorgCreate_s create;
		xorgDraw_s draw;
		xorgFocus_s focus;
		xorgVisible_s visible;
		xorgProperty_s property;
		xorgClient_s client;
		xorgClipboard_s clipboard;
	};
}xorgEvent_s;	

#define xorg_root(XORG) ((XORG)->screen->root)
#define xorg_root_x(XORG) ((XORG)->monitorCurrent->size.x)
#define xorg_root_y(XORG) ((XORG)->monitorCurrent->size.y)
#define xorg_root_width(XORG) ((XORG)->monitorCurrent->size.w)
#define xorg_root_height(XORG) ((XORG)->monitorCurrent->size.h)
#define xorg_root_visual(XORG) ((XORG)->screen->root_visual)
#define xorg_fd(XORG) xcb_get_file_descriptor((XORG)->connection)

#define xorg_resources_string_get(XORG, NAME, CLASS, PTRSTR) xcb_xrm_resource_get_string((XORG)->resources, NAME, CLASS, PTRSTR)
#define xorg_resources_long_get(XORG, NAME, CLASS, PTRLONG) xcb_xrm_resource_get_long((XORG)->resources, NAME, CLASS, PTRLONG)
#define xorg_resources_bool_get(XORG, NAME, CLASS, PTRBOOL) xcb_xrm_resource_get_bool((XORG)->resources, NAME, CLASS, PTRBOOL)

/** create new xorg client*/
xorg_s* xorg_client_new(const char* display, int defaultScreen);

/** free xorg client*/
void xorg_client_free(xorg_s* x);

/** initialized root to screen, this function is called inside client_new*/
err_t xorg_root_init(xorg_s* x, int onscreen);

/** flush a request, not really need*/
void xorg_client_flush(xorg_s* x);

/** sync connectoion, not really need*/
void xorg_client_sync(xorg_s* x);

#ifdef XCB_ERROR_ENABLE
const char* xorg_error_major(xorg_s* x, xcb_generic_error_t* err);
const char* xorg_error_minor(xorg_s* x, xcb_generic_error_t* err);
const char* xorg_error_string(xorg_s* x, xcb_generic_error_t* err, const char** extensionname);
#endif

/** return screen from idscreen*/
xcb_screen_t* xorg_screen_get(xorg_s* x, int idScreen);

/** refresh randr monitor*/
void xorg_randr_monitor_refresh(xorg_s* x);

/** set monitor by name*/
err_t xorg_monitor_byname(xorg_s* x, char const* name);

/** set monitor in position size*/
err_t xorg_monitor_bysize(xorg_s* x, g2dCoord_s* size);

/** set primary monitor*/
err_t xorg_monitor_primary(xorg_s* x);

/** get name from atom*/
const char* xorg_atom_name(xorg_s* x, xcb_atom_t atom);

/** get atom friom name*/
xcb_atom_t xorg_atom_id(xorg_s* x, const char* name);

/** create new atom*/
xcb_atom_t xorg_atom_new_id(xorg_s* x, const char* name);

/** load default atom*/
void xorg_atom_load(xorg_s* x);

xcb_visualid_t xorg_find_depth(xorg_s* x, uint8_t depth);

/** xcp wrap*/
int xorg_xcb_attribute(xorg_s* x, xcb_get_window_attributes_cookie_t cookie);

/** xcb wrap*/
err_t xorg_xcb_geometry(xorg_s* x, xcb_get_geometry_cookie_t cookie, unsigned* X, unsigned* Y, unsigned* W, unsigned* H, unsigned* B);

/** xcb wrap*/
int xorg_xcb_property_cardinal(xorg_s* x, xcb_get_property_cookie_t cookie);

/** xcb wrap*/
xcb_get_property_cookie_t xorg_xcb_property_cookie_string(xorg_s* x, xcb_window_t win, xcb_atom_t atom);

/** xcb wrap*/
char* xorg_xcb_property_string(xorg_s* x, xcb_get_property_cookie_t cookie);

/** xcb wrap*/
err_t xorg_xcb_property_structure(void* out, xorg_s* x, xcb_get_property_cookie_t cookie, xcb_atom_t type, size_t size, size_t minsize);

/** xcb wrap*/
xcb_pixmap_t xorg_xcb_property_pixmap(xorg_s* x, xcb_get_property_cookie_t cookie);

/** send creat message*/
void xorg_send_creat(xorg_s* x, xcb_window_t parent, xcb_window_t win, int px, int py, int w, int h);

/** send destroy message*/
void xorg_send_destroy(xorg_s* x, xcb_window_t win);

/** send expose, redraw, message*/
void xorg_send_expose(xorg_s* x, xcb_window_t win, int px, int py, int w, int h);

/** send key press message*/
void xorg_send_key_press(xorg_s* x, xcb_window_t win, xcb_keycode_t keycode, xcb_timestamp_t time, int rx, int ry, int px, int py, int state, int samescreen);

/** send key release message*/
void xorg_send_key_release(xorg_s* x, xcb_window_t win, xcb_keycode_t keycode, xcb_timestamp_t time, int rx, int ry, int px, int py, int state, int samescreen);

/** send button press*/
void xorg_send_button_press(xorg_s* x, xcb_window_t win, xcb_button_t button, xcb_timestamp_t time, int rx, int ry, int px, int py, int state, int samescreen);

/** send button release*/
void xorg_send_button_release(xorg_s* x, xcb_window_t win, xcb_button_t button, xcb_timestamp_t time, int rx, int ry, int px, int py, int state, int samescreen);

/** send mouse move*/
void xorg_send_motion(xorg_s* x, xcb_window_t win, xcb_button_t button, xcb_timestamp_t time, int rx, int ry, int px, int py, int state, int samescreen);

/** send mouse enter*/
void xorg_send_enter(xorg_s* x, xcb_window_t win, xcb_button_t button, xcb_timestamp_t time, int rx, int ry, int px, int py, int state, int samescreen);

/** send mouse exit*/
void xorg_send_leave(xorg_s* x, xcb_window_t win, xcb_button_t button, xcb_timestamp_t time, int rx, int ry, int px, int py, int state, int samescreen);

/** send focus in*/
void xorg_send_focus_in(xorg_s* x, xcb_window_t win);

/** send focus out*/
void xorg_send_focus_out(xorg_s* x, xcb_window_t win);

/** send map/visible*/
void xorg_send_map(xorg_s* x, xcb_window_t win);

/** send unmap/hide*/
void xorg_send_unmap(xorg_s* x, xcb_window_t win);

/** send configure*/
void xorg_send_configure(xorg_s* x, xcb_window_t win, int px, int py, int w, int h, int border);

/** send property*/
void xorg_send_property(xorg_s* x, xcb_window_t win, xcb_atom_t atom);

/** send client*/
void xorg_send_client(xorg_s* x, xcb_window_t win, uint8_t type, xcb_atom_t atom, uint8_t* data, size_t len);

/** send client 32*/
void xorg_send_client32(xorg_s* x, xcb_window_t win, xcb_window_t dest, xcb_atom_t atom, const uint32_t* data, size_t len);

/** send active window*/
void xorg_send_active_window(xorg_s* x, xcb_window_t current, xcb_window_t activate);

/** send current desktop*/
void xorg_send_current_desktop(xorg_s* x, uint32_t desktop);

/** send set desktop*/
void xorg_send_set_desktop(xorg_s* x, xcb_window_t win, uint32_t desktop);

/** free window */
void xorg_window_release(xorgWindow_s* win);

/** tree of window*/
xorgWindow_s* xorg_query_tree(size_t* count, xorg_s* x, xcb_window_t root);

/** tree of window app*/
xorgWindow_s* xorg_window_application(xorg_s* x,  size_t nworkspace, xcb_window_t id, xorgWindow_s* stack, size_t* appCount);

/** get window parent*/
xcb_window_t xorg_parent(xorg_s* x, xcb_window_t win);

/** return countr of workspace*/
unsigned xorg_workspace_count(xorg_s* x);

/** return active workspace*/
unsigned xorg_workspace_active(xorg_s* x);

/** get workspace name*/
char** xorg_workspace_name_get(xorg_s* x);

/** return buffer ximage*/
uint8_t* xorg_ximage_get_composite(unsigned* outW, unsigned* outH, unsigned* outV, unsigned* outD, xorg_s* x, xcb_window_t id);

/** get root pixmap*/
xcb_pixmap_t xorg_root_pixmap_get(xorg_s* x);

/** get pixel root image*/
uint8_t* xorg_ximage_root_get(unsigned* outW, unsigned* outH, unsigned* outV, unsigned* outD, xorg_s* x);

/** grab image of app*/
g2dImage_s* xorg_image_grab(xorg_s* x, xcb_window_t id);

/** grab root image*/
g2dImage_s* xorg_root_image_grab(xorg_s* x);

/** set window title*/
void xorg_win_title(xorg_s* x, xcb_window_t id, char const* name);

/** set window class*/
void xorg_win_class(xorg_s* x, xcb_window_t id, char const* name);

/** show windo*/
void xorg_win_show(xorg_s* x, xcb_window_t id, int show);

/** move window*/
void xorg_win_move(xorg_s* x, xcb_window_t id, unsigned X, unsigned y);

/** change border size*/
void xorg_win_border(xorg_s* x, xcb_window_t id, unsigned border);

/** resize window*/
void xorg_win_resize(xorg_s* x, xcb_window_t id, unsigned w, unsigned h);

/** set window coordinate*/
void xorg_win_coord(xorg_s* x, xcb_window_t id, g2dCoord_s* pos);

/** set window size*/
void xorg_win_size(g2dCoord_s* out, unsigned* outBorder, xorg_s* x, xcb_window_t idxcb);

/** redraw surface*/
void xorg_win_surface_redraw(xorg_s* x, xcb_window_t id,  xorgSurface_s* surface);

/** set window type */
void xorg_win_type_set(xorg_s* x, xcb_window_t id, xorgWindowType_e type);

/** remove window decoration */
void xorg_win_decoration_remove(xorg_s* x, xcb_window_t id);

/** set window state*/
void xorg_win_state_set(xorg_s* x, xcb_window_t id, xorgWindowState_e state);

/** set window action*/
void xorg_win_action_set(xorg_s* x, xcb_window_t id, xorgWindowAction_e action);

/** set top window*/
void xorg_win_set_top(xorg_s* x, xcb_window_t parent,  xcb_window_t id, int enable);

/** clear partial decoration*/
void xorg_win_round_decoration_clear(xorg_s* x, xcb_window_t win, const unsigned w, const unsigned h, unsigned size);

/** set round border*/
void xorg_win_round_border(xorg_s* x, xcb_window_t win, const unsigned w, const unsigned h, const int r);

/** remove round border*/
void xorg_win_round_remove(xorg_s* x, xcb_window_t win);

/** request a compositor to set window opacity */
void xorg_win_opacity_set(xorg_s* x, xcb_window_t win, unsigned int opacity);

/** request a compositor to get opacity*/
unsigned xorg_win_opacity_get(xorg_s* x, xcb_window_t win);

/** reserve dock space*/
void xorg_wm_reserve_dock_space_on_top(xorg_s* x, xcb_window_t id, unsigned X, unsigned w, unsigned h);

/** reserve dock space*/
void xorg_wm_reserve_dock_space_on_bottom(xorg_s* x, xcb_window_t id, unsigned X, unsigned w, unsigned h);

/** reserve dock space*/
void xorg_wm_reserve_dock_space_on_left(xorg_s* x, xcb_window_t id, unsigned y, unsigned w, unsigned h);

/** reserve dock space*/
void xorg_wm_reserve_dock_space_on_right(xorg_s* x, xcb_window_t id, unsigned y, unsigned w, unsigned h);

/** register event on window*/
void xorg_register_events(xorg_s* x, xcb_window_t window, unsigned int eventmask);

/** create new window, if surface return new surface, remember to free*/
xcb_window_t xorg_win_new(
		xorgSurface_s** surface, xorg_s* X, xcb_window_t parent, 
		int x, int y, unsigned w, unsigned h, int border, 
		g2dColor_t colbor, g2dColor_t background
);
//xcb_window_t xorg_win_new(xorgSurface_s** surface, xorg_s* X, xcb_window_t parent, int x, int y, unsigned w, unsigned h, unsigned border, g2dColor_t background);

/** resize surface*/
void xorg_surface_resize(xorg_s* X, xorgSurface_s* surface, unsigned w, unsigned h);
//void xorg_surface_resize(xorgSurface_s* surface, unsigned w, unsigned h);

/** deprecate resize a surface, blitting img*/
//void xorg_surface_resize_bitblt(xorgSurface_s* surface, unsigned w, unsigned h);

/** destroy/free sourface*/
void xorg_surface_destroy(xorg_s* x, xorgSurface_s* surface);

/** destroy window*/
void xorg_win_destroy(xorg_s* x, xcb_window_t id);

/** set focus on window*/
void xorg_win_focus(xorg_s* x, xcb_window_t id);

/** get new event, remember to release event*/
xorgEvent_s* xorg_event_new(xorg_s* x, int async);

/** free message*/
void xorg_event_free(xorgEvent_s* ev);

/** set primary owner for receve copy event*/
void xorg_clipboard_primary_copy(xorg_s* x, xcb_window_t owner);

/** set clipboard owner for receve copy event*/
void xorg_clipboard_clipboard_copy(xorg_s* x, xcb_window_t owner);

/** request paste*/
void xorg_clipboard_primary_paste(xorg_s* x, xcb_window_t win);

/** request paste*/
void xorg_clipboard_clipboard_paste(xorg_s* x, xcb_window_t win);

#endif 
