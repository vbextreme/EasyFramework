#include "easystring.h"

#include <stdlib.h>
#include <stdio.h>

#ifdef _DEBUG
	#include <easybenchmark.h>
	extern BCH_PERF_GLOBAL;
#endif


CHAR* str_skipspace(register CHAR* s)
{
	#ifdef _DEBUG
		BCH_PERF_START;
	#endif
	
	while ( *s && (*s == ' ' || *s == '\t') ) ++s;
	
	#ifdef _DEBUG
		BCH_PERF_STOP;
	#endif
	
	return s;
}

CHAR* str_skipline(register CHAR* s)
{
	#ifdef _DEBUG
		BCH_PERF_START;
	#endif
	
	while ( *s && *s != '\n' ) ++s;
	if ( *s ) ++s;
	
	#ifdef _DEBUG
		BCH_PERF_STOP;
	#endif
	
	return s;
}


CHAR* str_skipc(register CHAR* s, register CHAR skip)
{
	#ifdef _DEBUG
		BCH_PERF_START;
	#endif
	
	while ( *s && *s == skip ) ++s;
	
	#ifdef _DEBUG
		BCH_PERF_STOP;
	#endif
	
	return s;
}

CHAR* str_skips(register CHAR* s, register CHAR* skip)
{
	#ifdef _DEBUG
		BCH_PERF_START;
	#endif
	
	register CHAR* dd;
	while( *s )
	{
		for ( dd = skip; *dd; ++dd)
		{
			if ( *s == *dd ) {goto WCONT;}
		}
		break;
		WCONT: 
		++s;
	}
	
	#ifdef _DEBUG
		BCH_PERF_STOP;
	#endif
	
	return s;
}

CHAR* str_movetoc(register CHAR* s, register CHAR delimiter)
{
	#ifdef _DEBUG
		BCH_PERF_START;
	#endif
	
	while ( *s && *s != delimiter ) ++s;
	
	#ifdef _DEBUG
		BCH_PERF_STOP;
	#endif
	
	return s;
}

CHAR* str_movetos(register CHAR* s, register CHAR* delimiter)
{
	#ifdef _DEBUG
		BCH_PERF_START;
	#endif
	
	CHAR* dd;
	for (; *s; ++s )
	{
		for ( dd = delimiter; *dd; ++dd)
			if ( *s == *dd ) {return s;} 
	}
	
	#ifdef _DEBUG
		BCH_PERF_STOP;
	#endif
	
	return s; 
}

CHAR* str_copytoc(register CHAR* d, register CHAR* s, register CHAR delimiter)
{
	#ifdef _DEBUG
		BCH_PERF_START;
	#endif
	
	while ( *s && *s != delimiter ) *d++ = *s++;
	*d = '\0';
	
	#ifdef _DEBUG
		BCH_PERF_STOP;
	#endif
	
	return s;
}

CHAR* str_copytos(register CHAR* d, register CHAR* s, register CHAR* delimiter)
{
	#ifdef _DEBUG
		BCH_PERF_START;
	#endif
	
	register CHAR* dd;
	while ( *s )
	{
		for ( dd = delimiter; *dd; ++dd)
			if ( *s == *dd ) { *d = '\0'; return s;} 
		*d++ = *s++;
	}
	*d = '\0';
	
	#ifdef _DEBUG
		BCH_PERF_STOP;
	#endif
	
	return s; 
}

CHAR* str_firstvalidchar(register CHAR* s)
{
	#ifdef _DEBUG
		BCH_PERF_START;
	#endif
	
	while ( *s && ( *s == ' ' || *s == '\t' || *s == '\n' ) ) ++s;
	
	#ifdef _DEBUG
		BCH_PERF_STOP;
	#endif
	
	return s; 
}

CHAR* str_toend(register CHAR* s)
{
	#ifdef _DEBUG
		BCH_PERF_START;
	#endif
	
	while ( *s ) ++s;
	
	#ifdef _DEBUG
		BCH_PERF_STOP;
	#endif
	
	return s;
}

CHAR* str_insc(register CHAR* d, register CHAR c)
{
	#ifdef _DEBUG
		BCH_PERF_START;
	#endif
	
	UINT32 l = strlen(d) + 1;
	memmove(d+1,d,l);
	*d = c;
	
	#ifdef _DEBUG
		BCH_PERF_STOP;
	#endif
	
	return d + 1;
}

CHAR* str_inss(register CHAR* d, register CHAR* s)
{
	#ifdef _DEBUG
		BCH_PERF_START;
	#endif
	
	register UINT32 ls = strlen(s);
	register UINT32 ld = strlen(d) + 1;
	memmove(d+ls,d,ld);
	memcpy(d,s,ls);
	
	#ifdef _DEBUG
		BCH_PERF_STOP;
	#endif
	
	return d + ls;
}

CHAR* str_del(register CHAR* d, register UINT32 n)
{
	#ifdef _DEBUG
		BCH_PERF_START;
	#endif
	
	register UINT32 l = str_len(d);
	if ( n >= l ) { *d = '\0'; return d;}
	memmove(d,d+n, (l - n)+1);
	
	#ifdef _DEBUG
		BCH_PERF_STOP;
	#endif
	
	return d;
}

BOOL str_empty(register CHAR* s)
{	
	return ( !s || !*s ) ? TRUE : FALSE;
}

INT32 str_isvoc(register INT32 c)
{	
	c |= 0x20;
	return !((c ^ 0x61) && (c ^ 0x65) && (c ^ 0x69) && (c ^ 0x6F) && (c ^ 0x75));
}

CHAR* str_ltrim(register CHAR* s)
{
	register CHAR* st = s;
	s = str_skipspace(s);
	if ( *s == '\0' ) { *st = '\0'; return st;}
	if ( s == st ) { return st; }
	memmove(st,s,strlen(s) + 1);
	return st;
}

CHAR* str_rtrim(register CHAR* s)
{
	register CHAR* st = s;
	s = str_toend(s);
	if ( s == st ) return st;
	--s;
	while ( s >= st && (*s == ' ' || *s == '\t' || *s == '\n') ) --s;
	if ( s < st ) { *st = '\0'; return st;}
	*(s+1) = '\0';
	return st;
}

CHAR* str_trim(register CHAR* s)
{
	s = str_ltrim(s);
	return str_rtrim(s);
}

INT32 rex_exec(CHAR** stout, CHAR** enout, CHAR** s, REGEX* r)
{
	#ifdef _DEBUG
		BCH_PERF_START;
	#endif
	
	regmatch_t rm;
	INT32 ret = regexec(r,*s,1,&rm,0);
	if ( 0 == ret )
	{
		*stout = rm.rm_so + *s;
		*enout = rm.rm_eo + *s;
		*s = *s + rm.rm_eo + 1;
		#ifdef _DEBUG
			BCH_PERF_STOP;
		#endif
		return 0;
	}
	
	#ifdef _DEBUG
		BCH_PERF_STOP;
	#endif
	return ret;
}
	
VOID rex_perror(INT32 err, REGEX* r)
{
	#ifdef _DEBUG
		BCH_PERF_START;
	#endif
	
	CHAR msg[0x1000];
	regerror(err,r,msg,0x1000);
	printf("rex %s\n",msg);
	
	#ifdef _DEBUG
		BCH_PERF_STOP;
	#endif
}



















