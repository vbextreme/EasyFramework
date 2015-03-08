#include "easylist.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/// /////// ///
/// ELEMENT ///
/// /////// ///

ELEMENT* element_new(INT32 type, VOID* st, BOOL autofree, CBKLSTFREE fncf)
{
    ELEMENT* e = malloc(sizeof(ELEMENT));
    e->type = type;
    e->data = st;
    e->autofree = autofree;
    e->fncf = fncf;
    e->next = NULL;
    e->prev = NULL;
    return e;
}

VOID element_free(ELEMENT* e)
{
    if ( !e ) return;

    if ( e->data )
    {
        if ( e->autofree )
            free(e->data);
        else if ( e->fncf )
            e->fncf(e->type,e->data);
    }
    free(e);
}

/// //// ///
/// LIST ///
/// //// ///

VOID lst_init(LIST* l)
{
    l->first = NULL;
    l->last = NULL;
    l->count = 0;
}

LIST* lst_new()
{
	LIST* l = malloc(sizeof(LIST));
	lst_init(l);
	return l;
}

VOID lst_clear(LIST* l)
{
    ELEMENT* a = l->first;
    ELEMENT* b;
    while ( a )
    {
        b = a->next;
        element_free(a);
        a = b;
    }
    l->first = NULL;
    l->last = NULL;
    l->count = 0;
}

VOID lst_free(LIST* l)
{
    lst_clear(l);
    free(l);
}

VOID lst_add(LIST* l, ELEMENT* e, LSTMODE m, ELEMENT* from)
{
    if ( !l->first )
    {
        l->first = e;
        l->last = e;
        l->count = 1;
        return;
    }

    switch( m )
    {
        case LFIRST:
            e->next = l->first;
            l->first->prev = e;
            l->first = e;
        break;

        case LLAST:
            e->prev = l->last;
            l->last->next = e;
            l->last = e;
        break;

        case LPREV:
            if ( !from ) return;
            e->prev = from->prev;
            e->next = from;
            from->prev = e;
            if ( e->prev )
                e->prev->next = e;
            else
                l->first = e;
        break;

        case LNEXT:
            if ( !from ) return;
            e->prev = from;
            e->next = from->next;
            from->next = e;
            if ( e->next )
                e->next->prev = e;
            else
                l->last = e;
        break;
        
        default: return;
    }
    
    ++l->count;
}

ELEMENT* lst_pull(LIST* l, ELEMENT* e)
{
    if ( !e ) return NULL;

    if ( e->prev )
        e->prev->next = e->next;
    else
        l->first = e->next;

    if ( e->next )
        e->next->prev = e->prev;
    else
        l->last = e->prev;

    e->prev = NULL;
    e->next = NULL;
	
	--l->count;
    return e;
}


VOID lst_remove(LIST* l,ELEMENT* e)
{
    e = lst_pull(l,e);
    element_free(e);
}

VOID lst_swap(LIST* l, ELEMENT* a, ELEMENT* b)
{
    if ( !a || !b ) return;

    if ( a->prev )
        a->prev->next = b;
    else
        l->first = b;

    if ( a->next )
        a->next->prev = b;
    else
        l->last = b;

    if ( b->prev )
        b->prev->next = a;
    else
        l->first = a;

    if ( b->next )
        b->next->prev = a;
    else
        l->last = a;

    ELEMENT* tp = a->prev;
    ELEMENT* tn = a->next;

    a->prev = b->prev;
    a->next = b->next;
    b->prev = tp;
    b->next = tn;
}

ELEMENT* lst_find(ELEMENT* from, INT32 type, VOID* data, CBKLSTFIND fncf)
{
	if ( !fncf ) return NULL;
	
	for(; from; from = from->next)
		if ( fncf(from->type, from->data, type, data) ) return from;
	return NULL;
}

