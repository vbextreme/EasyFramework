#include <ef/tui.h>
#include <ef/memory.h>
#include <ef/vector.h>
#include <ef/str.h>
#include <ef/err.h>

#define TUI_CHILD_INITIAL 2
#define TUI_CHILD_RESIZE  2

/*

/-----T---7
|     |   |
F-----+---3   
|     |   |
L-----1---j

(---------)
{---------}

.:

*/

void tui_begin(void){
	term_begin();
	term_load(NULL, term_name());
	term_load(NULL, term_name_extend());
	term_load(NULL, term_name_ef());
	term_endon_sigint();
	term_screen_size_enable();
	term_input_enable();
	term_ca_mode(1);
	term_flush();
}

void tui_end(){
	term_color_reset();
	term_font_attribute(TERM_FONT_RESET);
	term_ca_mode(0);
	term_flush();
	term_input_disable();
	term_buff_end();
	term_end();
}

__private utf8_t* tuiBorder[] = {
	U8("-|/7LjFT+13"),
	U8("─│┌┐└┘├┬┼┴┤"),
	U8("━┃┏┓┗┛┣┳╋┻┫"),
	U8("═║╔╗╚╝╠╦╬╩╣"),
    U8("▀▍▛▜▙▟▚▞"),
	U8("╭╮╰╯"),
	U8("┍┎┝ ┑┠╁┰╀┱┡┒┕┖┙┚┞┟┢┥┦┧┨┩┪┭┮┯┲┵┶┷┸┹┺┽┾┿╂╃╄╅╆╇╈╉╊╼╽╾╿"),
	U8("╒╓╕╖╘╙╛╜╞╟╡╢╤╥╧╨╪╫"),
	U8("╌┆┅┇┈┊┉┋╌╎╍╏")
};

utf_t tui_border_cast(int weight, char rappresentation){
	if( weight < 1 || weight > 3 ) return 0;
	char* pch = strchr((char*)tuiBorder[0], rappresentation);
	if( !pch ) return 0;
	return tuiBorder[weight][pch-(char*)tuiBorder[0]];
}

void tui_draw_hline(utf_t ch, unsigned count){
	while(count-->0){
		term_print_u8(ch);
	}
}

void tui_draw_vline(tuiPosition_s st, utf_t ch, unsigned count){
	const unsigned en = st.r + count;
	for(size_t y = st.r; y < en; ++y){
		term_gotorc(y, st.c);
		term_print_u8(ch);
	}
}

tui_s* tui_new(void){
	tui_s* tui = mem_new(tui_s);
	tui->usrdata = NULL;
	tui->childs = vector_new(tui_s*, TUI_CHILD_INITIAL, TUI_CHILD_RESIZE);
	tui->free = NULL;
	tui->draw = NULL;
	tui->border = 0;
	tui->name = NULL;
	tui->attribute = NULL;
	tui->clearchar = ' ';
	tui->visible = 1;
	return tui;
}

void tui_free(tui_s* tui){
	vector_foreach(tui->childs, i){
		tui_free(tui->childs[i]);
	}
	vector_free(tui->childs);

	if( tui->free ) tui->free(tui->usrdata);
	if( tui->name ) free(tui->name);
	if( tui->attribute ) free(tui->attribute);
	free(tui);
}

void tui_name_set(tui_s* tui, utf8_t* name){
	if( tui->name ) free(tui->name);
	tui->name = U8(str_dup((char*)name, 0));
	if( !tui->name ) err_fail("create name");
}

void tui_attribute_add(tui_s* tui, char* att){
	if( tui->attribute ){
		char* old = tui->attribute;
		tui->attribute = str_printf("%s%s", old, att);
		if( !tui->attribute ) err_fail("add attribute");
		free(old);
	}
	else{
		tui->attribute = str_dup(att,0);
	}
}

void tui_attribute_clear(tui_s* tui){
	free(tui->attribute);
	tui->attribute = NULL;
}

void tui_child_add(tui_s* parent, tui_s* child){
	vector_push_back(parent->childs, child);
}

