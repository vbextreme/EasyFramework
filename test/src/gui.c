#include "test.h"
#include <ef/gui.h>
#include <ef/guiLabel.h>
#include <ef/guiButton.h>

//#include <ef/ft.h>
//#include <ef/image.h>
//#include <ef/imageFiles.h>
//#include <ef/imageGif.h>
//#include <ef/media.h>
//#include <ef/xorg.h>
//#include <ef/os.h>
//#include <ef/utf8.h>
//#include <ef/deadpoll.h>
//#include <ef/delay.h>

/*@test -g --gui 'test gui'*/

void font_load(ftFonts_s* tfont,const char* name, const char* path,  unsigned size){
	ftFont_s* f = ft_fonts_load(tfont, path, name);
	if( !f ) err_fail("load font %s", path);
	if( ft_font_size(f, size, size) ) err_fail("size %s", name);
}

int main_exit(__unused gui_s* gui, __unused xorgEvent_s* ev){
	return -1;
}

int button_click(gui_s* gui, xorgEvent_s* ev){
	if( gui->type != GUI_TYPE_BUTTON ) err_fail("clang");
	if( ev->type == XORG_EVENT_CREATE ) err_fail("clang");

	dbg_error("BUTTON %u CLICK ON: %s", (uint32_t)gui->id, ev->type == XORG_EVENT_KEY_PRESS || ev->type == XORG_EVENT_KEY_RELEASE ? "key" : "mouse");
	return 0;
}

/*@fn*/
void test_gui(__unused const char* argA, __unused const char* argB){
	err_enable();
	gui_begin();

	ftFonts_s* tfont = ft_fonts_new();
	//font_load(tfont, "master", "Dejavu", 18);
	font_load(tfont, "masterFall", "FiraSans", 18);
	font_load(tfont, "fallback", "Symbola", 18);

	gui_s* main = gui_new(NULL, "test", NULL, 1, 50, 50, 400, 400, gui_color(255, 125,125,125), NULL);
	main->destroy = main_exit;

	gui_s* labl = 	gui_label_attach(
		gui_new(main, "labl", "label", 1, 10, 10, 130, 80, gui_color(255, 50, 50, 220), NULL),
		gui_label_new(tfont, 0, gui_color(255,40,40,40))
	);
	gui_label_text_set(labl, labl->control, U8("hello"));
	gui_label_redraw(labl, &labl->background, labl->control);

	gui_s* btn = gui_button_attach(
		gui_new(main, "but", "button", 1, 10, 100, 80, 50, gui_color(255, 80, 110, 80), NULL),
		gui_button_new(
			gui_label_new(tfont, 0, gui_color(255,40,40,40)),
			button_click
		)
	);
	btn->background.color = gui_color(255, 110,	80, 110);
	gui_label_text_set(btn, gui_button_label(btn->control), U8("click me")); 
	gui_button_redraw(btn, btn->control, 0);

	gui_show(main, 1);
	gui_show(labl, 1);
	gui_show(btn, 1);
	gui_focus(btn);

	gui_loop();

	gui_end();	
	err_restore();
}