VOID lst_sort(LIST* l, BOOL ascen, CBKLSTORDER fnco)
{
	if ( !fnco ) return;
	
    ELEMENT* n;
    BOOL con;
    INT32 ret;

    do
    {
        con = FALSE;
        foreach_lst(n,l)
        {
			if ( !n->next ) continue;
			
			ret = fnco(n->type, n->data, n->type, n->next->data );
			
            if ( ret < 0 && !ascen)
            {
                lst_swap( l, n, n->next);
                con = TRUE;
            }
            else if ( ret > 0 && ascen)
            {
                lst_swap( l, n, n->next);
                con = TRUE;
            }
        }
    }while( con );
}

UINT32 lst_recount(ELEMENT* e)
{
	UINT32 ret;
	for(ret = 0; e; e = e->next, ++ret);
	return ret;
} 

ELEMENT* lst_cut(LIST* l, ELEMENT* e)
{
    if ( !e ) return NULL;
	
	if ( e->prev )
	{
		e->prev->next = NULL;
		l->last = e->prev;
		l->count = lst_recount(l->first);
	}
	else
	{
		l->first = NULL;
		l->last = NULL;
		l->count = 0;
	}
	
	e->prev = NULL;
	
    return e;
}

VOID lst_paste(LIST* l, ELEMENT* e, LSTMODE m, ELEMENT* from)
{
	ELEMENT* lst;
	for (lst = e; lst->next; lst = lst->next);
	
    if ( !l->first )
    {
        l->first = e;
        l->last = lst;
        l->count = lst_recount(l->first);
        return;
    }
	
    switch( m )
    {
        case LFIRST:
            e->next = l->first;
            l->first->prev = e;
            l->first = e;
        break;

        case LLAST:
            e->prev = l->last;
            l->last->next = e;
            l->last = e;
        break;

        case LPREV:
            if ( !from ) return;
            e->prev = from->prev;
            e->next = from;
            from->prev = e;
            if ( e->prev )
                e->prev->next = e;
            else
                l->first = e;
        break;

        case LNEXT:
            if ( !from ) return;
            e->prev = from;
            e->next = from->next;
            from->next = e;
            if ( e->next )
                e->next->prev = e;
            else
                l->last = e;
        break;
        
        default: return;
    }
    
   l->count = lst_recount(l->first);
}

VOID element_dbg_print(ELEMENT* n)
{
	printf("[%7p<-%7p(T:%d)->%7p]\n",n->prev,n,n->type,n->next);
}

VOID lst_debug(LIST* l)
{
    ELEMENT* n;
    int c;
    for ( c = 0, n = l->first; n; n = n->next, ++c )
    {
        printf("(%4d)",c);
        element_dbg_print(n);
    }
}

/// //// ///
/// NODE ///
/// //// ///

NODE* node_new(INT32 type, VOID* st, BOOL autofree, CBKLSTFREE fncf)
{
    NODE* n = malloc(sizeof(NODE));
    n->type = type;
    n->data = st;
    n->autofree = autofree;
    n->fncf = fncf;
    n->left = NULL;
    n->right = NULL;
    n->inode = NULL;
    return n;
}

VOID node_free(NODE* n)
{
    if ( !n ) return;

    if ( n->data )
    {
        if ( n->autofree )
            free(n->data);
        else if ( n->fncf )
            n->fncf(n->type,n->data);
    }
    free(n);
}

/// //// ///
/// TREE ///
/// //// ///

VOID ltr_init(TREE* t)
{
    t->root = NULL;
    t->count = 0;
}

TREE* ltr_new()
{
	TREE* t = malloc(sizeof(TREE));
	ltr_init(t);
	return t;
}

VOID _ltrrecfree(NODE* n)
{
	if ( n->left ) _ltrrecfree(n->left);
	if ( n->right ) _ltrrecfree(n->right);
	
	node_free(n);
}

VOID ltr_clear(TREE* t)
{
	if ( t->root ) _ltrrecfree(t->root);
	t->root = NULL;
	t->count = 0;
}

VOID ltr_free(TREE* t)
{
    ltr_clear(t);
    free(t);
}

