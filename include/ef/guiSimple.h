#ifndef __EF_GUI_SIMPLE_H__
#define __EF_GUI_SIMPLE_H__

#include <ef/gui.h>
#include <ef/guiImage.h>
#include <ef/guiResources.h>
#include <ef/guiCaption.h>
#include <ef/guiLabel.h>
#include <ef/guiButton.h>
#include <ef/guiBar.h>
#include <ef/guiText.h>
#include <ef/guiDiv.h>

#define GUI_SIMPLE_DEFAULT_NAME_FONTS              "defaultFT"

#define GUI_SIMPLE_DEFAULT_BORDER_COLOR            gui_color(255,0x8D,0x8D,0x8D)
#define GUI_SIMPLE_DEFAULT_WINDOW_BACKGROUND_COLOR gui_color(255,0xEF,0xEF,0xEF)
#define GUI_SIMPLE_DEFAULT_BACKGROUND_COLOR        gui_color(255,0xDB,0xDB,0xDB)
#define GUI_SIMPLE_DEFAULT_FOREGROUND              gui_color(255,0x00,0x00,0x00)
#define GUI_SIMPLE_DEFAULT_ACTIVE_COLOR            gui_color(255,0xE0,0xE0,0xE0)
#define GUI_SIMPLE_DEFAULT_ENABLE_COLOR            gui_color(255,0x9E,0x9E,0x9E)
#define GUI_SIMPLE_DEFAULT_CURSOR_COLOR            gui_color(255,0x21,0x21,0x21)
#define GUI_SIMPLE_DEFAULT_BLINK                   600

#define GUI_SIMPLE_DEFAULT_BORDER                  1
#define GUI_SIMPLE_DEFAULT_CONTROL_W               100
#define GUI_SIMPLE_DEFAULT_CONTROL_H               100
#define GUI_SIMPLE_DEFAULT_TAB                     4

#define GUI_SIMPLE_CLASS_WINDOW                    "window"
#define GUI_SIMPLE_CLASS_LABEL                     "label"
#define GUI_SIMPLE_CLASS_BUTTON                    "button"
#define GUI_SIMPLE_CLASS_TEXT                      "text"
#define GUI_SIMPLE_CLASS_BAR                       "bar"

#define GUI_SIMPLE_COMPOSITE_SIZE                  2

void gui_simple_begin(const char* appname);
void gui_simple_end(void);
void gui_simple_show_all(gui_s* parent, int show);
void gui_simple_draw(gui_s* gui);
void gui_simple_apply_change(gui_s* main);

gui_s* gui_simple_window_layout_vertical_new(const char* name, unsigned x, unsigned y, unsigned w, unsigned h);
gui_s* gui_simple_window_layout_horizontal_new(const char* name, unsigned x, unsigned y, unsigned w, unsigned h);
gui_s* gui_simple_window_layout_table_new(const char* name, unsigned x, unsigned y, unsigned w, unsigned h);
void gui_simple_layout_table_add(gui_s* parent, gui_s* gui, double w, double h, int newline);
gui_s* gui_simple_paint(gui_s* parent, const char* name);
gui_s* gui_simple_label_new(gui_s* parent, const char* name, const utf8_t* caption);
gui_s* gui_simple_button_new(gui_s* parent, const char* name, const utf8_t* caption, guiEvent_f onclick);
gui_s* gui_simple_text_new(gui_s* parent, const char* name);
gui_s* gui_simple_bar_new(gui_s* parent, const char* name, const utf8_t* caption, double max);

#endif
