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

#define TESTEF_NAME       "testef"
#define TESTEF_MAIN_NAME  "main"
#define TESTEF_MAIN_X     50
#define TESTEF_MAIN_Y     50
#define TESTEF_MAIN_W     800
#define TESTEF_MAIN_H     600

#define TESTEF_LABEL_NAME "viewname"
#define TESTEF_LABEL_W    100.0
#define TESTEF_LABEL_H    15.0


/*@test -g --gui 'test gui'*/

int main_exit(__unused gui_s* gui, __unused xorgEvent_s* ev){
	return -1;
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
	
	gui_simple_apply_change(main);

	gui_show(main, 1);
	gui_show(lbl, 1);

	gui_loop();

	gui_simple_end();
	err_restore();
}

