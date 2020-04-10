#include "test.h"
#include <ef/gui.h>

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

/*@fn*/
void test_gui(__unused const char* argA, __unused const char* argB){
	err_enable();
	gui_begin();

	gui_s* main = gui_new(NULL, "test", NULL, 1, 50, 50, 400, 400, gui_color(255, 125,125,125), NULL, NULL);
	gui_show(main, 1);
	gui_focus(main);

	gui_loop();

	gui_end();	
	err_restore();
}

