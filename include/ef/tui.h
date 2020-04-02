#ifndef __EF_TUI_H__
#define __EF_TUI_H__

#include <ef/type.h>
#include <ef/term.h>

#define TUI_TYPE_ROOT   0
#define TUI_TYPE_WINDOW 1
#define TUI_TYPE_LABEL  2
#define TUI_TYPE_BUTTON 3
#define TUI_TYPE_LIST   4
#define TUI_TYPE_CHECK  5
#define TUI_TYPE_TEXT   6

#define TUI_EVENT_RETURN_EXIT         2
#define TUI_EVENT_RETURN_FOCUS_PARENT 3
#define TUI_EVENT_RETURN_FOCUS_CHILD  4
#define TUI_EVENT_RETURN_FOCUS_NEXT    1
#define TUI_EVENT_RETURN_FOCUS_PREV   -1

typedef enum { 
	TUI_COLOR_BLACK,
	TUI_COLOR_RED,
	TUI_COLOR_GREEN,
	TUI_COLOR_YELLOW,
	TUI_COLOR_BLUE,
	TUI_COLOR_MAGENTA,
	TUI_COLOR_CYAN,
	TUI_COLOR_GRAY,
	TUI_COLOR_LIGHT_GRAY,
	TUI_COLOR_LIGHT_RED,
	TUI_COLOR_LIGHT_GREEN,
	TUI_COLOR_LIGHT_YELLOW,
	TUI_COLOR_LIGHT_BLUE,
	TUI_COLOR_LIGHT_MAGENTA,
	TUI_COLOR_LIGHT_CYAN,
	TUI_COLOR_WHYTE,
	TUI_COLOR_BLACK_BK,
	TUI_COLOR_RED_BK,
	TUI_COLOR_GREEN_BK,
	TUI_COLOR_YELLOW_BK,
	TUI_COLOR_BLUE_BK,
	TUI_COLOR_MAGENTA_BK,
	TUI_COLOR_CYAN_BK,
	TUI_COLOR_GRAY_BK,
	TUI_COLOR_LIGHT_GRAY_BK,
	TUI_COLOR_LIGHT_RED_BK,
	TUI_COLOR_LIGHT_GREEN_BK,
	TUI_COLOR_LIGHT_YELLOW_BK,
	TUI_COLOR_LIGHT_BLUE_BK,
	TUI_COLOR_LIGHT_MAGENTA_BK,
	TUI_COLOR_LIGHT_CYAN_BK,
	TUI_COLOR_WHYTE_BK,
	TUI_COLOR_RESET,
	TUI_FONT_BOLT,
	TUI_FONT_ITALIC,
	TUI_FONT_UNDERLINE,
	TUI_FONT_RESET,
	TUI_ATT_COUNT
}tuiAttributes_s;


typedef struct tui tui_s;

typedef void(*tuiFree_f)(void*);
typedef void(*tuiDraw_f)(tui_s* tui);
typedef int(*tuiEventMouse_f)(tui_s* tui, termMouse_s mouse);
typedef int(*tuiEventKey_f)(tui_s* tui, termKey_s key);
typedef int(*tuiEventInt_f)(tui_s* tui, int val);

typedef struct tuiPosition{
	int r;
	int c;
}tuiPosition_s;

typedef struct tuiSize{
	int width;
	int height;
}tuiSize_s;

typedef struct tui{
	utf8_t* name;
	utf_t* attribute[2];
	tuiPosition_s position;
	tuiSize_s size;
	struct tui* parent;
	struct tui** childs;
	void* usrdata;
	tuiFree_f free;
	tuiDraw_f draw;
	tuiEventMouse_f eventMouse;
	tuiEventKey_f eventKey;
	tuiEventInt_f eventFocus;
	utf_t clearchar;
	int id;
	int type;
	int border;
	int focusBorder;
	//int visible;
	int focused;
}tui_s;

/** begin use tui*/
void tui_begin(void);

/** end use tui*/
void tui_end(void);

/** get utf rappresent attribute*/
utf_t tui_att_get(tuiAttributes_s att);

/** cast a char rappresentation of border to utf_t rappresentation
 * /-----T---7
 * |     |   |
 * F-----+---3   
 * |     |   |
 * L-----1---j
 * (---------)
 * {---------}
 * .:
*/
utf_t tui_border_cast(int weight, char rappresentation);

/** draw horizontal line */
void tui_draw_hline(utf_t ch, unsigned count);

/** draw vertical line*/
void tui_draw_vline(tuiPosition_s st, utf_t ch, unsigned count);

/** create new tui object*/
tui_s* tui_new(tui_s* parent, int id, utf8_t* name, int border, int r, int c, int width, int height, void* userdata);

/** free a tui object and remove tui from parent*/
void tui_free(tui_s* tui);

/** set name object*/
void tui_name_set(tui_s* tui, utf8_t* name);

/** add an attribute to tui*/
void tui_attribute_add(tui_s* tui, int focus, utf_t att);

/** clear all attribute*/
void tui_attribute_clear(tui_s* tui);

/** print attribute*/
void tui_attribute_print(tui_s* tui);

/** add an child*/
void tui_child_add(tui_s* parent, tui_s* child);

/** remove a child*/
tui_s* tui_child_remove(tui_s* parent, tui_s* child);

/** find a child, if name is null is used id*/
tui_s* tui_child_find(tui_s* parent, utf8_t* name, int id);

/** get index of child*/
ssize_t tui_child_index(tui_s* parent, tui_s* child);

/** get area position */
tuiPosition_s tui_area_position(tui_s* tui);

/** get area size */
tuiSize_s tui_area_size(tui_s* tui);

/** goto to area*/
void tui_area_goto(tui_s* tui);

/** clear area, no border clear*/
void tui_clear_area(tui_s* tui);

/** clear */
void tui_clear(tui_s* tui);

/** clear all child*/
void tui_clear_all(tui_s* tui);

/** draw border*/
void tui_draw_border(tui_s* tui);

/** draw all child*/
void tui_draw(tui_s* tui);

/** move tui to position*/
void tui_move(tui_s* tui, int r, int c);

/** default event for key*/
int tui_default_event_key(__unused tui_s* tui, termKey_s key);

/** default event for focus*/
int tui_default_event_focus(tui_s* tui, int enable);



#endif