tui_s* tui_child_remove(tui_s* parent, tui_s* child){
	vector_foreach(parent->childs, i){
		if(parent->childs[i] == child){
			tui_s* ret = parent->childs[i];
			vector_remove(parent->childs, i);
			return ret;
		}
	}
	return NULL;
}

tui_s* tui_child_find(tui_s* parent, utf8_t* name, int id){
	vector_foreach(parent->childs, i){
		if( name ){
			if( !strcmp((char*)name, (char*)parent->childs[i]->name) ){
				return parent->childs[i];
			}
		}
		else if( id == parent->childs[i]->id ){
			return parent->childs[i];
		}
	}
	return NULL;
}

tuiPosition_s tui_area_position(tui_s* tui){
	tuiPosition_s ret = tui->position;
	if( tui->border ){
		++ret.c;
		++ret.r;
	}
	return ret;
}

tuiSize_s tui_area_size(tui_s* tui){
	tuiSize_s ret = tui->size;
	if( tui->border ){
		ret.width -= 2;
		ret.width -= 2;
	}
	return ret;
}

void tui_clear_area(tui_s* tui){
	if( !tui->visible ) return;
	const unsigned h = tui->size.height + tui->position.r;
	const unsigned ha = tui->border ? h-2 : h;
	const unsigned w = tui->size.width + tui->position.c;
	const unsigned wa = tui->border ? w-2 : w;
	const unsigned xs = tui->border ? tui->position.c + 1 : tui->position.c;
	const unsigned ys = tui->border ? tui->position.r + 1 : tui->position.r;

	if( tui->attribute ) term_print(tui->attribute);

	for(unsigned y = ys; y < ha; ++y){
		term_gotorc(y,xs);
		for( unsigned x = xs; x < wa; ++x){
			term_print_u8(tui->clearchar);
		}
	}

	term_gotorc(ys, xs);
}

void tui_clear(tui_s* tui){
	if( !tui->visible ) return;
	const unsigned h = tui->size.height + tui->position.r;
	const unsigned w = tui->size.width + tui->position.c;
	const unsigned xs = tui->border ? tui->position.c + 1 : tui->position.c;
	const unsigned ys = tui->border ? tui->position.r + 1 : tui->position.r;

	if( tui->attribute ) term_print(tui->attribute);

	for(unsigned y = ys; y < h; ++y){
		term_gotorc(y,xs);
		for( unsigned x = xs; x < w; ++x){
			term_print_u8(tui->clearchar);
		}
	}

	term_gotorc(ys, xs);
}

void tui_draw_border(tui_s* tui){
	if( !tui->visible ) return;
	if( !tui->border ) return;
	if( tui->attribute ) term_print(tui->attribute);
	
	utf_t h = tui_border_cast(tui->border, '-');
	utf_t v = tui_border_cast(tui->border, '|');
	
	term_gotorc(tui->position.r, tui->position.c);
	term_print_u8(tui_border_cast(tui->border, '/'));
	tui_draw_hline(h, tui->size.width - 2);
	term_print_u8(tui_border_cast(tui->border, '7'));

	term_gotorc(tui->position.r + tui->size.height, tui->position.c);
	term_print_u8(tui_border_cast(tui->border, 'L'));
	tui_draw_hline(h, tui->size.width - 2);
	term_print_u8(tui_border_cast(tui->border, 'j'));

	tui_draw_vline((tuiPosition_s){.r = tui->position.r + 1, .c = tui->position.c}, v, tui->size.height - 2);
	tui_draw_vline((tuiPosition_s){.r = tui->position.r + 1, .c = tui->position.c + tui->size.width}, v, tui->size.height - 2);
}

void tui_draw(tui_s* tui){
	if( !tui->visible ) return;
	if( tui->border ) tui_draw_border(tui) ;
	if( tui->draw ) tui->draw(tui);
	vector_foreach(tui->childs,i){
		tui_draw(tui->childs[i]);
	}
}

void tui_visible(tui_s* tui, int visible){
	if( tui->visible == visible ) return;

	if( visible ){
		tui->visible = 1;
		tui_draw(tui);	
	}
	else{	
		tui_clear(tui);
		tui->visible = 0;
		if( tui->parent ) tui_draw(tui->parent);	
	}
}














