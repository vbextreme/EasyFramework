#ifndef __EF_TUI_LIST_H__
#define __EF_TUI_LIST_H__

#include <ef/tui.h>


typedef enum { TUI_LIST_VERTICAL, TUI_LIST_HORIZONTAL, TUI_LIST_GRID } tuiListMode_e;
typedef enum { TUI_LIST_NORMAL, TUI_LIST_CHECK, TUI_LIST_OPTION } tuiListType_e;

typedef struct tuiListElement{
	utf8_t* name;
	int val;
	int r;
	int c;
	void* usrdata;
	tuiEventInt_f onpress;
}tuiListElement_s;

typedef struct tuiList{
	tuiListElement_s* elements;
	tuiListMode_e mode;
	tuiListType_e type;
	unsigned sel;
	size_t scrollRow;
}tuiList_s;

#define TUI_LIST_CHECKED_TRUE  U8("☑")
#define TUI_LIST_CHECKED_FALSE U8("☐")
#define TUI_LIST_OPTION_TRUE    U8("◉")
#define TUI_LIST_OPTION_FALSE   U8("⊙")

/** event draw*/
void tui_list_event_draw(tui_s* tui);

/** event key*/
int tui_list_event_key(tui_s* tui, termKey_s key);

/** event focus*/
int tui_list_event_focus(tui_s* tui, int enable);

/** create new list*/
tui_s* tui_list_new(tui_s* parent, int id, utf8_t* name, int border, int r, int c, int width, int height);

/** add list element */
void tui_list_add(tui_s* tui, const utf8_t* name, int val, void* userdata, tuiEventInt_f fn);

/** get list element*/
tuiListElement_s* tui_list_element(tui_s* tui, unsigned id);

/** get count list elements*/
size_t tui_list_element_count(tui_s* tui);

/** clear all elements*/
void tui_list_clear(tui_s* tui);

/** set mode list*/
void tui_list_option(tui_s* tui, tuiListMode_e mode, tuiListType_e type);

#endif
