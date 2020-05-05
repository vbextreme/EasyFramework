#include <ef/guiSimple.h>
#include <ef/err.h>
#include <ef/vector.h>

#define DEFAULT_FONT_SIZE 12

__private const char* appName;

__private void default_fallback_fonts(ftFonts_s* fonts){
	char* name[] = {
		"Symbola",
		"Source Code Pro",
		"Fira code",
		"inconsolata",
		"Dejavu",
		"monospace",
		NULL
	};

	for(size_t i = 0; name[i]; ++i){
		ftFont_s* font = ft_fonts_load(fonts, name[i], name[i]);
		if( font ){
			dbg_info("use fallback:%s", name[i]);
			ft_font_size(font, DEFAULT_FONT_SIZE, DEFAULT_FONT_SIZE);
			break;
		}
	}
}

__private void default_fonts(void){
	char* name[] = {
		"Dejavu",
		"inconsolata",
		"Source Code Pro",
		"Fira code",
		"monospace",
		NULL
	};

	ftFonts_s* fonts = ft_fonts_new(GUI_SIMPLE_DEFAULT_NAME_FONTS);
	
	for(size_t i = 0; name[i]; ++i){
		ftFont_s* font = ft_fonts_load(fonts, name[i], name[i]);
		if( font ){
			dbg_info("use font:%s", name[i]);
			ft_font_size(font, DEFAULT_FONT_SIZE, DEFAULT_FONT_SIZE);
			default_fallback_fonts(fonts);
			gui_resource_new(GUI_SIMPLE_DEFAULT_NAME_FONTS, fonts);
			return;
		}
	}
	err_fail("unable to continue without any fonts");
}

inline __private ftFonts_s* default_fonts_get(void){
	guiResource_s* res = gui_resource(GUI_SIMPLE_DEFAULT_NAME_FONTS);
	iassert(res);
	iassert( res->type == GUI_RESOURCE_FONTS );
	return res->fonts;
}

void gui_simple_begin(const char* appname){
	gui_begin();
	default_fonts();
	appName = appname;
}

void gui_simple_end(void){
	gui_end();
}

void gui_simple_show_all(gui_s* parent, int show){
	gui_show(parent,show);
	vector_foreach(parent->childs, i){
		gui_simple_show_all(parent->childs[i], show);
	}
}

void gui_simple_draw(gui_s* gui){
	gui_redraw(gui);
	gui_draw(gui);
}

inline __private void simple_layout(gui_s* gui){
	gui_themes(gui, appName);
	gui_div_apply_select(gui);
	gui_div_align(gui);
}

void gui_simple_apply_change(gui_s* main){
	if( main->type == GUI_TYPE_DIV ) simple_layout(main);
	gui_redraw(main);
	gui_draw(main);
	vector_foreach(main->childs, i){
		gui_simple_apply_change(main->childs[i]);
	}
}

gui_s* gui_simple_window_layout_vertical_new(const char* name, unsigned x, unsigned y, unsigned w, unsigned h){
	gui_s* gui = gui_div_attach( 
		gui_new(
			NULL, name, GUI_SIMPLE_CLASS_WINDOW, GUI_MODE_NORMAL,
			GUI_SIMPLE_DEFAULT_BORDER, x, y, w, h,
			GUI_SIMPLE_DEFAULT_BORDER_COLOR,
			gui_composite_add(
				gui_composite_new(GUI_SIMPLE_COMPOSITE_SIZE),
				gui_image_color_new(GUI_SIMPLE_DEFAULT_WINDOW_BACKGROUND_COLOR, w, h, 0)
			),
			0, NULL
		),
		gui_div_new(GUI_DIV_VERTICAL, NULL, GUI_DIV_FLAGS_FIT)
	);
	return gui;
}

