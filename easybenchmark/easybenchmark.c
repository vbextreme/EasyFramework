#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "easybenchmark.h"

typedef struct _BCHINFO
{
	CHAR fname[512];
	FLOAT64 t;
	BYTE flags;
	struct _BCHINFO* prev;
	struct _BCHINFO* next;
}BCHINFO;

typedef struct __BCHPERF
{
	BCHINFO* first;
}_BCHPERF;


FLOAT64 bch_get()
{
    struct timeval t;
    struct timezone tzp;
    gettimeofday(&t, &tzp);
    return t.tv_sec + t.tv_usec*1e-6;
}

FLOAT64 bch_clc(FLOAT64 st, FLOAT64 en)
{
    return en-st;
}

BCHPERF bch_perf_init()
{
	BCHPERF p = malloc(sizeof(_BCHPERF));
	p->first = NULL; 
	return p;
}

VOID bch_perf_add(BCHPERF p,const CHAR* fname, BYTE flags)
{
	FLOAT64 t = bch_get();
	BCHINFO* i = malloc(sizeof(BCHINFO));
	i->t = t;
	i->flags = flags;
	strcpy(i->fname,fname);
	i->prev = NULL;
	i->next = p->first;
	if ( p->first ) p->first->prev = i;
	p->first = i;
}

BOOL bch_perf(FILE* out, BCHPERF p)
{
	for (; p->first->next; p->first = p->first->next);
	
	fprintf(out,"[FNC] [COUNT] [TOTAL TIME] [TIME AVERAGE]\n");
	
	while ( p->first )
	{
		BCHINFO* i = p->first;
		if ( !(i->flags & BCH_FLAG_START) ) return FALSE;
		if ( i->prev ) {i->prev->next = NULL;}
		p->first = i->prev;
		
		CHAR cname[512];
			strcpy(cname,i->fname);
		
		BOOL onp = FALSE;
		FLOAT64 ts = i->t;
		FLOAT64 tr = 0.0;
		FLOAT64 tp = 0.0;
		FLOAT64 tt = 0.0;
		UINT32 cc = 0;
		
		free(i);
		i = p->first;
		while ( i )
		{
			if ( strcmp(cname,i->fname) ) { i = i->prev; continue;}
			
			if ( i->flags & BCH_FLAG_START )
			{
				if ( onp )
				{
					tr = i->t - tp;
				}
				else
				{
					tr = 0.0;
					ts = i->t;
					onp = FALSE;
				}
			}
			else if ( i->flags & BCH_FLAG_PAUSE )
			{
				tp = i->t;
				onp = TRUE;
			}
			else if ( i->flags & BCH_FLAG_STOP )
			{
				++cc;
				tt += (i->t - ts ) - tr;
			}
			
			if ( i->prev ) { i->prev->next = i->next; }
			if ( i->next ) { i->next->prev = i->prev; }
			if ( i == p->first ) p->first = p->first->prev;
			BCHINFO* r = i;
			i = i->prev;
			free(r);
		}
		
		fprintf(out,"[%s] [%d] [%f] [%f]\n",cname,cc,tt,tt/(double)cc);
	}
	
	return TRUE;
}

