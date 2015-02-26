#include "easystring.h"

#include <stdlib.h>
#include <stdio.h>
#include <easylist.h>

typedef struct __LINE
{
	CHAR* buf;
	UINT32 sz;
	UINT32 len;
	struct __LINE* prev;
	struct __LINE* next;
}_LINE;

typedef struct __CARRET
{
	_LINE* s;
	_LINE* l;
	CHAR* c;
	UINT32 k;
	BOOL ins;
	UINT32 sel;
}_CARRET;

typedef struct __EDIT
{
	_LINE* head;
	_LINE* tail;
	UINT32 count;
	UINT32 rebuf;
	_CARRET c;
}_EDIT;

_LINE* _line_new(UINT32 szb)
{
	_LINE* l = malloc(sizeof(_LINE));
	l->len = 0;
	l->sz = szb;
	l->buf = malloc(sizeof(CHAR) * szb);
	ele_init(l);
	return l;
}

VOID _line_add(_EDIT* e, _LINE* n, INT32 m, _LINE* c)
{
	lst_add(e,n,m,c);
	++e->count;
}

EDIT edt_new(UINT32 szb)
{
	_EDIT* e = malloc(sizeof(_EDIT));
	e->rebuf = szb;
	e->count = 0;
	
	lst_init(e);
	_LINE* l = _line_new(szb);
	_line_add(e,l,LFIRST,NULL);
	
	e->c.s = e->c.l = l;
	e->c.c = l->buf;
	e->c.k = 0;
	e->c.sel = 1;
	e->c.ins = TRUE;
	
	return e;
}






CHAR* str_skipspace(CHAR* s)
{
	while ( *s && (*s == ' ' || *s == '\t') ) ++s;
	return s;
}

CHAR* str_skipline(CHAR* s)
{
	while ( *s && *s != '\n' ) ++s;
	if ( *s ) ++s;
	return s;
}


CHAR* str_skipc(CHAR* s, CHAR skip)
{
	while ( *s && *s == skip ) ++s;
	return s;
}

CHAR* str_skips(CHAR* s, CHAR* skip)
{
	CHAR* dd;
	while( *s )
	{
		for ( dd = skip; *dd; ++dd)
			if ( *s == *dd ) {++s;continue;}
		break; 
	}
	return s;
}

CHAR* str_movetoc(CHAR* s, CHAR delimiter)
{
	while ( *s && *s != delimiter ) ++s;
	return s;
}

CHAR* str_movetos(CHAR* s, CHAR* delimiter)
{
	CHAR* dd;
	for (; *s; ++s )
	{
		for ( dd = delimiter; *dd; ++dd)
			if ( *s == *dd ) {return s;} 
	}
	return s; 
}

CHAR* str_copytoc(CHAR* d, CHAR* s, CHAR delimiter)
{
	while ( *s && *s != delimiter ) *d++ = *s++;
	*d = '\0';
	return s;
}

CHAR* str_copytos(CHAR* d, CHAR* s, CHAR* delimiter)
{
	CHAR* dd;
	while ( *s )
	{
		for ( dd = delimiter; *dd; ++dd)
			if ( *s == *dd ) { *d = '\0'; return s;} 
		*d++ = *s++;
	}
	*d = '\0';
	return s; 
}

CHAR* str_firstvalidchar(CHAR* s)
{
	while ( *s && ( *s == ' ' || *s == '\t' || *s == '\n' ) ) ++s;
	return s; 
}

CHAR* str_toend(CHAR* s)
{
	while ( *s ) ++s;
	return s;
}

CHAR* str_insc(CHAR* d, CHAR c)
{
	UINT32 l = strlen(d) + 1;
	memmove(d+1,d,l);
	*d = c;
	return d + 1;
}

CHAR* str_inss(CHAR* d, CHAR* s)
{
	UINT32 ls = strlen(s);
	UINT32 ld = strlen(d) + 1;
	memmove(d+ls,d,ld);
	memcpy(d,s,ls);
	return d + ls;
}

CHAR* str_del(CHAR* d, UINT32 n)
{
	UINT32 l = str_len(d);
	if ( n >= l ) { *d = '\0'; return d;}
	memmove(d,d+n, (l - n)+1);
	return d;
}

BOOL str_empty(CHAR* s)
{
	return ( !s || !*s ) ? TRUE : FALSE;
}

int str_isvoc(register int c)
{
   c |= 0x20;
   return !((c ^ 0x61) && (c ^ 0x65) && (c ^ 0x69) && (c ^ 0x6F) && (c ^ 0x75));
}

INT32 rex_exec(CHAR** stout, CHAR** enout, CHAR** s, REGEX* r)
{
	regmatch_t rm;
	INT32 ret = regexec(r,*s,1,&rm,0);
	if ( 0 == ret )
	{
		*stout = rm.rm_so + *s;
		*enout = rm.rm_eo + *s;
		*s = *s + rm.rm_eo + 1;
		return 0;
	}
	return ret;
}
	
VOID rex_perror(INT32 err, REGEX* r)
{
	CHAR msg[0x1000];
	regerror(err,r,msg,0x1000);
	printf("rex %s\n",msg);
}



