gui_s* gui_simple_window_layout_horizontal_new(const char* name, unsigned x, unsigned y, unsigned w, unsigned h){
	gui_s* gui = gui_div_attach( 
		gui_new(
			NULL, name, GUI_SIMPLE_CLASS_WINDOW, GUI_MODE_NORMAL,
			GUI_SIMPLE_DEFAULT_BORDER, x, y, w, h,
			GUI_SIMPLE_DEFAULT_BORDER_COLOR,
			gui_composite_add(
				gui_composite_new(GUI_SIMPLE_COMPOSITE_SIZE),
				gui_image_color_new(GUI_SIMPLE_DEFAULT_WINDOW_BACKGROUND_COLOR, w, h, 0)
			),
			0, NULL
		),
		gui_div_new(GUI_DIV_HORIZONTAL, NULL, GUI_DIV_FLAGS_FIT)
	);
	return gui;
}

gui_s* gui_simple_window_layout_table_new(const char* name, unsigned x, unsigned y, unsigned w, unsigned h){
	gui_s* gui = gui_div_attach( 
		gui_new(
			NULL, name, GUI_SIMPLE_CLASS_WINDOW, GUI_MODE_NORMAL,
			GUI_SIMPLE_DEFAULT_BORDER, x, y, w, h,
			GUI_SIMPLE_DEFAULT_BORDER_COLOR,
			gui_composite_add(
				gui_composite_new(GUI_SIMPLE_COMPOSITE_SIZE),
				gui_image_color_new(GUI_SIMPLE_DEFAULT_WINDOW_BACKGROUND_COLOR, w, h, 0)
			),
			0, NULL
		),
		gui_div_new(GUI_DIV_TABLE, NULL, GUI_DIV_FLAGS_FIT)
	);
	return gui;
}

void gui_simple_layout_table_add(gui_s* parent, gui_s* gui, double w, double h, int newline){
	iassert(parent);
	iassert(parent->type == GUI_TYPE_DIV);
	guiDiv_s* div = parent->control;
	if( div->mode != GUI_DIV_TABLE ) return;
	gui_div_table_col_new(
		parent,
		newline ? gui_div_table_create_row(parent, h) : gui_div_table_row_last(parent), 
		gui,
		w,
		-1
	);
}

gui_s* gui_simple_label_new(gui_s* parent, const char* name, const utf8_t* caption){
	gui_s* gui = gui_label_attach( 
		gui_new(
			parent, name, GUI_SIMPLE_CLASS_LABEL, GUI_MODE_NORMAL,
			GUI_SIMPLE_DEFAULT_BORDER, 0, 0, GUI_SIMPLE_DEFAULT_CONTROL_W, GUI_SIMPLE_DEFAULT_CONTROL_H,
			GUI_SIMPLE_DEFAULT_BORDER_COLOR,
			gui_composite_add(
				gui_composite_new(GUI_SIMPLE_COMPOSITE_SIZE),
				gui_image_color_new(GUI_SIMPLE_DEFAULT_BACKGROUND_COLOR, GUI_SIMPLE_DEFAULT_CONTROL_W, GUI_SIMPLE_DEFAULT_CONTROL_H, 0)
			),
			0, NULL
		),
		gui_label_new(
			gui_caption_new(
				default_fonts_get(), 
				GUI_SIMPLE_DEFAULT_FOREGROUND, 
				GUI_CAPTION_CENTER_X | GUI_CAPTION_CENTER_Y
			)
		)
	);
	gui_themes(gui, appName);
	if( caption ) gui_label_text_set(gui, caption);
	return gui;
}

gui_s* gui_simple_button_new(gui_s* parent, const char* name, const utf8_t* caption, guiEvent_f onclick){
	gui_s* gui = gui_button_attach( 
		gui_new(
			parent, name, GUI_SIMPLE_CLASS_BUTTON, GUI_MODE_NORMAL,
			GUI_SIMPLE_DEFAULT_BORDER, 0, 0, GUI_SIMPLE_DEFAULT_CONTROL_W, GUI_SIMPLE_DEFAULT_CONTROL_H,
			GUI_SIMPLE_DEFAULT_BORDER_COLOR,
			gui_composite_add(
				gui_composite_new(GUI_SIMPLE_COMPOSITE_SIZE),
				gui_image_color_new(GUI_SIMPLE_DEFAULT_BACKGROUND_COLOR, GUI_SIMPLE_DEFAULT_CONTROL_W, GUI_SIMPLE_DEFAULT_CONTROL_H, 0)
			),
			0, NULL
		),
		gui_button_new(
			gui_caption_new(
				default_fonts_get(), 
				GUI_SIMPLE_DEFAULT_FOREGROUND, 
				GUI_CAPTION_CENTER_X | GUI_CAPTION_CENTER_Y
			),
			gui_image_color_new(GUI_SIMPLE_DEFAULT_ENABLE_COLOR, GUI_SIMPLE_DEFAULT_CONTROL_W, GUI_SIMPLE_DEFAULT_CONTROL_H, 0),
			gui_image_color_new(GUI_SIMPLE_DEFAULT_ACTIVE_COLOR, GUI_SIMPLE_DEFAULT_CONTROL_W, GUI_SIMPLE_DEFAULT_CONTROL_H, 0),
			onclick,
			GUI_BUTTON_FLAGS_HOVER
		)
	);
	gui_themes(gui, appName);
	if( caption ) gui_button_text_set(gui, caption);
	return gui;
}

