#include <ef/tuiText.h>
#include <ef/memory.h>
#include <ef/str.h>
#include <ef/err.h>

void tui_text_event_draw(tui_s* tui){
	term_readline_draw(tui->usrdata);
}

int tui_text_event_key(tui_s* txt, termKey_s key){
	if( key.ch == TERM_INPUT_CHAR_ESC ) return TUI_EVENT_RETURN_FOCUS_PARENT;
	if( key.escape == TERM_KEY_NPAGE ) return TUI_EVENT_RETURN_FOCUS_NEXT;
	if( key.escape == TERM_KEY_PPAGE ) return TUI_EVENT_RETURN_FOCUS_PREV;
	term_readline_process_key(txt->usrdata, key);
	term_readline_draw(txt->usrdata);
	term_flush();
	return 0;
}
	
tui_s* tui_text_new(tui_s* parent, int id, utf8_t* name, int border, int r, int c, int width, int height){
	tui_s* tr = tui_new(parent, id, name, border, r, c, width, height, NULL);
	tr->free = (tuiFree_f)term_readline_free;
	tr->draw = tui_text_event_draw;
	tr->eventKey = tui_text_event_key;
	tr->eventFocus = tui_default_event_focus;
	tr->type = TUI_TYPE_BUTTON;
	
	tuiPosition_s p = tui_area_position(tr);
	tuiSize_s s = tui_area_size(tr);
	termReadLine_s* rl = term_readline_new(NULL, p.r, p.c, s.width, s.height);
	if( !rl ) err_fail("create readline");
	term_readline_mode(rl, TERM_READLINE_MODE_INSERT | TERM_READLINE_MODE_SCROLL_ROW);
	tr->usrdata = rl;
	
	return tr;
}

termReadLine_s* tui_text_readline(tui_s* tui){
	return tui->usrdata;
}
