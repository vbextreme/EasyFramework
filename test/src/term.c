#include "test.h"
#include <ef/term.h>
#include <ef/tui.h>
#include <ef/tuiRoot.h>
#include <ef/tuiWindow.h>
#include <ef/tuiLabel.h>
#include <ef/tuiButton.h>
#include <ef/tuiText.h>
#include <ef/tuiList.h>

#include <ef/memory.h>
#include <ef/file.h>
#include <ef/sig.h>

/*@test -E --term 'test terminal'*/

int btn_press(tui_s* tui, __unused int press){
	utf8_t* buttons[] = {
		U8("cancel"),
		U8("ok")
	};
	
	int ret = tui_window_msgbox(tui_root_get(tui), 7, U8("test pulsanti"), 1, 4, 4, 28, 10, U8("text mesage box"), buttons, 2);
	dbg_info("button pressed:%d", ret);

	return 0;
}

/*@fn*/
void test_term(__unused const char* argA, __unused const char* argB){
	err_enable();
	tui_begin();
	
	tui_s* root = tui_root_new();
	
	tui_s* win = tui_window_new(root, 1, U8("msg box"), 1, 3, 3, 30,15);
	win->focusBorder = 1;
	
	tuiPosition_s pos = tui_area_position(win);
	tuiSize_s size = tui_area_size(win);
	tui_s* lbl = tui_label_new(win, 2, NULL, 0, pos.r, pos.c, size.width, size.height);
	tui_label_set(lbl, U8("this is a message box, test of ef/tui"));

	tui_s* btn = tui_button_new(win, 3, NULL, 0, pos.r + size.height - 2, pos.c + size.width - 10, 9, 1);
	tui_button_set(btn, U8("click me"));
	tui_button_onpress_set(btn, btn_press, NULL);
	tui_attribute_add(btn, 1, tui_att_get(TUI_COLOR_LIGHT_GREEN_BK));
	tui_attribute_add(btn, 1, tui_att_get(TUI_COLOR_BLACK));

	tui_s* txt = tui_text_new(win, 4, NULL, 0, pos.r + size.height - 4, pos.c, 15, 3);
	term_readline_prompt_change(tui_text_readline(txt), U8("input:"));
	
	tui_s* lst = tui_list_new(win, 5, NULL, 0, pos.r+3, pos.c, 12,4);
	tui_list_option(lst, TUI_LIST_VERTICAL, TUI_LIST_CHECK);
	tui_list_add(lst, U8("a"), 0, NULL);
	tui_list_add(lst, U8("b"), 0, NULL);
	tui_list_add(lst, U8("c"), 0, NULL);
	tui_list_add(lst, U8("d"), 0, NULL);
	tui_list_add(lst, U8("e"), 0, NULL);
	
	tui_draw(win);

	tui_root_focus_set(root, btn);
	term_flush();
	tui_root_loop(root);
	tui_free(root);

	tui_end();
	


/*
	term_begin();
	term_endon_sigint();
	term_screen_size_enable();
	term_input_enable();
	
	__mem_free char* lcex = path_resolve("../../build/" TERM_EF_EXTEND);
	printf("../../build::%s\n",lcex);

	if( term_load(NULL, term_name()) ){
		dbg_error("ops load default term");
		err_print();
		term_input_disable();
		term_end();
		exit(1);
	}
	if( term_load(lcex, term_name_ef()) ){
		dbg_error("ops load ef term");
		err_print();
		term_input_disable();
		term_end();
		exit(1);
	}
	term_update_key();
	term_flush();
	
	term_mouse(1);
	//term_mouse_move(1);
	//term_mouse_focus(1);
	//term_print("\x1B[?10003h\x1B[?1006\x1B[?1015h");
	term_flush();

	termKey_s key;
	while( 1 ){
		term_print("showkey:");	
		term_flush();
		key = term_input_extend();
		if( key.escape == TERM_KEY_MOUSE ){
			//key = term_input_extend();
			//printf("b:%d ", key.ch);
			//key = term_input_extend();
			//printf("r:%d ", key.ch-33);
			//key = term_input_extend();
			//printf("c:%d \n", key.ch-33);
		

			
			termMouse_s mouse = term_input_mouse();
			printf("r:%d c:%d b:%d m:%d s:%d c:%d\n", mouse.r, mouse.c, mouse.button, mouse.meta, mouse.shift, mouse.control);
		}
		else{
			printf("utf:0x%0X escape:%d\n", key.ch, key.escape);
		}
		term_flush();
		if( key.ch == TERM_INPUT_CHAR_ESC || key.ch == '\n' ) break;
		//if( key.ch == TERM_INPUT_CHAR_ESC || key.ch == '\n' ) break;
	}
	term_input_disable();
	term_end();
	return;

	termReadLine_s* rl = term_readline_new(U8("inp: "), -1, -1, 12, 4);
	term_readline_draw(rl);
	
	char escColRes[256];
	char escColLGreen[256];
	char escColLRed[256];

	term_escapemk(escColRes, "color_reset");
	term_escapemk(escColLGreen, "color16_fg", TERM_COLOR_LIGHT_GREEN);
	term_escapemk(escColLRed, "color16_fg", TERM_COLOR_LIGHT_RED);
	utf_t ecolreset = term_readline_attribute_new(rl, escColRes);
	utf_t ecolgreen = term_readline_attribute_new(rl, escColLGreen);
	utf_t ecolred   = term_readline_attribute_new(rl, escColLRed);


	term_flush();

	int rlMode = TERM_READLINE_MODE_INSERT;
	rlMode |= TERM_READLINE_MODE_SCROLL_COL;
	rlMode |=  TERM_READLINE_MODE_SCROLL_ROW;
	term_readline_mode(rl, rlMode);

	while(1){
		termKey_s key = term_input_extend();
		if( key.ch == 0 && key.escape == 0 ) break;
		if( key.ch == TERM_INPUT_CHAR_ESC ) break;
		dbg_info("ch:%d es:%d", key.ch, key.escape);
		switch( key.escape ){
			case TERM_KEY_BACKSPACE:
				term_readline_backspace(rl);
			break;

			case TERM_KEY_DC:
				term_readline_del(rl);
			break;

			case TERM_KEY_LEFT:
				term_readline_cursor_prev(rl);
			break;

			case TERM_KEY_RIGHT:
				term_readline_cursor_next(rl);
			break;

			case TERM_KEY_SHIFT_LEFT:
				term_readline_cursor_scroll_left(rl);
			break;

			case TERM_KEY_SHIFT_RIGHT:
				term_readline_cursor_scroll_right(rl);
			break;

			case TERM_KEY_UP:
				term_readline_cursor_up(rl);
			break;

			case TERM_KEY_DOWN:
				term_readline_cursor_down(rl);
			break;

			case TERM_KEY_FIND:
				term_readline_cursor_home(rl);
			break;

			case TERM_KEY_SELECT:
				term_readline_cursor_end(rl);
			break;

			case TERM_KEY_F5:
				if( rlMode & TERM_READLINE_MODE_SCROLL_COL ){
					rlMode &= ~TERM_READLINE_MODE_SCROLL_COL;
				}
				else{
					rlMode |= TERM_READLINE_MODE_SCROLL_COL;
				}
				term_readline_mode(rl, rlMode);
			break;

			case TERM_KEY_F4:
				term_readline_put(rl,ecolred);
			break;
	
			case TERM_KEY_F3:
				term_readline_put(rl,ecolgreen);
			break;
			
			case TERM_KEY_F2:
				term_readline_put(rl, ecolreset);
			break;

			case TERM_KEY_IC:
				if( rlMode & TERM_READLINE_MODE_INSERT ){
					rlMode &= ~TERM_READLINE_MODE_INSERT;
					rlMode |= TERM_READLINE_MODE_REPLACE;
				}
				else if( rlMode & TERM_READLINE_MODE_REPLACE){
					rlMode &= ~TERM_READLINE_MODE_REPLACE;
				   	rlMode |= TERM_READLINE_MODE_INSERT;
				}
				term_readline_mode(rl, rlMode);
			break;

			case 0:
				if( key.ch ){
					term_readline_put(rl,key.ch);
				}
			break;
		}
		term_readline_draw(rl);
		term_flush();
	}

	term_readline_free(rl);
	puts("");
	term_flush();
	term_input_disable();
	term_end();

*/

/*
	term_ca_mode(1);
	term_gotorc(0,0);
	puts("hello");
	term_cursor_visible(0);
	term_flush();
	delay_ms(1000);
	term_cursor_visible(1);
	term_clear(TERM_CLEAR);
	term_ca_mode(0);

	term_clear(TERM_CLEAR);
	term_flush();

	int x,y;
	if( term_cursor_position(&y, &x) ){
		err_fail("cursor position");
	}
	printf("y:%d, x:%d\n", y, x);
	puts("next try in raw mode");
	term_flush();
	delay_ms(1000);
	term_clear(TERM_CLEAR);
	term_raw_enable();
	printf("raw mode, goto to x=4 y=3");
	term_gotorc(3,4);
	term_flush();
	if( term_cursor_position(&y, &x) ){
		term_raw_disable();
		err_fail("cursor position");
	}
	printf("y:%d, x:%d\n", y, x);
	delay_ms(1000);
	term_raw_disable();

	term_color16_bk(TERM_COLOR_WHYTE);
	term_color16_fg(TERM_COLOR_BLACK);
	term_print("black");
	term_color_reset();
	term_print(" ");


	char* colorn[] = {
		"black",
		"red",
		"green",
		"yellow",
		"blue",
		"magenta",
		"cyan",
		"gray", 
		NULL
	};
	
	for( size_t i = 0; colorn[i]; ++i){
		term_color16_fg(i);
		term_print(colorn[i]);
		term_color_reset();
		term_print("\n");		
	}
	for( size_t i = 0; colorn[i]; ++i){
		term_color16_fg(60+i);
		term_print("light ");
		term_print(colorn[i]);
		term_color_reset();
		term_print("\n");		
	}

	puts("");
	
	term_print("insert: ");
	term_flush();

	utf8_t inp[80] = {0};
	utf8_t* ins = inp;
	*ins = 0;
	termKey_s key;

	while( (key=term_input_extend()).ch ){
		if( key.ch == '\n' ) break;
		utf_putch(ins, key.ch);
		//printf("%s",ins);
		utf_t prev;
		ins = (utf8_t*)utf_next(&prev, ins);
		utf8_fputchar(stdout, prev);
		term_flush();
	}
	utf_putch(ins, 0);
	puts("");
	
	if( key.escape ){
		printf("escape  :%d\n", key.escape);
	}
	if( inp[0] ){
		printf("inserted:'%s'\n", inp);
	}
	//term_escapef("term_move", 50,50);
	term_input_disable();
	term_end();

*/

	err_restore();
}

