#include <ef/terminput.h>
#include <ef/termmode.h>
#include <ef/terminfo.h>
#include <ef/sig.h>
#include <ef/rbuffer.h>
#include <ef/trie.h>
#include <ef/os.h>
#include <ef/err.h>

__private volatile int updateResize = 0;
__private rbuffer_s* uinp; 


__private void sig_resize(__unused int sig, __unused siginfo_t* sinfo, __unused void* context){
	updateResize = 1;
}

err_t term_screen_size_get(unsigned* rows, unsigned* cols){
	winsize_s ws;
	if( term_winsize_get(&ws)) return -1;
	updateResize = 0;
    if( rows ) *rows = ws.ws_row;
    if( cols ) *cols = ws.ws_col;
	return 0;
}

void term_screen_size_enable(void){
	os_signal_set(NULL, SIGWINCH, sig_resize);
}

void term_flushin(void){
    tcflush(0, TCIFLUSH);
	int ch;
	while( (ch = getchar()) != EOF );
	if( uinp ) rbuffer_clear(uinp);	
}

void term_input_enable(void){
	utf_begin();
	term_raw_enable();
	uinp = rbuffer_new(sizeof(termKey_s), 1024);
	term_buff_same_screen();
	if( !uinp ) err_fail("rbuffer_new");
}

void term_input_disable(void){
	term_raw_disable();
	rbuffer_free(uinp);
	uinp = NULL;
	term_buff_end();
}

int term_kbhit(void){	
	int toread;
	if( ioctl(0, FIONREAD, &toread) ) toread = 0;
	toread += updateResize;
   	if( uinp ) toread += rbuffer_available_read(uinp);
    return toread;
}

void term_input_ungetch(termKey_s key){
	if( uinp && rbuffer_write(uinp, &key) ) err_fail("buffer ungetch full");
}

__private inline int term_input_char(void){
    int ch;
	if ( read(0,&ch,1) < 1 ){
		return -1;
	}	
    return ch;
}

termKey_s term_input_utf8(void){
	termKey_s key = { .ch = 0, .escape = 0};
   
	if( uinp && rbuffer_read(uinp, &key) > 0 ){
		return key;
	}

	if( updateResize ){
		//dbg_info("resize");
		updateResize = 0;
		key.escape = TERM_INPUT_EXTEND_SCREEN;
		return key;
	}
	
	int ch;
	utf8_t inch[64];
	size_t ni = 0;
	while( (ch=term_input_char()) != -1 ){
		if( ni > 62 ) err_fail("malformed input");
		inch[ni++] = ch;
		inch[ni]   = 0;
		if( utf_validate_n(inch, ni) ) continue;
		const utf8_t* next = utf_next(&key.ch, inch);
		if( next == inch ){
			err_fail("malformed input");
		}
		return key;
	}
    return key;
}

__private termKey_s term_input_unroll(termKey_s* unb, size_t count){
	for(size_t i = 1; i < count; ++i){
		term_input_ungetch(unb[i]);
	}
	return unb[0];
}

termKey_s term_input_extend(void){
	extern termInfo_s localTermInfo;
	trieElement_s* scan = &localTermInfo.caupcake->root;
	kbData_s* kbd;
	termKey_s key[64];
	size_t count = 0;

	while(1){
		key[count++] = term_input_utf8();
		//dbg_info("term_input_utf8 ch:%d u:%d es:%d", (char)key[count-1].ch, key[count-1].ch, key[count-1].escape);

		if( key[count-1].ch == 0 ){
			//dbg_info(".ch == 0, return");
			return term_input_unroll(key, count);
		}
		
		kbd	= NULL;
		switch( trie_step(&kbd, &scan, key[count-1].ch) ){
			default: case TRIE_STEP_ERROR:
				//dbg_info("step 0 on error");
				if( trie_step(&kbd, &scan, 1) == TRIE_STEP_END_NODE ){
					//dbg_info("kbd.name:%s", kbd->name);
					key[0].ch = 0;
					key[0].escape = kbd->id + TERM_INPUT_EXTEND_OFFSET;
					return key[0];
				}
				//dbg_info("trie step error");
			return term_input_unroll(key, count);

			case TRIE_STEP_NEXT:
				if( term_kbhit() < 1 ){
					//dbg_info("step 0 on next");
					if( trie_step((void**)&kbd, &scan, 1) == TRIE_STEP_END_NODE ){
						//dbg_info("kbd.name:%s", kbd->name);
						key[0].ch = 0;
						key[0].escape = kbd->id + TERM_INPUT_EXTEND_OFFSET;
						return key[0];
					}
					//dbg_info("no kbhit, return");
					return term_input_unroll(key, count);
				}
			break;

			case TRIE_STEP_END_NODE:
				//dbg_info("kbd.name:%s", kbd->name);
				key[0].ch = 0;
				key[0].escape = kbd->id + TERM_INPUT_EXTEND_OFFSET;
			return key[0];
		}
	}

	err_fail("oops");
	return key[0];
}

