#include "easylist.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*


/// /////// ///
/// ELEMENT ///
/// /////// ///

ELE* ele_new(VOID* st)
{
    ELE* e = malloc(sizeof(ELE));
    e->s = st;
    e->next = NULL;
    e->prev = NULL;
    return e;
}

VOID ele_free(LST* l, ELE* e)
{
    if ( !e ) return;

    if ( e->s )
    {
        if ( l->autofree )
            free(e->s);
        else if ( l->clbkf != NULL)
            l->clbkf(e->s);
    }

    free(e);
}

/// //// ///
/// NODE ///
/// //// ///

NOD* nod_new(VOID* st)
{
    NOD* n = malloc(sizeof(NOD));
    n->s = st;
    lst_init(&n->c,FALSE,NULL);

    return n;
}

VOID nod_free(LGR* g, NOD* n)
{
    if ( !n ) return;

    lst_free(&n->c);

    if ( n->s )
    {
        if ( g->autofree )
            free(n->s);
        else if ( g->clbkf != NULL)
            g->clbkf(n->s);
    }

    free(n);
}

VOID nod_con_add(NOD* n, NOD* add)
{
    lst_add(&n->c,ele_new(add),ILAST);
}

INT32 _nodfi(NOD* val, NOD* n)
{
    return (val == n) ? 1 : 0;
}

VOID nod_con_remove(NOD* n,NOD* rem)
{
    lst_find(&n->c,rem,(ELEFIND)_nodfi);
    if (n->c.current)
        lst_remove(&n->c,n->c.current);
}

/// ////////////// ///
/// INITIALIZATION ///
/// ////////////// ///

VOID lst_init(LST* l, BOOL autofree, ELEFREE clbkf)
{
    l->first = NULL;
    l->last = NULL;
    l->current = NULL;
    l->autofree = autofree;
    l->clbkf = clbkf;
}

VOID lhs_init(LHS* h, UINT32 maxmap, BOOL autofree, ELEFREE clbkf)
{
    h->maxmap = maxmap;

    h->mp = malloc(sizeof(LST) * maxmap);
    int i;
    for (i = 0; i < maxmap; i++)
    {
        lst_init(&h->mp[i],autofree,clbkf);
    }
}

VOID lgr_init(LGR* g, BOOL autofree, ELEFREE clbkf)
{
    g->autofree = autofree;
    g->clbkf = clbkf;
    lst_init(&g->n,autofree,clbkf);
}

/// //// ///
/// FREE ///
/// //// ///

VOID lst_free(LST* l)
{
    ELE* a = l->first;
    ELE* b;
    while ( a )
    {
        b = a->next;
        ele_free(l,a);
        a = b;
    }
}

VOID ltr_free(LTR* t)
{
    ELE* myr;
    if ( !t->last )
    {
        myr = t->current = t->last = t->first;
    }
    else if ( t->current )
    {
        myr = t->current;
    }
    else
    {
        return;
    }

    t->current = myr->prev;
    ltr_free(t);
    t->current = myr->next;
    ltr_free(t);

    ele_free(t,myr);
}

VOID lhs_free(LHS* h)
{
    int i;
    for (i = 0; i < h->maxmap; i++)
    {
        lst_free(&h->mp[i]);
    }
    free(h->mp);
}

VOID lgr_free(LGR* g)
{
    ELE* a = g->n.first;
    ELE* b;
    NOD* n;
    while ( a )
    {
        b = a->next;
        n = (NOD*) a->s;
        nod_free(g,n);
        free(a);
        a = b;
    }
}

/// /// ///
/// ADD ///
/// /// ///

VOID lst_add(LST* l, ELE* e, IMODE m)
{
    if ( !l->first )
    {
        l->first = e;
        l->current = e;
        l->last = e;
        return;
    }

    switch( m )
    {
        case IFIRST:
            e->next = l->first;
            l->first->prev = e;
            l->first = e;
            l->current = e;
        return;

        case ILAST:
            e->prev = l->last;
            l->last->next = e;
            l->last = e;
        return;

        case IBEFORE:
        case ILEFT:
            if ( !l->current ) return;
            e->prev = l->current->prev;
            e->next = l->current;
            l->current->prev = e;
            if ( e->prev )
                e->prev->next = e;
            else
                l->first = e;
        return;

        case IAFTER:
        case IRIGHT:
            if ( !l->current ) return;
            e->prev = l->current;
            e->next = l->current->next;
            l->current->next = e;
            if ( e->next )
                e->next->prev = e;
            else
                l->last = e;
        return;
    }
}

