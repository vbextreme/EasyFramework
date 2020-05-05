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
	
	gui_s* lbl = gui_simple_label_new(main, TESTEF_LABEL_NAME, U8(TESTEF_NAME));
	gui_simple_layout_table_add(main, lbl, TESTEF_LABEL_W, TESTEF_LABEL_H, 1);

	gui_s* loadResource = gui_simple_button_new(main, TESTEF_BUTTONRR_NAME, U8(TESTEF_BUTTONRR_NAME), button_resources);
	gui_simple_layout_table_add(main, loadResource, TESTEF_BUTTONRR_W, TESTEF_BUTTONRR_H, 1);
	loadResource->userdata = lbl;

	gui_s* loadTheme = gui_simple_button_new(main, TESTEF_BUTTONRT_NAME, U8(TESTEF_BUTTONRT_NAME), button_themes);
	gui_simple_layout_table_add(main, loadTheme, TESTEF_BUTTONRR_W, TESTEF_BUTTONRR_H, 0);
	loadTheme->userdata = main;

	gui_simple_apply_change(main);
	gui_simple_show_all(main, 1);
	gui_focus(loadResource);

	gui_loop();

	gui_simple_end();
	err_restore();
}

