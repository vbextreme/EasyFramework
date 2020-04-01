#ifndef __EF_TUI_TEXT_H__
#define __EF_TUI_TEXT_H__

#include <ef/tui.h>

void tui_text_event_draw(tui_s* tui);

int tui_text_event_key(tui_s* txt, termKey_s key);

tui_s* tui_text_new(tui_s* parent, int id, utf8_t* name, int border, int r, int c, int width, int height);

termReadLine_s* tui_text_readline(tui_s* tui);


#endif
