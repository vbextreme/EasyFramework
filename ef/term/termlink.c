#include <ef/type.h>
#include <ef/sig.h>
#include <ef/memory.h>
#include <ef/file.h>
#include <ef/str.h>
#include <ef/delay.h>
#include <ef/err.h>
#include <ef/terminfo.h>
#include <ef/termcapstr.h>
#include <ef/termlink.h>
#include <ef/termmode.h>
#include <ef/terminput.h>

__private int camode;

#define def_cap_val_null(CAP) static tiData_s* em = NULL;\
	if( !em ){\
		em = term_info(CAP); \
		if( !em ) err_fail("%s", CAP);\
	}\
	tvariable_s var[10] = { \
		[1].type = 0, [1].l = 0,\
   	};\
	term_escape_make_print(em->str, var)

#define def_cap_val_bool(VAL, CAP_FALSE, CAP_TRUE) static tiData_s* em[2] = {0};\
	if( VAL < 0 || !em[0] ){\
		em[0] = term_info(CAP_TRUE);\
		em[1] = term_info(CAP_FALSE);\
		if( !em[0] || !em[1] ){\
			err_fail("%s || %s", CAP_FALSE, CAP_TRUE);\
		}\
		if( VAL < 0 ) return;\
	}\
	tvariable_s var[10] = { [1].type = 0, [1].l = 0 };\
	term_escape_make_print(em[!VAL]->str, var)

#define def_cap_val_int(VAL, CAP) static tiData_s* em = NULL;\
	if( VAL < 0 || !em ){\
		em = term_info(CAP);\
		if( !em ) err_fail("%s", CAP);\
		if( VAL < 0 ) return;\
	}\
	tvariable_s var[10] = { \
		[1].type = 0, [1].l = VAL\
   	};\
	term_escape_make_print(em->str, var)

#define def_cap_val_int_int(VAL1, VAL2, CAP) static tiData_s* em = NULL;\
	if( VAL1 < 0 || VAL2 <0 || !em ){\
		em = term_info(CAP);\
		if( !em ) err_fail("%s", CAP);\
		if( VAL1 < 0 || VAL2 < 0 ) return;\
	}\
	tvariable_s var[10] = { \
		[1].type = 0, [1].l = VAL1,\
		[2].type = 0, [2].l = VAL2\
   	};\
	term_escape_make_print(em->str, var)

__private void term_ca_mode_raw(int ee){
	def_cap_val_bool(ee, cap_exit_ca_mode, cap_enter_ca_mode);
}

void term_ca_mode(int ee){
	if( !ee && camode ){
		term_ca_mode_raw(ee);
		camode = ee;
	}
	else if( ee && !camode ){
		term_ca_mode_raw(ee);
		camode = ee;
	}
}

void term_gotorc(int r, int c){
	def_cap_val_int_int(r, c, cap_cursor_address);
}

void term_clear(termClearMode_e mode){
	static tiData_s* em[TERM_CLEAR_COUNT] = {0};

	if( mode == TERM_CLEAR_RELOAD || !em[TERM_CLEAR] ){
		char* capn[] = {
			[TERM_CLEAR] = cap_clear_screen,
			[TERM_CLEAR_BEGIN_LINE]= cap_clr_bol,
			[TERM_CLEAR_END_OF_LINE]= cap_clr_eol,
			[TERM_CLEAR_END_OF_SCREEN]= cap_clr_eos,
			NULL
		};
		for( size_t i = 0; capn[i]; ++i){
			em[i] = term_info(capn[i]); 
			if( !em[i] ) err_fail("%s", capn[i]);
		}
	}
	if( mode <= TERM_CLEAR_RELOAD || mode >= TERM_CLEAR_COUNT ) return;
	tvariable_s var[10] = { 
		[1].type = 0, [1].l = 0
   	};
	term_escape_make_print(em[mode]->str, var);
}

void term_cursor(termCursor_e mode){
	static tiData_s* em[TERM_CURSOR_COUNT] = {0};

	if( mode == TERM_CURSOR_RELOAD || !em[TERM_CURSOR_LEFT] ){
		char* capn[] = {
			[TERM_CURSOR_LEFT] = cap_cursor_left,
			[TERM_CURSOR_RIGHT]= cap_cursor_right,
			[TERM_CURSOR_DOWN] = cap_cursor_down,
			[TERM_CURSOR_UP]   = cap_cursor_up,
			[TERM_CURSOR_HOME] = cap_cursor_home,
			[TERM_CURSOR_END]  = cap_cursor_to_ll,
			NULL
		};
		for( size_t i = 0; capn[i]; ++i){
			em[i] = term_info(capn[i]); 
			if( !em[i] ) err_fail("%s", capn[i]);
		}
	}
	if( mode <= TERM_CURSOR_RELOAD || mode >= TERM_CURSOR_COUNT ) return;
	tvariable_s var[10] = { 
		[1].type = 0, [1].l = 0
   	};
	term_escape_make_print(em[mode]->str, var);
}

void term_cursor_visible(int v){
	def_cap_val_bool(v, cap_cursor_invisible, cap_cursor_visible);
}

void term_cursor_mem(int store){
	def_cap_val_bool(store, cap_restore_cursor, cap_save_cursor);
}

void term_color16_bk(termColor_e color){
	def_cap_val_int(color, "color16_bk");
}

void term_color16_fg(termColor_e color){
	def_cap_val_int(color, "color16_fg");
}

void term_color256_bk(int color){
	def_cap_val_int(color, "color256_bk");
}