VOID ltr_add(TREE* t, NODE* n, TREEMODE m, TREEMODE leftright, NODE* from)
{
    if ( !t->root )
    {
        t->root = n;
        t->count = 1;
        return;
    }

    switch( m )
    {
        case TROOT:
			switch ( leftright )
			{
				case TROOT: case TBEFORE: case TLEFT: n->left = t->root; break;
				case TAFTER: case TRIGHT: n->right = t->root; break;
				default: return;
			}
			t->root->inode = n;
			t->root = n;
		break;

        case TAFTER:
			if ( !from ) return;
            switch ( leftright )
			{
				case TROOT: case TBEFORE: case TLEFT:	
					if (from->left) from->left->inode = n;
					n->left = from->left; 
					from->left = n;
				break;
				case TAFTER: case TRIGHT: 
					if (from->right) from->right->inode = n;
					n->right = from->right; 
					from->right = n;
				break;
				default: return;
			}
			n->inode = from;
        break;
		
		case TBEFORE:
			if ( !from ) return;
			
			n->inode = from->inode;
			if ( n->inode )
			{
				if ( n->inode->left == from )
					n->inode->left = n;
				else
					n->inode->right = n;
			}
			
            switch ( leftright )
			{
				default: case TLEFT: n->left = from; break;
				case TRIGHT: n->right = from; break;
			}
			from->inode = n;
        break;
        
        default: return;
    }
    ++t->count;
}

UINT32 ltr_recount(NODE* n)
{
	INT32 ret = 0;
	if ( n->left ) ret += ltr_recount(n->left);
	if ( n->right ) ret += ltr_recount(n->right);
	return 1;
}

NODE* ltr_cut(TREE* t, NODE* n)
{
    if ( !n ) return NULL;

    if ( n->inode)
    {
        if ( n->inode->left == n )
        {
            n->inode->left = NULL;
        }
        else
        {
            n->inode->right = NULL;
        }
        n->inode = NULL;
        t->count = ltr_recount(t->root);
    }
    else
    {
        t->root = NULL;
        t->count = 0;
    }
	
    return n;
}

VOID ltr_remove(TREE* t,NODE* n)
{
    n = ltr_cut(t,n);
	if ( n ) _ltrrecfree(n);
}

VOID ltr_swap(TREE* t, NODE* a, NODE* b)
{
    if ( !a || !b ) return;

    if ( a->inode)
    {
        if ( a->inode->left == a )
            a->inode->left = b;
        else
            a->inode->right = b;
    }
    else
    {
        t->root = b;
    }

    if ( b->inode)
    {
        if ( b->inode->left == b )
            b->inode->left = a;
        else
            b->inode->right = a;
    }
    else
    {
        t->root = a;
    }

    NODE* tmp = a->inode;
    a->inode = b->inode;
    b->inode = tmp;
}

NODE* ltr_find(NODE* n, INT32 type, VOID* data, CBKLSTFIND fncf)
{
	NODE* r;
	if ( n->left ) r = ltr_find(n->left,type,data,fncf);
	if ( r ) return r;
	if ( n->right ) r = ltr_find(n->right,type,data,fncf);
	if ( r ) return r;
	return ( fncf(n->type,n->data,type,data) ) ? n : NULL;
}

/// ///// ///
/// HASHL ///
/// ///// ///

VOID lhs_init(HASHL* h, UINT32 tblsz)
{
	h->tbl = malloc(sizeof(LIST) * tblsz);
	h->szt = tblsz;
	
	INT32 i;
	for (i = 0; i < tblsz; ++i)
		lst_init(&h->tbl[i]);
    h->count = 0;
}

HASHL* lhs_new(UINT32 tblsz)
{
	HASHL* h = malloc(sizeof(HASHL));
	lhs_init(h,tblsz);
	return h;
}

VOID lhs_clear(HASHL* h)
{
	INT32 i;
	for (i = 0; i < h->szt; ++i)
		lst_clear(&h->tbl[i]);
	
    h->count = 0;
}

VOID lhs_free(HASHL* h)
{
    lhs_clear(h);
    free(h);
}

VOID lhs_add(HASHL* h, UINT32 hash, ELEMENT* e)
{
	if ( hash >= h->szt ) return;
	lst_add(&h->tbl[hash],e,LFIRST,NULL);
    ++h->count;
}

