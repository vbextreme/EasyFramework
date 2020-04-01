#ifndef __EF_TUI_H__
#define __EF_TUI_H__

/* TODO 
list
tab
*/

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

void tui_begin(void);
void tui_end(void);

utf_t tui_att_get(tuiAttributes_s att);

utf_t tui_border_cast(int weight, char rappresentation);

void tui_draw_hline(utf_t ch, unsigned count);

void tui_draw_vline(tuiPosition_s st, utf_t ch, unsigned count);

tui_s* tui_new(tui_s* parent, int id, utf8_t* name, int border, int r, int c, int width, int height, void* userdata);

void tui_free(tui_s* tui);

void tui_name_set(tui_s* tui, utf8_t* name);

void tui_attribute_add(tui_s* tui, int focus, utf_t att);

void tui_attribute_clear(tui_s* tui);

void tui_attribute_print(tui_s* tui);

void tui_child_add(tui_s* parent, tui_s* child);

tui_s* tui_child_remove(tui_s* parent, tui_s* child);

tui_s* tui_child_find(tui_s* parent, utf8_t* name, int id);

ssize_t tui_child_index(tui_s* parent, tui_s* child);

tuiPosition_s tui_area_position(tui_s* tui);

tuiSize_s tui_area_size(tui_s* tui);

void tui_area_goto(tui_s* tui);

void tui_clear_area(tui_s* tui);

void tui_clear(tui_s* tui);

void tui_draw_border(tui_s* tui);

void tui_draw(tui_s* tui);

//void tui_visible(tui_s* tui, int visible);

void tui_move(tui_s* tui, int r, int c);

int tui_default_event_key(__unused tui_s* tui, termKey_s key);

int tui_default_event_focus(tui_s* tui, int enable);



#endif
