#ifndef EASYLIST_H_INCLUDED
#define EASYLIST_H_INCLUDED

#include <easytype.h>

///LIST TREE HASHLIST 
//(QUEUE<fifo/lifo><priority><increasepriority>)

#define ELEMENTCASTDATA(ELE,TYPE) ((TYPE)ELE->data)
#define NODECASTDATA(NOD,TYPE) ((TYPE)NOD->data)

typedef VOID(*CBKLSTFREE)(INT32 type, VOID* S_T);
typedef BOOL(*CBKLSTFIND)(INT32 typea, VOID* S_T, INT32 typeb, VOID* val);
typedef INT32(*CBKLSTORDER)(INT32 typea, VOID* AS_T,INT32 typeb, VOID* BS_T);

typedef enum{LFIRST, LLAST,LPREV,LNEXT} LSTMODE;
typedef enum{TROOT, TBEFORE, TAFTER,TRIGHT, TLEFT} TREEMODE;
typedef enum{QFIFO, QFIFOPRI, QLIFO, QLIFOPRI} QUEUEMODE;

typedef struct _ELEMENT
{
	INT32 type;
	VOID* data;
	BOOL autofree;
	CBKLSTFREE fncf;
	struct _ELEMENT* next;
	struct _ELEMENT* prev;
}ELEMENT;

typedef struct _LIST
{
	ELEMENT* first;
	ELEMENT* last;
	UINT32 count;
}LIST;

typedef struct _NODE
{
	INT32 type;
	VOID* data;
	BOOL autofree;
	CBKLSTFREE fncf;
	struct _NODE* inode;
	struct _NODE* right;
	struct _NODE* left;
}NODE;

typedef struct _TREE
{
	NODE* root;
	UINT32 count;
}TREE;

typedef struct _HASHL
{
	LIST* tbl;
	UINT32 szt;
	UINT32 count;
}HASHL;

typedef struct _PELEMENT
{
	INT32 pri;
	INT32 type;
	VOID* data;
	BOOL autofree;
	CBKLSTFREE fncf;
	struct _PELEMENT* next;
	struct _PELEMENT* prev;
}PELEMENT;

typedef struct _QUEUEL
{
	PELEMENT* first;
	PELEMENT* last;
	QUEUEMODE mode;
	UINT32 count;
}QUEUEL;

#define foreach_lst(NOD,LST) for( NOD = LST->first; NOD; NOD = NOD->next )

///element
ELEMENT* element_new(INT32 type, VOID* st, BOOL autofree, CBKLSTFREE fncf);
VOID element_free(ELEMENT* e);
///list
VOID lst_init(LIST* l);
LIST* lst_new();
VOID lst_clear(LIST* l);
VOID lst_free(LIST* l);
VOID lst_add(LIST* l, ELEMENT* e, LSTMODE m, ELEMENT* from);
ELEMENT* lst_pull(LIST* l, ELEMENT* e);
VOID lst_remove(LIST* l,ELEMENT* e);
VOID lst_swap(LIST* l, ELEMENT* a, ELEMENT* b);
ELEMENT* lst_find(ELEMENT* from, INT32 type, VOID* data, CBKLSTFIND fncf);
VOID lst_sort(LIST* l, BOOL ascen, CBKLSTORDER fnco);
UINT32 lst_recount(ELEMENT* e);
ELEMENT* lst_cut(LIST* l, ELEMENT* e);
VOID lst_paste(LIST* l, ELEMENT* e, LSTMODE m, ELEMENT* from);
VOID element_dbg_print(ELEMENT* n);
VOID lst_debug(LIST* l);
///node
NODE* node_new(INT32 type, VOID* st, BOOL autofree, CBKLSTFREE fncf);
VOID node_free(NODE* n);
///tree
VOID ltr_init(TREE* t);
TREE* ltr_new();
VOID ltr_clear(TREE* t);
VOID ltr_free(TREE* t);
VOID ltr_add(TREE* t, NODE* n, TREEMODE m, TREEMODE leftright, NODE* from);
UINT32 ltr_recount(NODE* n);
NODE* ltr_cut(TREE* t, NODE* n);
VOID ltr_swap(TREE* t, NODE* a, NODE* b);
NODE* ltr_find(NODE* n, INT32 type, VOID* data, CBKLSTFIND fncf);
///hashl
VOID lhs_init(HASHL* h, UINT32 tblsz);
HASHL* lhs_new(UINT32 tblsz);
VOID lhs_clear(HASHL* h);
VOID lhs_free(HASHL* h);
VOID lhs_add(HASHL* h, UINT32 hash, ELEMENT* e);
ELEMENT* lhs_pull(HASHL* l, UINT32 hash, ELEMENT* e);
VOID lhs_remove(HASHL* h, UINT32 hash, ELEMENT* e);
ELEMENT* lhs_find(HASHL* h, UINT32 hash, ELEMENT* e, INT32 type, VOID* data, CBKLSTFIND fncf);
///pelement
PELEMENT* pelement_new(INT32 pri, INT32 type, VOID* st, BOOL autofree, CBKLSTFREE fncf);
VOID pelement_free(PELEMENT* e);
///queuel
VOID lqu_init(QUEUEL* q, QUEUEMODE m);
QUEUEL* lqu_new(QUEUEMODE m);
VOID lqu_clear(QUEUEL* q);
VOID lqu_free(QUEUEL* q);
VOID lqu_push(QUEUEL* q, UINT32 pri, PELEMENT* e);
PELEMENT* lqu_pull(QUEUEL* q);

#endif // EASYLIST_H_INCLUDED
