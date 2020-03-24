#include <ef/list.h>
#include <ef/memory.h>
#include <ef/err.h>

/**********************/
/* simple linked list */
/**********************/

void* list_simple_new_raw(size_t size, void* data, listFree_f clean){
	listSimple_s* lst = mem_flexible_structure_new(listSimple_s, size, 1);
	if( lst == NULL ){
		err_pushno("malloc");
		return NULL;
	}
	lst->next = NULL;
	lst->clean = clean;
	lst->size = size;
	if( data ) memcpy(lst->data, data, size);
	return lst->data;
}

void list_simple_add_head(void* head, void* lst){
	void** r = (void**)head;
	listSimple_s* l = LIST_SIMPLE(lst);
	
	l->next = *r;
	*r = lst;
}

void list_simple_add_tail(void* head, void* lst){
	void** tail;
	for(tail = (void**)head; *tail; tail = &(LIST_SIMPLE(*tail)->next));
	*tail = lst;
}

void list_simple_add_before(void* head, void* before, void* lst){
	void** be;
	for(be = (void**)head; *be && *be != before; be = &(LIST_SIMPLE(*be)->next));
	LIST_SIMPLE(lst)->next = *be;
	*be = lst;
}

void list_simple_add_after(void* head, void* after, void* lst){
	void** h = (void**)head;
	if( *h == NULL ){
		list_simple_add_head(head, lst);
		return;
	}
	void* af;
	for(af = *h; LIST_SIMPLE(af)->next && af != after; af = LIST_SIMPLE(af)->next);
	LIST_SIMPLE(lst)->next = LIST_SIMPLE(af)->next;
	LIST_SIMPLE(af)->next = lst;
}

void* list_simple_extract(void* head, void* lst){
	void** be;
	for(be = (void**)head; *be && *be != lst; be = &(LIST_SIMPLE(*be)->next));
	if( !*be ) return NULL;
	*be = LIST_SIMPLE(lst)->next;
	LIST_SIMPLE(lst)->next = NULL;
	return lst;
}

void* list_simple_find_extract(void* head, void* userdata, listCmp_f fn){
	void** be;
	for(be = (void**)head; *be && fn(*be, userdata); be = &(LIST_SIMPLE(*be)->next));
	if( !*be ) return NULL;
	void* ret = *be;
	*be = LIST_SIMPLE(*be)->next;
	LIST_SIMPLE(ret)->next = NULL;
	return ret;
}

void list_simple_free(void* lst){
	listSimple_s* l = LIST_SIMPLE(lst);
	if( l->clean ) l->clean(lst);
	free(l);
}

void list_simple_free_auto(void* lst){
	list_simple_free(*(void**)lst);
}

void list_simple_all_free(void* lst){
	void* next;
	while( lst ){
		next = LIST_SIMPLE(lst)->next;
		list_simple_free(lst);
		lst = next;
	}
}

void list_simple_all_free_auto(void* head){
	list_simple_all_free(*(void**)head);
}

/*
void list_simple_add_head(listSimple_s** head, listSimple_s* lst){
	lst->next = *head;
	*head = lst;
}

void list_simple_add_tail(listSimple_s** head, listSimple_s* lst){
	listSimple_s** tail;
	for(tail = head; *tail; tail = &((*tail)->next));
	*tail = lst;
}

void list_simple_add_before(listSimple_s** head, const listSimple_s* before, listSimple_s* lst){
	listSimple_s** be;
	for(be = head; *be && *be != before; be = &((*be)->next));
	lst->next = *be;
	*be = lst;
}

void list_simple_add_after(listSimple_s** head, const listSimple_s* after, listSimple_s* lst){
	if( *head == NULL ){
		list_simple_add_head(head, lst);
		return;
	}
	listSimple_s* af;
	for(af = *head; af->next && af != after; af = af->next);
	lst->next = af->next;;
	af->next = lst;
}

listSimple_s* list_simple_extract(listSimple_s** head, listSimple_s* lst){
	listSimple_s** be;
	for(be = head; *be && *be != lst; be = &((*be)->next));
	*be = lst->next;
	lst->next = NULL;
	return lst;
}

void list_simple_free(listSimple_s* lst){
	if( lst->clean ) lst->clean(lst->data);
	free(lst);
}

void list_simple_free_all(listSimple_s* lst){
	listSimple_s* next;
	while( lst ){
		next = lst->next;
		list_simple_free(lst);
		lst = next;
	}
}
*/

