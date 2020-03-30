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
	dbg_info("ungetch: %d", key.ch);
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

	if( uinp && !rbuffer_read(uinp, &key) ){
		dbg_warning("get ungetch:%d", key.ch);
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

termMouse_s term_input_mouse(void){
	termMouse_s mouse = {0};
	int button = term_input_extend().ch;

	mouse.c = term_input_extend().ch - 33;
	mouse.r = term_input_extend().ch - 33;

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
		button = 0;
		sum = 99;
	}

	mouse.button = sum + (button & 3);
	button >>= 2;
	mouse.shift = button & 1;
	button >>= 1;
	mouse.meta = button & 1;	
	button >>= 1;
	mouse.control = button & 1;
	return mouse;
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
		dbg_info("term_input_utf8 ch:%d u:%d es:%d", (char)key[count-1].ch, key[count-1].ch, key[count-1].escape);

		if( key[count-1].ch == 0 ){
			//dbg_info(".ch == 0, return");
			return term_input_unroll(key, count);
		}
		
		kbd	= NULL;
		switch( trie_step(&kbd, &scan, key[count-1].ch) ){
			default: case TRIE_STEP_ERROR:
				dbg_info("step 0 on error");
				if( trie_step(&kbd, &scan, 1) == TRIE_STEP_END_NODE ){
					dbg_info("kbd.name:%s", kbd->name);
					key[0].ch = 0;
					key[0].escape = kbd->id + TERM_INPUT_EXTEND_OFFSET;
					term_input_ungetch(key[count-1]);
					return key[0];
				}
				//dbg_info("trie step error");
			return term_input_unroll(key, count);

			case TRIE_STEP_NEXT:
				if( term_kbhit() < 1 ){
					dbg_info("step 0 on next");
					if( trie_step((void**)&kbd, &scan, 1) == TRIE_STEP_END_NODE ){
						dbg_info("kbd.name:%s", kbd->name);
						key[0].ch = 0;
						key[0].escape = kbd->id + TERM_INPUT_EXTEND_OFFSET;
						return key[0];
					}
					//dbg_info("no kbhit, return");
					return term_input_unroll(key, count);
				}
			break;

			case TRIE_STEP_END_NODE:
				dbg_info("kbd.name:%s", kbd->name);
				key[0].ch = 0;
				key[0].escape = kbd->id + TERM_INPUT_EXTEND_OFFSET;
			return key[0];
		}
	}

	err_fail("oops");
	return key[0];
}