void term_color256_fg(int color){
	def_cap_val_int(color, "color256_fg");
}

void term_color24_bk(unsigned char r, unsigned char g, unsigned char b){
	static tiData_s* em = NULL;
	
	if( !em ){
		em = term_info("color24_bk");
		if( !em ) err_fail("%s", "color24_bk");
	}
	tvariable_s var[10] = {
		[1].type = 0, [1].l = r,
		[2].type = 0, [2].l = g,
		[3].type = 0, [3].l = b,
   	};
	term_escape_make_print(em->str, var);
}

void term_color24_fg(unsigned char r, unsigned char g, unsigned char b){
	static tiData_s* em = NULL;
	
	if( !em ){
		em = term_info("color24_fg");
		if( !em ) err_fail("%s", "color24_fg");
	}
	tvariable_s var[10] = {
		[1].type = 0, [1].l = r,
		[2].type = 0, [2].l = g,
		[3].type = 0, [3].l = b,
   	};
	term_escape_make_print(em->str, var);
}

void term_color_reset(void){
	def_cap_val_null("color_reset");
}

void term_font_attribute(termFontAttribute_e att){
	static tiData_s* em[TERM_FONT_COUNT] = {0};

	if( att == TERM_FONT_RELOAD || !em[0] ){
		char* capn[] = {
			[TERM_FONT_BOLT]      = cap_enter_bold_mode,
			[TERM_FONT_ITALIC]    = cap_enter_italics_mode,
			[TERM_FONT_UNDERLINE] = cap_enter_underline_mode,
			[TERM_FONT_RESET]     = cap_exit_attribute_mode,
			NULL
		};
		for( size_t i = 0; capn[i]; ++i){
			em[i] = term_info(capn[i]); 
			if( !em[i] ) err_fail("%s", capn[i]);
		}
	}
	if( att <= TERM_FONT_RELOAD || att >= TERM_FONT_COUNT ) return;
	tvariable_s var[10] = { 
		[1].type = 0, [1].l = 0
   	};
	term_escape_make_print(em[att]->str, var);
}

void term_change_scroll_region(int startRow, int endRow){
	def_cap_val_int_int(startRow, endRow, cap_change_scroll_region);
}

void term_resize(int w, int h){
	def_cap_val_int_int(w, h, "term_resize");
}

void term_mouse(int enable){
	def_cap_val_bool(enable, "mouse_disable", "mouse_enable");
}

void term_mouse_move(int enable){
	def_cap_val_bool(enable, "mouse_move_disable", "mouse_move_enable");
}

void term_mouse_focus(int enable){
	def_cap_val_bool(enable, "mouse_focus_disable", "mouse_focus_enable");
}

err_t term_cursor_position(int* r, int* c){
    const char *dev;
	termios_s oldtio;
	termios_s newtio;
    __fd_close int fd = -1;

    if( !(dev = ttyname(STDIN_FILENO)) || !(dev = ttyname(STDOUT_FILENO)) ){
		err_pushno("can't get tty name");
		return -1;
	}

    while( (fd = open(dev, O_RDWR | O_NOCTTY)) == -1 && errno == EINTR);
    if( fd == -1){
		err_pushno("open tty %s", dev);
		return -1;
	}

    if( term_settings_fdget(fd, &oldtio) ){
		err_pushno("tcgetattr");
		return -1;
	}

	newtio = oldtio;
	newtio.c_lflag &= ~ICANON;
    newtio.c_lflag &= ~ECHO;
    newtio.c_cflag &= ~CREAD;
	if( term_settings_fdset(fd, &newtio) ){
		err_pushno("tcsetattr");
		return -1;
	}
	
	char mkuser7[128];
	term_escapemk(mkuser7, cap_user7);
	size_t len = strlen(mkuser7);
	if( write(fd, mkuser7, len) != (ssize_t)len ){
		err_pushno("write user7");
		term_settings_fdset(fd, &oldtio);
		return -1;
	}

	int ch;
	if( fd_read(fd, &ch, 1) != 1 || ch != 0x1B ){
		err_push("not read esc");
		term_settings_fdset(fd, &oldtio);
		return -1;
	}
	if( fd_read(fd, &ch, 1) != 1 || ch != '[' ){
		err_push("not read [ but (%d)%c", ch, ch);
		term_settings_fdset(fd, &oldtio);
		return -1;
	}

	*r = 0;
	while( fd_read(fd, &ch, 1) == 1 && ch >= '0' && ch <= '9' ){
		*r = 10 * *r + (ch-'0');
	}
	
	if( ch != ';' ){
		err_pushno("not read ;");
		term_settings_fdset(fd, &oldtio);
		return -1;
	}

	*c = 0;
	while( fd_read(fd, &ch, 1) == 1 && ch >= '0' && ch <= '9' ){
		*c = 10 * *c + (ch-'0');
	}
	
	if( ch != 'R' ){
		err_pushno("not read R");
		term_settings_fdset(fd, &oldtio);
		return -1;
	}
	
	--(*r);
	--(*c);

	term_settings_fdset(fd, &oldtio);
	return 0;
}

__private void term_sigint(int sig, __unused siginfo_s* u, __unused void* un){
	if( sig != SIGINT ) return;
	term_color_reset();
	term_font_attribute(TERM_FONT_RESET);
	term_ca_mode(0);
	term_flush();
	term_input_disable();
	term_buff_end();
	term_end();
	exit(1);
}

void term_endon_sigint(void){
	os_signal_set(NULL, SIGINT, term_sigint);
}	