/**********************/
/* double linked list */
/**********************/

void* list_doubly_new_raw(size_t size, void* data, listFree_f clean){
	listDouble_s* lst = mem_flexible_structure_new(listDouble_s, size, 1);
	if( lst == NULL ){
		err_pushno("malloc");
		return NULL;
	}
	lst->next = lst->data;
	lst->prev = lst->data;
	if( data ) memcpy(lst->data, data, size);
	lst->clean = clean;
	return lst->data;
}

void list_doubly_add_before(void* before, void* lst){
	listDouble_s* b = LIST_DOUBLY(before);
	listDouble_s* l = LIST_DOUBLY(lst);
	l->next = before;
	l->prev = b->prev;
	LIST_DOUBLY(l->prev)->next = lst;
	b->prev = lst;
}

void list_doubly_add_after(void* after, void* lst){
	listDouble_s* a = LIST_DOUBLY(after);
	listDouble_s* l = LIST_DOUBLY(lst);

	l->prev = after;
	l->next = a->next;
	a->next = lst;
	LIST_DOUBLY(l->next)->prev = lst;
}

void list_doubly_merge(void* a, void* b){
	listDouble_s* la = LIST_DOUBLY(a);
	listDouble_s* lb = LIST_DOUBLY(b);

	LIST_DOUBLY(lb->prev)->next = la->next;
	LIST_DOUBLY(la->next)->prev = lb->prev;
	la->next = b;
	lb->prev = a;
}

void* list_doubly_extract(void* lst){
	listDouble_s* l = LIST_DOUBLY(lst);
	LIST_DOUBLY(l->prev)->next = l->next;
	LIST_DOUBLY(l->next)->prev = l->prev;
	l->next = lst;
	l->prev = lst;	
	return lst;
}

void list_doubly_free(void* lst){
	if( !lst ) return;
	listDouble_s* l = LIST_DOUBLY(lst);
	if( l->clean ) l->clean(lst);
	free(l);
}

void list_doubly_all_free(void* lst){
	if( !lst ) return;
	void* endnode = lst;
	void* next;
	do{
		next = LIST_DOUBLY(lst)->next;
		list_doubly_free(lst);
		lst = next;
	}while( lst != endnode );
}

void list_doubly_all_free_auto(void* lst){
	list_doubly_all_free(*(void**)lst);
}

int list_doubly_only_root(void* lst){
	return LIST_DOUBLY(lst)->next == lst ? 1 : 0;
}

/*
listDouble_s* list_double_new(void* data, listFree_f clean){
	listDouble_s* lst = mem_new(listDouble_s);
	if( lst == NULL ) return NULL;
	lst->next = lst;
	lst->prev = lst;
	lst->data = data;
	lst->clean = clean;
	return lst;
}

void list_double_add_before(listDouble_s* before, listDouble_s* lst){
	lst->next = before;
	lst->prev = before->prev;
	lst->prev->next = lst;
	before->prev = lst;	
}

void list_double_add_after(listDouble_s* after, listDouble_s* lst){
	lst->prev = after;
	lst->next = after->next;
	after->next = lst;
	lst->next->prev = lst;
}

void list_double_merge(listDouble_s* a, listDouble_s* b){
	b->prev->next = a->next;
	a->next->prev = b->prev;
	a->next = b;
	b->prev = a;
}

listDouble_s* list_double_extract(listDouble_s* lst){
	lst->prev->next = lst->next;
	lst->next->prev = lst->prev;
	lst->next = lst;
	lst->prev = lst;	
	return lst;
}

void list_double_free(listDouble_s* lst){
	if( lst->clean ) lst->clean(lst->data);
	free(lst);
}

void list_double_free_all(listDouble_s* lst){
	listDouble_s* endnode = lst;
	listDouble_s* next;
	do{
		next = lst->next;
		list_double_free(lst);
		lst = next;
	}while( lst != endnode );
}
*/