gui_s* gui_simple_text_new(gui_s* parent, const char* name){
	gui_s* gui = gui_text_attach( 
		gui_new(
			parent, name, GUI_SIMPLE_CLASS_TEXT, GUI_MODE_NORMAL,
			GUI_SIMPLE_DEFAULT_BORDER, 0, 0, GUI_SIMPLE_DEFAULT_CONTROL_W, GUI_SIMPLE_DEFAULT_CONTROL_H,
			GUI_SIMPLE_DEFAULT_BORDER_COLOR,
			gui_composite_add(
				gui_composite_new(GUI_SIMPLE_COMPOSITE_SIZE),
				gui_image_color_new(GUI_SIMPLE_DEFAULT_BACKGROUND_COLOR, GUI_SIMPLE_DEFAULT_CONTROL_W, GUI_SIMPLE_DEFAULT_CONTROL_H, 0)
			),
			0, NULL
		),
		gui_text_new(
			default_fonts_get(), 
			GUI_SIMPLE_DEFAULT_FOREGROUND,
			GUI_SIMPLE_DEFAULT_ACTIVE_COLOR,
			GUI_SIMPLE_DEFAULT_CURSOR_COLOR,
			GUI_SIMPLE_DEFAULT_TAB,
			GUI_SIMPLE_DEFAULT_BLINK,
			GUI_TEXT_SCROLL_Y | GUI_TEXT_CUR_VISIBLE | GUI_TEXT_INSERT | GUI_TEXT_CURSOR_LIGHT
		)
	);
	gui_themes(gui, appName);
	return gui;
}

gui_s* gui_simple_bar_new(gui_s* parent, const char* name, const utf8_t* caption, double max){
	gui_s* gui = gui_bar_attach( 
		gui_new(
			parent, name, GUI_SIMPLE_CLASS_BAR, GUI_MODE_NORMAL,
			GUI_SIMPLE_DEFAULT_BORDER, 0, 0, GUI_SIMPLE_DEFAULT_CONTROL_W, GUI_SIMPLE_DEFAULT_CONTROL_H,
			GUI_SIMPLE_DEFAULT_BORDER_COLOR,
			gui_composite_add(
				gui_composite_new(GUI_SIMPLE_COMPOSITE_SIZE),
				gui_image_color_new(GUI_SIMPLE_DEFAULT_BACKGROUND_COLOR, GUI_SIMPLE_DEFAULT_CONTROL_W, GUI_SIMPLE_DEFAULT_CONTROL_H, 0)
			),
			0, NULL
		),
		gui_bar_new(
			gui_caption_new(
				default_fonts_get(), 
				GUI_SIMPLE_DEFAULT_FOREGROUND, 
				GUI_CAPTION_CENTER_X | GUI_CAPTION_CENTER_Y
			),
			gui_image_color_new(GUI_SIMPLE_DEFAULT_ENABLE_COLOR, GUI_SIMPLE_DEFAULT_CONTROL_W, GUI_SIMPLE_DEFAULT_CONTROL_H, 0),
			0.0,
			max,
			0.0,
			GUI_BAR_HORIZONTAL
		)
	);
	gui_themes(gui, appName);
	if( caption ) gui_bar_text_set(gui, caption, NULL);
	return gui;
}


