#ifndef EASYLIST_H_INCLUDED
#define EASYLIST_H_INCLUDED

#include <easytype.h>

typedef VOID(*ELEFREE)(VOID* s);
typedef INT32(*ELEFIND)(VOID* d,VOID* s);
typedef INT32(*ELEORDER)(VOID* a, VOID* b); // a < b = -1 ; a > b = 1 ; a == b = 0;

typedef enum{IFIRST, ILAST, IAFTER, IBEFORE, ILEFT, IRIGHT}IMODE;

typedef struct _ELE
{
    VOID* s;
    struct _ELE* next;
    struct _ELE* prev;
    struct _ELE* inode;
}ELE;

typedef struct _LST
{
    ELE* first;
    ELE* last;
    ELE* current;
    BOOL autofree;
    ELEFREE clbkf;
}LST;

typedef struct _LST LSK;
typedef struct _LST LTR;


typedef struct _LHS
{
    LST* mp;
    UINT32 maxmap;
}LHS;


typedef enum {DNO,DFW,DBK} DARC;

typedef struct _ARC
{
    INT32 w;
    DARC d;
}ARC;

typedef struct _NOD
{
    void* s;
    LST c;
}NOD;

typedef struct _LGR
{
    LST n;
    BOOL autofree;
    ELEFREE clbkf;
}LGR;

UINT32 fast_hash(char * data, INT32 len);
UINT32 lhs_hash(LHS* h, CHAR* val, INT32 len);

ELE* ele_new(VOID* st);
VOID ele_free(LST* l, ELE* e);

NOD* nod_new(VOID* st);
VOID nod_free(LGR* g, NOD* n);
VOID nod_con_add(NOD* n, NOD* add);
VOID nod_con_remove(NOD* n,NOD* rem);

VOID lst_init(LST* l, BOOL autofree, ELEFREE clbkf);
#define lsk_init( PTRL, AF, CLBKF ) lst_init(PTRL,AF,CLBKF)
#define ltr_init( PTRL, AF, CLBKF ) lst_init(PTRL,AF,CLBKF)
VOID lhs_init(LHS* h, UINT32 maxmap, BOOL autofree, ELEFREE clbkf);
VOID lgr_init(LGR* g, BOOL autofree, ELEFREE clbkf);

VOID lst_free(LST* l);
#define lsk_free( PTRL ) lst_free(PTRL)
VOID ltr_free(LTR* t);
VOID lhs_free(LHS* h);
VOID lgr_free(LGR* g);

VOID lst_add(LST* l, ELE* e, IMODE m);
#define lsk_push( PTRL, E) lst_add(PTRL,E,ILAST)
VOID ltr_add(LTR* t, ELE* e, IMODE m, BOOL flleft);
VOID lhs_add(LHS* h, ELE* e, UINT32 hash);
VOID lgr_add(LGR* g, NOD* n);

ELE* lst_extract(LST* l, ELE* e);
#define lsk_pop( PTRL ) lst_extract(PTRL,(PTRL)->last)
ELE* ltr_extract(LTR* t, ELE* e);

VOID lst_remove(LST* l,ELE* e);
VOID ltr_remove(LTR* t, ELE* e);

VOID lst_swap(LST* l, ELE* a, ELE* b);
VOID ltr_swap(LTR* t, ELE* a, ELE* b);

VOID lst_find(LST* l,VOID* data, ELEFIND fncf);
LST* lhs_find(LHS* h, VOID* data, ELEFIND fncf, INT32 hash);

VOID lst_sort(LST* l, ELEORDER fnco, BOOL ascen);

VOID lst_reset(LST* l);
#define ltr_reset( PTRL ) lst_reset(PTRL)

BOOL lst_next(LST* l);
#define ltr_right( PTRL ) lst_next(PTRL)

BOOL lst_prev(LST* l);
#define ltr_left( PTRL ) lst_prev(PTRL)

VOID lst_debug(LST* l);
#define lsk_debug( PTRL ) lst_debug( PTRL )
VOID lhs_debug(LHS* h);

#endif // EASYLIST_H_INCLUDED
