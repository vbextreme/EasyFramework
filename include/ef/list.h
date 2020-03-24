#ifndef __EF_LIST_H__
#define __EF_LIST_H__

#include <ef/type.h>

/** callback clean function*/
typedef void(*listFree_f)(void* data);

/** callback list compare */
typedef int(*listCmp_f)(void* a, void* b);

/**************************/
/*** simple linked list ***/
/**************************/

typedef struct listSimple{
	void* next;       /**< next element*/
	listFree_f clean; /**< cleanup function*/
	size_t size;      /**< size of data*/
	void* data[0];    /**< user data*/
}listSimple_s;

#define LIST_SIMPLE(L) ((listSimple_s*)(ADDR(L)-sizeof(listSimple_s)))

/** create new list
 * @param size size of element
 * @param data userdata is copied to new list element
 * @param clean cleanup function, can pass NULL
 * @return pointer to data of new list or NULL for error
 */
void* list_simple_new_raw(size_t size, void* data, listFree_f clean);

/** list simple new with auto sizeof */
#define list_simple_new(TYPE, DATA, CLEAN) (TYPE*)list_simple_new_raw(sizeof(TYPE), DATA, CLEAN)

/** add to head
 * @param head address of pointer to data head list
 * @param lst data list to add in head
 */
void list_simple_add_head(void* head, void* lst);

/** add to tail
 * @param head address of pointer to data head list
 * @param lst data list to add to tail
 */
void list_simple_add_tail(void* head, void* lst);

/** add before element
 * @param head address of pointer to head list
 * @param before add before this element
 * @param lst list to add
 */
void list_simple_add_before(void* head, void* before, void* lst);

/** add after element
 * @param head address of pointer to head list
 * @param after add after this element
 * @param lst list to add
 */
void list_simple_add_after(void* head, void* after, void* lst);

/** extract element from list
 * @param head address of pointer to head list
 * @param lst list to extract
 * @return an extracted list or null if not find
 */
void* list_simple_extract(void* head, void* lst);

/** extract element from list when listcmp return 0
 * @param head address of pointer to head list
 * @param userdata data to compare
 * @param fn callback function for compare data, fn(head, userdata)
 * @return an extracted list or null if not find
 */
void* list_simple_find_extract(void* head, void* userdata, listCmp_f fn);

/** free list
 * @param lst list to be free, call callback clean if needed
 */
void list_simple_free(void* lst);

/** cleanup */
void list_simple_free_auto(void* lst);

#define __listsimple_free __cleanup(list_simple_free_auto)

/** free all list
 * @param lst is list head, call list_simple_free for each element
 * @see list_simple_free
 */
void list_simple_all_free(void* lst);

/** cleanup */
void list_simple_all_free_auto(void* head);

/** cleanup*/
#define __listsimple_allfree __cleanup(list_simple_all_free_auto)

/** foreach element in list
 * @param HEAD list
 * @param IT variable to set a data
 * @code
 * int* head = ....;
 * int* element;
 * listsimple_foreach(head, element){
 *	if( *element == ... ) ...;
 * }
 * @endcode
 */
#define listsimple_foreach(HEAD, IT) for(IT = HEAD; IT; IT = (LIST_SIMPLE(IT)->next))

/** foreach element in list, using void**
 * @param HEAD pointer to head or element list
 * @param IT void** iterator name
 * @code
 * int* head = list_simple_new(...);
 * *head = 1234;
 * listsimple_generic_foreach(head, lst){
 *  if( *(int*)(lst) == 1234 ) break;
 * }
 * @endcode
 */
#define listsimple_generic_foreach(HEAD, IT) for(void** IT = (void**)HEAD; *IT; IT = &(LIST_SIMPLE(*IT)->next))

/***************************************/
/*** double linked list concatenated ***/
/***************************************/

typedef struct listDouble{
	void* next;       /**< next element*/
	void* prev;       /**< prev element*/
	listFree_f clean; /**< cleanup function*/
	size_t size;      /**< size data*/
	void* data[0];    /**< user data*/
}listDouble_s;

#define LIST_DOUBLY(L) ((listDouble_s*)(ADDR(L)-sizeof(listDouble_s)))

/** create new list auto refereced
 * @param size size of data
 * @param data if userdata != NULL is copied to new list element
 * @param clean cleanup function, can pass NULL
 * @return new list or NULL for error
 */
void* list_doubly_new_raw(size_t size, void* data, listFree_f clean);

/** list doubly new with auto sizeof */
#define list_doubly_new(TYPE, DATA, CLEAN) (TYPE*)list_doubly_new_raw(sizeof(TYPE), DATA, CLEAN)

/** add before element
 * @param before add before this element
 * @param lst list to add
 */
void list_doubly_add_before(void* before, void* lst);

/** add after element
 * @param after add after this element
 * @param lst list to add
 */
void list_doubly_add_after(void* after, void* lst);

/** merge list b after list a
 * @param a add list after this element
 * @param b list to merge
 */
void list_doubly_merge(void* a, void* b);

/** extract element from list
 * @param lst list to extract
 * @return same lst pass, but setted next and prev to lst
 */
void* list_doubly_extract(void* lst);

/** free list
 * @param lst list to be free, call callback clean if needed
 */
void list_doubly_free(void* lst);

/** free all list
 * @param lst is list head, call list_simple_free for each element
 * @see list_simple_free
 */
void list_doubly_all_free(void* lst);

/** cleanup */
void list_doubly_all_free_auto(void* lst);

/** cleanup */
#define __listdoubly_allfree __cleanup(list_doubly_all_free_auto)

/** return 1 if have only root in list **/
int list_doubly_only_root(void* lst);

/** do while element in list, for traversing all list
 * @param HEAD head or element list
 * @param IT iterator name
 * @code
 * int* head = list_doubly_new(...);
 * ...
 * int* iterator;
 * listdoubly_do(head, iterator){
 *  if( *(iterator) == 1234 ) break;
 * }listdoubly_while(head, iterator);
 * @endcode
 */
#define listdoubly_do(HEAD, IT) do{ void* ___ ## IT ## ___ = HEAD; IT = HEAD; do
#define listdoubly_while(HEAD,IT) while( (IT=LIST_DOUBLY(IT)->next) != ___ ## IT ## ___ );}while(0)

#endif