VOID ltr_add(LTR* t, ELE* e, IMODE m, BOOL flleft)
{
    if ( !t->first )
    {
        t->first = e;
        t->current = e;
        return;
    }

    ELE* la;

    switch( m )
    {
        case IFIRST:
            if ( flleft )
            {
                e->prev = t->first;
            }
            else
            {
                e->next = t->first;
            }
            t->first->inode = e;
            t->first = e;
        return;

        case ILAST:
            if ( !t->current ) return;
            la = t->current;

            if ( flleft )
            {
                for (; la->prev; la = la->prev);
                la->prev = e;
            }
            else
            {
                for (; la->next; la = la->next);
                la->next = e;
            }
            e->inode = la;
        return;

        case IBEFORE:
        case ILEFT:
            if ( !t->current ) return;

            e->inode = t->current;

            if ( t->current->prev )
            {
                e->prev = t->current->prev;
                t->current->prev->inode = e;
            }

            t->current->prev = e;
        return;

        case IAFTER:
        case IRIGHT:
            if ( !t->current ) return;

            e->inode = t->current;

            if ( t->current->next )
            {
                e->next = t->current->next;
                t->current->next->inode = e;
            }

            t->current->next = e;
        return;
    }
}

VOID lhs_add(LHS* h, ELE* e, UINT32 hash)
{
    lst_add( &h->mp[hash], e, ILAST );
}

VOID lgr_add(LGR* g, NOD* n)
{
    lst_add(&g->n,ele_new(n),ILAST);
}

/// ////////// ///
/// ESTRAZIONE ///
/// ////////// ///

ELE* lst_extract(LST* l, ELE* e)
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

    return e;
}

ELE* ltr_extract(LTR* t, ELE* e)
{
    if ( !e ) return NULL;

    if ( e->inode)
    {
        if ( e->inode->prev == e )
        {
            e->inode->prev = NULL;
        }
        else
        {
            e->inode->next = NULL;
        }
        e->inode = NULL;
    }
    else
    {
        t->first = NULL;
    }

    return e;
}

VOID lst_remove(LST* l,ELE* e)
{
    e = lst_extract(l,e);
    ele_free(l,e);
}

VOID ltr_remove(LTR* t, ELE* e)
{
    e = ltr_extract(t,e);

    ELE* fakefirst = t->first;
    ELE* fakecurrent = t->current;

    t->first = e;
    ltr_free(t);

    t->first = fakefirst;
    t->current = fakecurrent;
    t->last = NULL;
}


VOID lst_swap(LST* l, ELE* a, ELE* b)
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

    ELE* tp = a->prev;
    ELE* tn = a->next;

    a->prev = b->prev;
    a->next = b->next;
    b->prev = tp;
    b->next = tn;
}

VOID ltr_swap(LTR* t, ELE* a, ELE* b)
{
    if ( !a || !b ) return;

    if ( a->inode)
    {
        if ( a->inode->prev == a )
        {
            a->inode->prev = b;
        }
        else
        {
            a->inode->next = b;
        }
    }
    else
    {
        t->first = b;
    }

    if ( b->inode)
    {
        if ( b->inode->prev == b )
        {
            b->inode->prev = a;
        }
        else
        {
            b->inode->next = a;
        }
    }
    else
    {
        t->first = b;
    }

    ELE* tmp = a->inode;
    a->inode = b->inode;
    b->inode = tmp;
}

VOID lst_find(LST* l,VOID* data, ELEFIND fncf)
{
    for (; l->current && !fncf( data, l->current->s); l->current = l->current->next );
}

LST* lhs_find(LHS* h, VOID* data, ELEFIND fncf, INT32 hash)
{
    lst_find( &h->mp[hash], data, fncf );
    return &h->mp[hash];
}

VOID lst_sort(LST* l, ELEORDER fnco, BOOL ascen)
{
    ELE* e;
    BOOL con;
    INT32 ret;

    do
    {
        con = FALSE;
        for (e = l->first; e->next; e = e->next )
        {
            ret = fnco( e->s, e->next->s );
            if ( ret < 0 && !ascen)
            {
                lst_swap( l, e, e->next);
                con = TRUE;
            }
            else if ( ret > 0 && ascen)
            {
                lst_swap( l, e, e->next);
                con = TRUE;
            }
        }
    }while( con );
}

VOID lst_reset(LST* l)
{
    l->current = l->first;
}

BOOL lst_next(LST* l)
{
    if ( l->current )
        l->current = l->current->next;

    return ( l->current ) ? TRUE : FALSE;
}

BOOL lst_prev(LST* l)
{
    if ( l->current )
        l->current = l->current->prev;

    return ( l->current ) ? TRUE : FALSE;
}

VOID lst_debug(LST* l)
{
    ELE* e;
    int n;
    for ( n = 0, e = l->current; e ; e = e->next, n++ )
    {
        printf("%4d) [%7p]%7p[%7p]\n",n,e->prev,e,e->next);
    }
}

VOID lhs_debug(LHS* h)
{
    int n;
    for ( n = 0; n < h->maxmap; n++ )
    {
        if ( h->mp[n].first == NULL ) continue;

        printf("\t<<%6d>>\n",n);
        lst_debug(&h->mp[n]);
    }
}

*/
