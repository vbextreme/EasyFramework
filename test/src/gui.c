#include "test.h"
#include <ef/guiSimple.h>

#include <ef/ft.h>
#include <ef/image.h>
#include <ef/imageFiles.h>
#include <ef/imageGif.h>
#include <ef/media.h>
#include <ef/xorg.h>
#include <ef/os.h>
#include <ef/utf8.h>
#include <ef/delay.h>
#include <ef/spawn.h>

#define TESTEF_NAME       "testef"
#define TESTEF_MAIN_NAME  "main"
#define TESTEF_MAIN_X     50
#define TESTEF_MAIN_Y     50
#define TESTEF_MAIN_W     800
#define TESTEF_MAIN_H     600

#define TESTEF_LABEL_NAME "viewname"
#define TESTEF_LABEL_W    100.0
#define TESTEF_LABEL_H    15.0

#define TESTEF_BUTTONRR_NAME "reloadResources"
#define TESTEF_BUTTONRR_W    50.0
#define TESTEF_BUTTONRR_H    15.0

#define TESTEF_BUTTONRT_NAME "reloadTheme"
#define TESTEF_BUTTONRT_W    50.0
#define TESTEF_BUTTONRT_H    15.0

#define TESTEF_TEXT_NAME "input"
#define TESTEF_TEXT_W    50
#define TESTEF_TEXT_H    10

#define TESTEF_PAINT_NAME "paint"
#define TESTEF_PAINT_W    50
#define TESTEF_PAINT_H    10

#define TESTEF_OPTION_NAME      "opt"
#define TESTEF_OPTION_W         50
#define TESTEF_OPTION_H         30
#define TESTEF_OPTION_ELEMENT_H 15

#define TESTEF_THEME_RELOAD_PATH "~/testef.Xresources"

#define TESTEF_RELOAD "~/Project/c/EasyFramework/test/build/testef -g"

/*@test -g --gui 'test gui'*/

__private int main_exit(__unused gui_s* gui, __unused xorgEvent_s* ev){
	return -1;
}

__private int xresources_loaded(gui_s* lbl, xorgEvent_s* ev){
	int* fd = ev->data.data;
	gui_fd_unregister(*fd);
	spawn_waitfd_read(*fd, NULL, NULL);

	gui_label_text_set(lbl, U8("resources loaded"));
	gui_simple_draw(lbl);
	return 1;
}

__private int button_resources(gui_s* gui, __unused xorgEvent_s* ev){
	gui_s* lbl = gui->userdata;
	__mem_free char* path = path_resolve(TESTEF_THEME_RELOAD_PATH);
	if( !file_exists(path) ){
		gui_label_text_set(lbl, U8("file not exists"));
		gui_simple_draw(lbl);
		return 0;
	}
	
	gui_label_text_set(lbl, U8("loading resource..."));
	gui_simple_draw(lbl);

	__mem_free char* run = str_printf("xrdb -load %s", path);
	pid_t pid = spawn_shell(run, 1);
	int fdpid = spawn_waitfd(pid);
	gui_fd_register(lbl, fdpid, 0, xresources_loaded);

	return 0;
}

__private int button_themes(gui_s* gui, __unused xorgEvent_s* ev){
	gui_s* main = gui->userdata;
	gui_s* lbl = gui_by_name(main, TESTEF_LABEL_NAME, GUI_SIMPLE_CLASS_LABEL);
	gui_label_text_set(lbl, U8("reloading themes..."));
	gui_simple_draw(lbl);

	shell(TESTEF_RELOAD);
	return 0;
}

__private int paint_test(gui_s* gui, __unused xorgEvent_s* ev){
	g2dImage_s* img = gui->surface->img;
	gui_composite_redraw(gui, gui->img);

	g2dPoint_s p;
	g2dColor_t e = gui_color(255,0,0,255);

	p.x = gui->surface->img->w / 2;
	p.y = gui->surface->img->h / 2;
	unsigned r = gui->surface->img->h / 3;
	g2d_circle_fill_antialiased(img, &p, r, e);

	//g2d_supersampling_alpha_to(img, 1);
	//g2d_arc(img, &p, 30, 0, 360, c, 1);

	//p.x = 150;
	//g2d_circle_fill(img, &p, 30, c);

	//g2d_supersampling_to(img, 3);
	
	gui_draw(gui);
	return 0;
}


/*@fn*/
void test_gui(__unused const char* argA, __unused const char* argB){
	err_enable();
	gui_simple_begin(TESTEF_NAME);

	gui_s* main = gui_simple_window_layout_table_new(
		TESTEF_MAIN_NAME,
		TESTEF_MAIN_X,
		TESTEF_MAIN_Y,
		TESTEF_MAIN_W,
		TESTEF_MAIN_H
	); 
	main->destroy = main_exit;
	gui_border(main, 0);

	gui_s* lbl = gui_simple_label_new(main, TESTEF_LABEL_NAME, U8(TESTEF_NAME));
	gui_simple_layout_table_add(main, lbl, TESTEF_LABEL_W, TESTEF_LABEL_H, 1);

	gui_s* loadResource = gui_simple_button_new(main, TESTEF_BUTTONRR_NAME, U8(TESTEF_BUTTONRR_NAME), button_resources);
	gui_simple_layout_table_add(main, loadResource, TESTEF_BUTTONRR_W, TESTEF_BUTTONRR_H, 1);
	loadResource->userdata = lbl;

	gui_s* loadTheme = gui_simple_button_new(main, TESTEF_BUTTONRT_NAME, U8(TESTEF_BUTTONRT_NAME), button_themes);
	gui_simple_layout_table_add(main, loadTheme, TESTEF_BUTTONRR_W, TESTEF_BUTTONRR_H, 0);
	loadTheme->userdata = main;

	gui_s* text = gui_simple_text_new(main, TESTEF_TEXT_NAME);
	gui_simple_layout_table_add(main, text, TESTEF_TEXT_W, TESTEF_TEXT_H, 1);
	gui_text_print(text, U8("text box"));

	gui_s* paint = gui_simple_paint(main, TESTEF_PAINT_NAME);
	gui_simple_layout_table_add(main, paint, TESTEF_PAINT_W, TESTEF_PAINT_H, 0);
	paint->redraw = paint_test;

	gui_s* opt = gui_simple_option_new(main, TESTEF_OPTION_NAME);
	gui_simple_layout_table_add(main, opt, TESTEF_OPTION_W, TESTEF_OPTION_H, 1);
	gui_simple_option_add(opt, "opt.a", U8("option A"), TESTEF_OPTION_ELEMENT_H);
	gui_simple_option_add(opt, "opt.b", U8("option B"), TESTEF_OPTION_ELEMENT_H);
	gui_simple_option_add(opt, "opt.c", U8("option C"), TESTEF_OPTION_ELEMENT_H);

	gui_simple_apply_change(main);
	gui_simple_show_all(main, 1);
	gui_focus(loadResource);

	gui_loop();

	gui_simple_end();
	err_restore();
}

