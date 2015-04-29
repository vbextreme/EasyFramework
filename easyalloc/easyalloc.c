#include "easyalloc.h"
#include <sched.h>

#define MAL_FLAGS_FREE 0x01
#define MAL_FLAGS_USED 0x02
#define MAL_FLAGS_LAST 0x04
#define MAL_FLAGS_LOCK 0x08

#define MAL_SIZE (sizeof(UINT32) + sizeof(UINT32))
#define mal_adrflag(ADR) (UINT32*)(ADR)
#define mal_adrsz(ADR) (UINT32*)(mal_adrflag(ADR)+1)
#define mal_offset(ADR,OF) ((BYTE*)(ADR) + (OF))

VOID alc_freem(VOID **b,UINT32 y)
{
    int i;
    for (i=0; i<y ; i++)
        free(b[i]);

    free(b);
}

VOID** alc_mallocm(UINT32 y, UINT32 x, SIZET st)
{
    VOID **b;

    b=(VOID**)malloc(y * sizeof(VOID*));
		if ( NULL == b )return NULL;

    INT32 i,ii;

    for ( i = 0; i < y; ++i)
    {
        b[i]=(VOID*)malloc(x * st);
        if ( NULL == b[i] )
        {
            for (ii = 0; ii < i; ++ii)
                free(b[ii]);
            free(b);
            return NULL;
        }
    }

    return b;
}

VOID** alc_reallocm(VOID **b,INT32 oldy,INT32 oldx,INT32 newy,INT32 newx,SIZET st)
{
    INT32 i;
    VOID**tmp,*stmp;

    if (newy != ALC_NOT_REALLOC)
    {
        tmp = (VOID**)realloc(b,sizeof(VOID*) * newy);
            if ( NULL == tmp ) return NULL;

        if (newy > oldy)
        {
            for (i=oldy; i < newy ; ++i)
                b[i]=(void*)malloc(oldx*st);
            oldy=newy;
        }
    }

    if(newx != ALC_NOT_REALLOC)
    {
        for (i=0; i < oldy ;i++)
        {
            stmp=realloc(b[i],st*newx);
                if (stmp == NULL) return NULL;
            b[i]=stmp;
        }
    }

    return b;
}

inline SIZET alc_rsizeof(SIZET type)
{
	return  ( type + type % sizeof(SIZET));
} 


inline VOID* mal_init(register VOID* baseadr,register SIZET szmem)
{
	*mal_adrflag(baseadr) = MAL_FLAGS_FREE | MAL_FLAGS_LAST;
	*mal_adrsz(baseadr) = szmem - MAL_SIZE;
	return baseadr;
}

VOID* mal_malloc(register VOID* baseadr,register SIZET reqmem)
{
	reqmem = alc_rsizeof(reqmem);
	
	register BYTE* retadr = NULL;
	register BYTE* adr =(BYTE*) baseadr;
	register BYTE* nxadr;
	BOOL havelock = FALSE;
	
	while ( 1 )
	{	
		
		if ( (*mal_adrflag(adr)) & MAL_FLAGS_USED )
		{
			havelock = FALSE;
			if ( (*mal_adrflag(adr)) & MAL_FLAGS_LAST ) break;
			register SIZET of = *mal_adrsz(adr) + MAL_SIZE;
			adr = mal_offset(adr,of);
			continue;
		}
		
		if ( !havelock && !__sync_bool_compare_and_swap(adr,(*mal_adrflag(adr)) & ~MAL_FLAGS_LOCK,(*mal_adrflag(adr)) | MAL_FLAGS_LOCK ) )
		{
			sched_yield();
			continue;
		}
		
		else if ( (*mal_adrflag(adr)) & MAL_FLAGS_FREE )
		{
			if (  *mal_adrsz(adr) > reqmem + MAL_SIZE)
			{
				retadr = adr + MAL_SIZE;
				nxadr = mal_offset(adr,reqmem + MAL_SIZE);
				if ( (*mal_adrflag(adr)) & MAL_FLAGS_LAST )
					*mal_adrflag(nxadr) = MAL_FLAGS_FREE | MAL_FLAGS_LAST;
				else
					*mal_adrflag(nxadr) = MAL_FLAGS_FREE;
				*mal_adrsz(nxadr) = *mal_adrsz(adr) - (reqmem + MAL_SIZE);
				
				*mal_adrflag(adr) = MAL_FLAGS_USED;
				*mal_adrsz(adr) = reqmem;
				
				__sync_and_and_fetch(adr,~MAL_FLAGS_LOCK);
				return retadr;
			}
			else if (*mal_adrsz(adr) == reqmem )
			{
				retadr = adr + MAL_SIZE;
				*mal_adrflag(adr) &= ~MAL_FLAGS_FREE;
				*mal_adrflag(adr) |= MAL_FLAGS_USED;
				
				__sync_and_and_fetch(adr,~MAL_FLAGS_LOCK);
				return retadr;
			}
			else if ( !((*mal_adrflag(adr)) & MAL_FLAGS_LAST) )
			{
				register SIZET of = *mal_adrsz(adr) + MAL_SIZE;
				register BYTE* nxadr = mal_offset(adr,of);
				while ( !__sync_bool_compare_and_swap(nxadr,(*mal_adrflag(nxadr)) & ~MAL_FLAGS_LOCK,(*mal_adrflag(nxadr)) | MAL_FLAGS_LOCK ) )
				{
					sched_yield();
					continue;
				}
				
				if ( (*mal_adrflag(nxadr)) & MAL_FLAGS_FREE )
				{
					*mal_adrsz(adr) += *mal_adrsz(nxadr) + MAL_SIZE;
					if ( (*mal_adrflag(nxadr)) & MAL_FLAGS_LAST ) *mal_adrflag(adr) |= MAL_FLAGS_LAST;
					__sync_and_and_fetch(nxadr,~MAL_FLAGS_LOCK);
					havelock = TRUE;
					continue;
				}
				
				__sync_and_and_fetch(nxadr,~MAL_FLAGS_LOCK);
			}
		}
		
		if ( (*mal_adrflag(adr)) & MAL_FLAGS_LAST ) break;
		register SIZET of = *mal_adrsz(adr) + MAL_SIZE;
		__sync_and_and_fetch(adr,~MAL_FLAGS_LOCK);
		havelock = FALSE;
		adr = mal_offset(adr,of);
	}
	
	__sync_and_and_fetch(adr,~MAL_FLAGS_LOCK);
	return NULL;
}

