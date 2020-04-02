#include <ef/tuiRoot.h>
#include <ef/memory.h>
#include <ef/vector.h>
#include <ef/str.h>
#include <ef/err.h>

tui_s* tui_root_new(void){
	tuiRoot_s* root = mem_new(tuiRoot_s);
	if( !root ) err_fail("malloc");

	unsigned w,h;
	term_screen_size_get(&h, &w);
	tui_s* tr = tui_new(NULL, 0, NULL, 0, 0, 0, w, h, root);
	
	tr->free = free;
	tr->eventKey = tui_default_event_key;
	tr->eventFocus = tui_default_event_focus;
	tr->type = TUI_TYPE_ROOT;
	root->focus = tr;
	return tr;
}

tui_s* tui_root_get(tui_s* any){
	while( any->parent ){
		any = any->parent;
	}
	iassert(any->type == TUI_TYPE_ROOT);
	return any;
}

tui_s* tui_root_focused(tui_s* tr){
	tuiRoot_s* root = tr->usrdata;
	return root->focus;
}

void tui_root_focus_set(tui_s* tr, tui_s* focus){
	tuiRoot_s* root = tr->usrdata;
	if( root->focus && root->focus->eventFocus ) root->focus->eventFocus(root->focus, 0);
	root->focus = focus;
	if( focus->eventFocus ) focus->eventFocus(focus, 1);
}

tui_s* tui_root_getin(tui_s* tr, int r, int c){
	vector_foreach(tr->childs, i){
		tui_s* ret = tui_root_getin(tr->childs[i], r, c);
		if( ret ) return ret;
	}

	dbg_info("%d.%s::mouse.r:%d mouse.c:%d pos.r:%d pos.c%d, size.h:%d size.w:%d", tr->id, (char*)tr->name, r, c, tr->position.r, tr->position.c, tr->size.height, tr->size.width);
	if( 
		r >= tr->position.r && c >= tr->position.c &&
		r <= tr->position.r + tr->size.height &&
		c <= tr->position.c + tr->size.width
	){
		return tr;
	}
	//never return NULL, mouse is always inside a root
	return NULL;
}	

__private void tui_root_focus_child_next(tui_s* tr){
	iassert(tr->type == TUI_TYPE_ROOT);
	tuiRoot_s* root = tr->usrdata;
	if( !root->focus || !root->focus->parent ) return;
	
	ssize_t index = tui_child_index(root->focus->parent, root->focus);
	if( index == -1 ) return;
	index = index + 1 < (ssize_t)vector_count(root->focus->parent->childs) ? index + 1 : 0; 
	
	tui_root_focus_set(tr, root->focus->parent->childs[index]);
}

__private void tui_root_focus_child_prev(tui_s* tr){
	iassert(tr->type == TUI_TYPE_ROOT);

	tuiRoot_s* root = tr->usrdata;
	if( !root->focus || !root->focus->parent ) return;

	ssize_t index = tui_child_index(root->focus->parent, root->focus);
	if( index == -1 ) return;
	index = index == 0 ? (ssize_t)vector_count(root->focus->parent->childs) - 1 : index - 1; 

	tui_root_focus_set(tr, root->focus->parent->childs[index]);
}

void tui_root_loop(tui_s* tr){
	iassert(tr->type == TUI_TYPE_ROOT);
	iassert(tr->parent == NULL);

	tuiRoot_s* root = tr->usrdata;
	while( root->focus ){		
		termKey_s key = term_input_extend();
		if( root->focus->eventKey ){
			int ret = root->focus->eventKey(root->focus, key);
			if( ret == TUI_EVENT_RETURN_EXIT ){
				break;
			}
			else if( ret == TUI_EVENT_RETURN_FOCUS_PARENT ){
				if( root->focus->parent ) tui_root_focus_set(tr, root->focus->parent);
			}
			else if( ret == TUI_EVENT_RETURN_FOCUS_CHILD ){
				if( vector_count(root->focus->childs) ) tui_root_focus_set(tr, root->focus->childs[0]);
			}
			else if( ret > 0 ){
				dbg_info("focus to next child");
				tui_root_focus_child_next(tr);
			}
			else if( ret < 0 ){
				tui_root_focus_child_prev(tr);	
			}
		}
	}
}

void tui_root_wait(tui_s* tui, tui_s* setFocus){
	tui_s* root = tui_root_get(tui);
	tui_s* old = tui_root_focused(root);

	tui_root_focus_set(root, tui);
	//tui_draw(tui);
	tui_root_focus_set(root, setFocus);
	term_flush();
	tui_root_loop(root);
	tui_clear(tui);
	tui_root_focused(root)->eventFocus = NULL;
	tui_root_focus_set(root, old);
	tui_free(tui);

	tui_draw(root);
	term_flush();
}