/*
#include <termll/terminput.h>
#include <termll/termmode.h>
#include <termll/terminfo.h>
#include <termios.h>
#include <fcntl.h>
#include <ef/os.h>
#include <ef/file.h>

#define TERM_STACK_CH 8

__private volatile int updateResize = 0;
__private char chstack[TERM_STACK_CH];
__private size_t idchstack;

void term_ungetch(char ch){
	iassert(idchstack < TERM_STACK_CH);
	chstack[idchstack++] = ch;
}

__private void sig_resize(__unused int sig, __unused siginfo_t* sinfo, __unused void* context){
	//dbg_warning("updateResize = 1");
	updateResize = 1;
}

err_t term_screen_size_get(unsigned* rows, unsigned* cols){
	winsize_s ws;
	const char* pts;
	__fd_autoclose int fd = -1;
	//dbg_warning("updateResize = 1");
	updateResize = 0;
    if( !(pts = ttyname(STDIN_FILENO)) || !(pts = ttyname(STDOUT_FILENO)) ){
		dbg_error("can't get tty name");
		return -1;
	}

	dbg_info("use tty %s", pts);
    while( (fd = open(pts, O_RDWR | O_NOCTTY)) == -1 && errno == EINTR);
    if( fd == -1){
		dbg_error("open tty %s", pts);
		dbg_errno();
		return -1;
	}

	if( ioctl(fd, TIOCGWINSZ, &ws) == -1 ){
		dbg_error("ioctl tiocswinsz");
		dbg_errno();
        return -1;
	}
	dbg_info("read %d %d", ws.ws_col, ws.ws_row);
    if( rows ) *rows = ws.ws_row;
    if( cols ) *cols = ws.ws_col;
	return 0;
}

void term_screen_size_enable(void){
	os_signal_set(NULL, SIGWINCH, sig_resize);
}

void term_flushin(void){
    tcflush(0, TCIFLUSH);
	int ch;
	while( (ch = getchar()) != EOF );
}

int term_screen_size_update(void){
	return updateResize;
}

__private inline int _kbhit(void){	
	int toread = 0;
	ioctl(0, FIONREAD, &toread);
    return toread;
}

int term_kbhit(void){
	return _kbhit() + updateResize + idchstack;
}

int term_get_ch(void){
    char ch;
	if( idchstack ){
		ch = chstack[--idchstack];
		return ch;
	}
	
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(0,&fds);
	if( updateResize ){
		//dbg_warning("exit updateResize");
		return EOF;
	}
	//dbg_error("enter");	
	if( select(1, &fds, NULL, NULL, NULL) <= 0 ){
		//dbg_error("select exit EOF");	 
		return EOF;
	}

	if ( read(0,&ch,1) < 1 ){
		//dbg_error("exit read EOF");	
		return EOF;
	}
	//dbg_error("exit");
    return ch;
}

utf_t term_get_utf(void){
	utf8_t in[12];
	size_t i = 0;
	int ch;
	utf_t ret;

	while(i < 11){
		if( (ch = term_get_ch()) == EOF ){
			dbg_warning("EOF");
			return UTF_NOT_VALID;
		}
		in[i++] = ch;
		in[i] = 0;
		if( utf_validate_n(in, i) ){
			continue;
		}
		const utf8_t* next = utf_next(&ret, in);	
		if( next == in ){
			dbg_error("get utf fail");
			return UTF_NOT_VALID;
		}
		return ret;
	}
	return UTF_NOT_VALID;
}

termMouse_s term_mouse_get(void){
	termMouse_s mouse;
	int button = term_get_ch();
	mouse.c = term_get_ch()-33;
	mouse.r = term_get_ch()-33;
	//dbg_info("button:%d", button);
	if( mouse.c < 0 ){
		mouse.c += 161 + 95;
	}
	if( mouse.r < 0 ){
		mouse.r += 161 + 95;
	}

	int sum = 0;
	if( button < 0 ){
		button = 0;
		sum = 99;
	}
	else if( button < 64 ){
		button -= 32;
	}
	else if( button < 90 ){
		button = 0;
		sum = 99;
	}
	else if( button < 127 ){
		button -= 96;
		sum = 4;
	}
	else{
		//dbg_error("invalid mouse value");
		button = 0;
		sum = 99;
	}

	//dbg_info("rebutton:%d + %d",button,sum);
	mouse.button = sum + (button & 3);
	button >>= 2;
	mouse.shift = button & 1;
	button >>= 1;
	mouse.control = button & 1;	
	button >>= 1;
	mouse.meta = button & 1;
	return mouse;
}

termKey_s term_get_ex(termInfo_s* ti){
    termKey_s key = { NULL, UTF_NOT_VALID };

	do{
		if( updateResize > 0 ){
			//dbg_warning("updateResize = 0");
			updateResize = 0;
			key.cnt = TERM_CNT_SCREEN_RESIZE;
			return key;
		}
	}while( (int)(key.utf = term_get_utf()) == UTF_NOT_VALID );
	
	if( key.utf == TERM_KEY_ESC ){
		int oldur = updateResize;
		void* out;
		char* last = NULL;

		updateResize = 0;
		trieElement_s* el = &ti->pac.root;
		int fmode = trie_search_ch(&out, &el, 0x1B);
		dbg_info("char '%s'->%d", term_escape_character(key.utf), fmode);

		while( term_kbhit() ){
			int ch = term_get_ch();
			fmode = trie_search_ch(&out, &el, ch);
			dbg_info("next char '%s'->%d", term_escape_character(ch), fmode);
			updateResize = 0;
			if( fmode == 1 ){
				dbg_info("multi found:%s", (char*)out);
				last = out;
			}
			else if( fmode < 0 ){
				out = NULL;
				dbg_info("multi not found");
				term_ungetch(ch);
				break;
			}
		}
		if( !out ){
			out = last;
		}
		updateResize = oldur;
		if( out ){
			key.cnt = out;
			return key;
		}
	}
	return key;
}

*/