VOID mal_free(register VOID* freeadr)
{
	register BYTE* adr = (BYTE*)(freeadr) - MAL_SIZE;
	
	*mal_adrflag(adr) |= MAL_FLAGS_FREE;
	*mal_adrflag(adr) &= ~MAL_FLAGS_USED;
}

/*
VOID* mal_mallocold(register VOID* baseadr,register SIZET reqmem)
{
	reqmem = alc_rsizeof(reqmem);
	
	register BYTE* retadr = NULL;
	register BYTE* adr =(BYTE*) baseadr;
	register BYTE* nxadr;
	
	while ( 1 )
	{
		if ( (*mal_adrflag(adr)) & MAL_FLAGS_FREE )
		{
			if (  *mal_adrsz(adr) > reqmem )
			{
				retadr = adr + MAL_SIZE;
				nxadr = mal_offset(adr,reqmem + MAL_SIZE);
				if ( (*mal_adrflag(adr)) & MAL_FLAGS_LAST )
					*mal_adrflag(nxadr) = MAL_FLAGS_FREE | MAL_FLAGS_LAST;
				else
					*mal_adrflag(nxadr) = MAL_FLAGS_FREE;
				*mal_adrsz(nxadr) = *mal_adrsz(adr) - (reqmem + MAL_SIZE);
				
				*mal_adrflag(adr) = MAL_FLAGS_USED;
				*mal_adrsz(adr) = reqmem;
				return retadr;
			}
			else if (*mal_adrsz(adr) == reqmem)
			{
				retadr = adr + MAL_SIZE;
				*mal_adrflag(adr) &= ~MAL_FLAGS_FREE;
				*mal_adrflag(adr) |= MAL_FLAGS_USED;
				return retadr;
			}
		}
		
		if ( (*mal_adrflag(adr)) & MAL_FLAGS_LAST ) break;
		register SIZET of = *mal_adrsz(adr) + MAL_SIZE;
		adr = mal_offset(adr,of);
	}
	
	return NULL;
}

VOID mal_freeold(register VOID* baseadr,register VOID* freeadr)
{
	register BYTE* adr =(BYTE*) baseadr;
	register BYTE* fadr = (BYTE*)(freeadr) - MAL_SIZE;
	register BYTE* nadr;
	
	///ultimo blocco
	
	if ( !((*mal_adrflag(fadr)) & MAL_FLAGS_LAST) )
	{	
		register SIZET of = *mal_adrsz(fadr) + MAL_SIZE;
		nadr = mal_offset(fadr,of);
		
		if ( (*mal_adrflag(nadr)) & MAL_FLAGS_FREE )
		{
			*mal_adrsz(fadr) += *mal_adrsz(nadr) + MAL_SIZE;
			if ( (*mal_adrflag(nadr)) & MAL_FLAGS_LAST ) *mal_adrflag(fadr) |= MAL_FLAGS_LAST;
		}
	}
	
	///primo blocco
	if ( adr != fadr )
	{
		nadr = adr;
		while( 1 )
		{
			register SIZET of = *mal_adrsz(nadr) + MAL_SIZE;
			nadr = mal_offset(nadr,of);
			if ( nadr == fadr ) break;
			adr = nadr;
		}
		
		if ( (*mal_adrflag(adr)) & MAL_FLAGS_FREE )
		{
			*mal_adrsz(adr) += *mal_adrsz(fadr) + MAL_SIZE;
			if ( (*mal_adrflag(fadr)) & MAL_FLAGS_LAST ) *mal_adrflag(adr) |= MAL_FLAGS_LAST;
		}
	}
	
	*mal_adrflag(fadr) &= ~MAL_FLAGS_USED;
	*mal_adrflag(fadr) |= MAL_FLAGS_FREE;
}
*/

INT32 _printflags(UINT32 flags)
{
	putchar((flags & MAL_FLAGS_LAST) ? 'L' : ' ');
	if (flags & MAL_FLAGS_FREE)
	{
		putchar('F');
	}
	else if (flags & MAL_FLAGS_USED)
	{
		putchar('U');
	}
	else
	{
		putchar('E');
		return -1;
	}
	return 0;
}

VOID mal_dbg_mem(VOID* baseadr)
{
	BYTE* adr =(BYTE*) baseadr;
	
	while(1)
	{
		printf("(%p)[",adr);
		if ( _printflags(*mal_adrflag(adr)) ) break;
		printf("][%6d]\n",*mal_adrsz(adr));
		
		if ( *mal_adrflag(adr) & MAL_FLAGS_LAST ) break;
		SIZET of = *mal_adrsz(adr) + MAL_SIZE;
		adr = mal_offset(adr,of);
	}
}

VOID mal_dbg_adr(VOID* wadr)
{
	BYTE* adr =(BYTE*) wadr;
	
	printf("(%p)[",adr);
	_printflags(*mal_adrflag(adr));
	printf("][%6d]\n",*mal_adrsz(adr));
}



