ELEMENT* lhs_pull(HASHL* h, UINT32 hash, ELEMENT* e)
{
	if ( hash >= h->szt ) return NULL;
	e = lst_pull(&h->tbl[hash],e);
    if ( !e ) return NULL;
	--h->count;
    return e;
}

VOID lhs_remove(HASHL* h, UINT32 hash, ELEMENT* e)
{
    e = lhs_pull(h,hash,e);
    if ( e )
		element_free(e);
}

ELEMENT* lhs_find(HASHL* h, UINT32 hash, INT32 type, VOID* data, CBKLSTFIND fncf)
{
	if ( !fncf ) return NULL;
	ELEMENT* e = lst_find(h->tbl[hash].first,type,data,fncf);
	return e;
}

/// //////// ///
/// PELEMENT ///
/// //////// ///

PELEMENT* pelement_new(INT32 pri, INT32 type, VOID* st, BOOL autofree, CBKLSTFREE fncf)
{
    PELEMENT* e = malloc(sizeof(PELEMENT));
    e->pri = pri;
    e->type = type;
    e->data = st;
    e->autofree = autofree;
    e->fncf = fncf;
    e->next = NULL;
    e->prev = NULL;
    return e;
}

VOID pelement_free(PELEMENT* e)
{
    if ( !e ) return;
    if ( e->data )
    {
        if ( e->autofree )
            free(e->data);
        else if ( e->fncf )
            e->fncf(e->type,e->data);
    }
    free(e);
}

/// ////// ///
/// QUEUEL ///
/// ////// ///

VOID lqu_init(QUEUEL* q, QUEUEMODE m)
{
	q->first = NULL;
	q->last = NULL;
	q->count = 0;
    q->mode = m;
}

QUEUEL* lqu_new(QUEUEMODE m)
{
	QUEUEL* q = malloc(sizeof(QUEUEL));
	lqu_init(q,m);
	return q;
}

VOID lqu_clear(QUEUEL* q)
{
	PELEMENT* a = q->first;
    PELEMENT* b;
    while ( a )
    {
        b = a->next;
        pelement_free(a);
        a = b;
    }
    q->first = NULL;
    q->last = NULL;
    q->count = 0;
}

VOID lqu_free(QUEUEL* q)
{
    lqu_clear(q);
    free(q);
}

VOID lqu_push(QUEUEL* q, UINT32 pri, PELEMENT* e)
{
	if ( !q->first )
	{
		q->first = q->last = e;
		q->count = 1;
		return;
	}
	
	switch ( q->mode )
	{
		default: case QFIFO: case QLIFO:
			e->next = q->first;
			q->first->prev = e;
			q->first = e;
		break;
		
		case QFIFOPRI: case QLIFOPRI:
			;
			PELEMENT* f = q->first;
			for(; f && pri > f->pri; f = f->next);
			
			if ( !f )
			{
				q->last->next = e;
				e->prev = q->last;
				q->last = e;
				break;
			}
			
			if ( f->prev )
			{
				f->prev->next = e;
				e->prev = f->prev;
				f->prev = e;
				e->next = f;
			}
			else
			{ 
				e->next = q->first;
				q->first->prev = e;
				q->first = e;
			}
			
		break;
	}
	++q->count;
}

PELEMENT* lqu_pull(QUEUEL* q)
{
	PELEMENT* ret;
	
	if ( q->first == q->last )
	{
		ret = q->first;
		q->first = NULL;
		q->last = NULL;
		q->count = 0;
		return ret;
	}
	
	switch ( q->mode )
	{
		default: case QFIFO: case QFIFOPRI:
			ret = q->last;
			q->last->prev->next = NULL;
			q->last = q->last->prev;
		break;
		
		case QLIFO: case QLIFOPRI:
			ret = q->first;
			q->first->next->prev = NULL;
			q->first = q->first->next;
		break;
	}
	
    ret->prev = NULL;
    ret->next = NULL;
	--q->count;
    return ret;
}

BOOL lqu_empty(QUEUEL* q)
{
	return !q->first;
}
